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
#include <iostream>

#include "zk_cpp.h"

#define MAXEPOLL 10000

int setnonblocking(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        printf("set blocking error: %d\n", errno);
        return -1;
    }
    return 0;
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

int main(int argc, char** argv) {
    // 获取ip port
    std::string url("127.0.0.1:2181");
    utility::zk_cpp zk;
    utility::zoo_rc ret = zk.connect(url);
    if (ret != utility::z_ok) {
        printf("try connect zk server failed, code[%d][%s]\n",
               ret, utility::zk_cpp::error_string(ret));
        exit(EXIT_FAILURE);
    }
    std::string path("/zk");
    std::string value;
    ret = zk.get_node(path.c_str(), value, nullptr, true);
    printf("try get path[%s]'s value, value[%s] ret[%d][%s]\n",
           path.c_str(), value.c_str(), ret, utility::zk_cpp::error_string(ret));
    std::string sep(":");
    std::size_t start_pos = 0;
    std::size_t pos = value.find(sep, start_pos);
    std::string ip = value.substr(start_pos, pos - start_pos);
    std::string str_port = value.substr(pos+1, value.size()-pos);
    std::cout << "ip: " << ip << std::endl;
    std::cout << "port: " << str_port << std::endl;

    int port = atoi(str_port.c_str());

    int conn_fd;
    struct sockaddr_in servaddr;
    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket error...\n");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);

    if (connect(conn_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error...\n");
        exit(EXIT_FAILURE);
    }

    if (setnonblocking(conn_fd) == -1) {
        printf("set non blocking error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    send_and_recv_epoll(conn_fd);
    //send_and_recv(conn_fd);

    close(conn_fd);
    printf("Exit\n");
    return 0;
}
