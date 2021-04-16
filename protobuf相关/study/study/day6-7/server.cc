#include <errno.h>
#include <fcntl.h>       // for nonblocking
#include <netinet/in.h>  // for sockaddr_in
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>     // for epoll
#include <sys/resource.h>  // for 最大连接数需要setrlimit
#include <sys/socket.h>    // for socket
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include "zk_cpp.h"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

#define MAXEPOLL 10000  // 对服务器来说，这个值可以很大
#define PORT 6000     // 端口
#define MAXBACK 1000  // 请求队列最大长度

// 设置非阻塞
int setnonblocking(int fd) {
    // if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
    //     printf("set blocking error: %d\n", errno);
    //     return -1;
    // }
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl(F_GETFL)\n");
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        perror("fcntl(F_SETFL)\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    // 向zk创建znode
    std::string url("127.0.0.1:2181");
    utility::zk_cpp zk;
    utility::zoo_rc ret = zk.connect(url);
    if (ret != utility::z_ok) {
        printf("try connect zk server failed, code[%d][%s]\n",
               ret, utility::zk_cpp::error_string(ret));
        exit(EXIT_FAILURE);
    }
    std::string path("/zk");
    std::string value("127.0.0.1:6000");
    std::vector<utility::zoo_acl_t> acl;
    acl.push_back(utility::zk_cpp::create_world_acl(utility::zoo_perm_all));
    ret = zk.create_persistent_node(path.c_str(), value, acl);
    printf("create path[%s] ret[%d][%s], path[%s]\n",
            path.c_str(), ret, utility::zk_cpp::error_string(ret), path.c_str());

    // 连接MongoDB
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{}};
    mongocxx::database db = client["mydb"];  // 访问数据库
    mongocxx::collection coll = db["test"];  // 访问集合
    db.drop();                               // 删除之前的记录

    int listen_fd;
    int conn_fd;
    int epoll_fd;
    int nread;
    int cur_fds;   // 当前已经存在的fd数量
    int wait_fds;  // epoll_wait的返回值
    int i;
    struct sockaddr_in servaddr, cliaddr;
    struct epoll_event ev, evs[MAXEPOLL];
    struct rlimit rlt;  // 设置连接数所需
    char buf[BUFSIZ];
    socklen_t len = sizeof(struct sockaddr_in);

    // 设置每个进程允许打开的最大文件数
    rlt.rlim_max = rlt.rlim_cur = MAXEPOLL;
    if (setrlimit(RLIMIT_NOFILE, &rlt) == -1) {
        printf("setrlimit error : %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // 建立套接字
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞模式
    if (setnonblocking(listen_fd) == -1) {
        printf("set non blocking error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 设置server地址
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // INADDR_ANY就是指定地址为0.0.0.0的地址,也就是表示本机的所有IP
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    // 绑定
    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(listen_fd, MAXBACK) == -1) {
        printf("listen error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 创建epoll
    epoll_fd = epoll_create(MAXEPOLL);
    if (epoll_fd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN | EPOLLET;  // accept read
    ev.data.fd = listen_fd;         // add listen_fd
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
        printf("epoll_ctl error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    cur_fds = 1;

    while (true) {
        if ((wait_fds = epoll_wait(epoll_fd, evs, cur_fds, -1)) == -1) {
            printf("epoll_wait error : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < wait_fds; ++i) {
            if (evs[i].data.fd == listen_fd && cur_fds < MAXEPOLL) {
                if ((conn_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &len)) == -1) {
                    printf("accept error : %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                printf("server get from client port : %d\n", cliaddr.sin_port);

                ev.events = EPOLLIN | EPOLLET;  // accept read
                ev.data.fd = conn_fd;           // add conn_fd
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0) {
                    printf("epoll error : %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                ++cur_fds;
                continue;
            }

            // 处理数据
            nread = read(evs[i].data.fd, buf, sizeof(buf));
            if (nread < 0) {
                close(evs[i].data.fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);  // 删除计入的fd
                --cur_fds;
                continue;
            }
            buf[nread] = 'a';
            buf[nread + 1] = '\0';

            // 写入MongoDB
            std::string mongo_buf(buf, nread);
            auto builder = bsoncxx::builder::stream::document{};
            bsoncxx::document::value doc_value = builder
                << "value" << mongo_buf << bsoncxx::builder::stream::finalize;
            bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(doc_value.view());
            std::cout << "insert one document, value : " << mongo_buf << std::endl;

            // 回写
            write(evs[i].data.fd, buf, nread);
        }
    }

    close(listen_fd);
    return 0;
}