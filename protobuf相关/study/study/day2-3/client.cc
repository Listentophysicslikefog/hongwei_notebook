#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>  // for epoll
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "myproto.h"

#define MAXEPOLL 10000
#define SERV_PORT 6000

int setnonblocking(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        printf("set blocking error: %d\n", errno);
        return -1;
    }
    return 0;
}

void myproto_send_and_recv(int conn_fd) {
    int epoll_fd;
    int cur_fds;
    int wait_fds;
    struct epoll_event ev;
    struct epoll_event evs[MAXEPOLL];
    char send[BUFSIZ];
    char recv[BUFSIZ];
    int lens;
    int n;
    int i = 0;
    FILE* fp = stdin;

    epoll_fd = epoll_create(MAXEPOLL);
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = conn_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0) {
        printf("epoll_ctl error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ++cur_fds;  // 即cur_fds = 1;

    while (true) {
        if ((wait_fds = epoll_wait(epoll_fd, evs, cur_fds, -1)) == -1) {
            printf("epoll_wait error : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < wait_fds; ++i) {
            if (evs[i].events & EPOLLIN) {
                printf("EPOLLIN ...\n");
                printf("client get from server ...\n");
                memset(recv, 0, sizeof(recv));
                n = read(evs[i].data.fd, recv, BUFSIZ);
                // read错误判断
                if (n == 0) {
                    printf("recv OK ...\n");
                    continue;
                } else if (n == -1) {
                    printf("recv ERROR ...\n");
                    continue;
                } else {
                    // 解析消息
                    MyProtoDecode myDecode;
                    myDecode.init();

                    if (!myDecode.parser(recv, n)) {
                        printf("parser msg failed!\n");
                        exit(EXIT_FAILURE);
                    }
                    MyProtoMsg* pMsg = NULL;
                    while (!myDecode.empty()) {
                        pMsg = myDecode.front();
                        printMyProtoMsg(*pMsg);
                        myDecode.pop();
                    }
                }
            }

            if (evs[i].events & EPOLLOUT) {
                printf("EPOLLOUT ...\n");
                memset(send, 0, sizeof(send));
                if (fgets(send, BUFSIZ, fp) == NULL) {
                    printf("End ...\n");
                    exit(EXIT_FAILURE);
                } else {
                    send[strlen(send) - 1] = '\0';
                    if (strcmp(send, "q") == 0) {
                        printf("Bye ...\n");
                        return;
                    }

                    uint32_t len = 0;
                    uint8_t* pData = NULL;
                    MyProtoMsg msg1;
                    MyProtoEncode myEncode;
                    // 放入消息
                    msg1.head.server = 1;
                    msg1.body = send;
                    pData = myEncode.encode(&msg1, len);

                    printf("client send: %s\n", send);
                    write(evs[i].data.fd, pData, len);
                }
            }
        }
    }
}

void send_and_recv_epoll(int conn_fd) {
    int epoll_fd;
    int cur_fds;
    int wait_fds;
    struct epoll_event ev;
    struct epoll_event evs[MAXEPOLL];
    char send[BUFSIZ];
    char recv[BUFSIZ];
    int lens;
    int n;
    int i = 0;
    FILE* fp = stdin;

    epoll_fd = epoll_create(MAXEPOLL);
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = conn_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0) {
        printf("epoll_ctl error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ++cur_fds;  // 即cur_fds = 1;

    while (true) {
        if ((wait_fds = epoll_wait(epoll_fd, evs, cur_fds, -1)) == -1) {
            printf("epoll_wait error : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < wait_fds; ++i) {
            if (evs[i].events & EPOLLIN) {
                printf("EPOLLIN ...\n");
                printf("client get from server ...\n");
                memset(recv, 0, sizeof(recv));
                n = read(evs[i].data.fd, recv, BUFSIZ);
                // read错误判断
                if (n == 0) {
                    printf("recv OK ...\n");
                    continue;
                } else if (n == -1) {
                    printf("recv ERROR ...\n");
                    continue;
                } else {
                    lens = strlen(recv);
                    recv[lens] = '\0';
                    printf("write to stdout...\n");
                    write(STDOUT_FILENO, recv, BUFSIZ);
                    printf("\n");
                }
            }

            if (evs[i].events & EPOLLOUT) {
                printf("EPOLLOUT ...\n");
                memset(send, 0, sizeof(send));
                if (fgets(send, BUFSIZ, fp) == NULL) {
                    printf("End ...\n");
                    exit(EXIT_FAILURE);
                } else {
                    lens = strlen(send);
                    send[lens - 1] = '\0';

                    if (strcmp(send, "q") == 0) {
                        printf("Bye ...\n");
                        return;
                    }

                    printf("client send: %s\n", send);
                    write(evs[i].data.fd, send, strlen(send));
                }
            }
        }
    }
}

void send_and_recv(int conn_fd) {
    FILE* fp = stdin;
    int lens;
    char send[BUFSIZ];
    char recv[BUFSIZ];
    fd_set rset;  // select()机制中提供一fd_set的数据结构
    FD_ZERO(&rset);
    int maxfd = (fileno(fp) > conn_fd ? fileno(fp) : (conn_fd + 1));
    int n;

    while (true) {
        FD_SET(fileno(fp), &rset);
        FD_SET(conn_fd, &rset);

        if (select(maxfd, &rset, NULL, NULL, NULL) == -1) {
            printf("client select error ...\n");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(conn_fd, &rset)) {
            printf("client get from server ...\n");
            memset(recv, 0, sizeof(recv));
            n = read(conn_fd, recv, BUFSIZ);
            if (n == 0) {
                printf("recv OK ...\n");
                break;
            } else if (n == -1) {
                printf("recv ERROR ...\n");
                break;
            } else {
                lens = strlen(recv);
                recv[lens] = '\0';
                write(STDOUT_FILENO, recv, BUFSIZ);
                printf("\n");
            }
        }

        if (FD_ISSET(fileno(fp), &rset)) {
            memset(send, 0, sizeof(send));
            if (fgets(send, BUFSIZ, fp) == NULL) {
                printf("End ...\n");
                exit(EXIT_FAILURE);
            } else {
                lens = strlen(send);
                send[lens - 1] = '\0';  // 减一的原因是不要回车字符

                if (strcmp(send, "q") == 0) {
                    printf("Bye ...\n");
                    return;
                }

                printf("client send: %s\n", send);
                write(conn_fd, send, strlen(send));
            }
        }
    }
}

int main(int argc, char** argv) {
    int conn_fd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        printf("input server ip! \n");
        exit(EXIT_FAILURE);
    }

    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket error...\n");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if (connect(conn_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error...\n");
        exit(EXIT_FAILURE);
    }

    if (setnonblocking(conn_fd) == -1) {
        printf("set non blocking error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    myproto_send_and_recv(conn_fd);
    //send_and_recv(conn_fd);

    close(conn_fd);
    printf("Exit\n");
    return 0;
}
