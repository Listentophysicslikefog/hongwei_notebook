//  https://blog.csdn.net/bestone0213/article/details/46743367
// https://blog.csdn.net/abcd1f2/article/details/38629387
// https://blog.csdn.net/liuhongxiangm/article/details/16113571

// pb 序列化和反序列化： https://blog.csdn.net/tennysonsky/article/details/73920767

size_t evbuffer_get_length(const  struct  evbuffer  * buf);  //返回evbuffer存储的字节数

int  evbuffer_add(struct  evbuffer  * buf, const  void   * data, size_t datlen);  //添加data处的datalen字节到buf的末尾，成功时返回0，失败时返回-1



ev_ssize_t evbuffer_copyout(struct  evbuffer  * buf, void   * data, size_t datlen);

//evbuffer_copyout（）的行为与evbuffer_remove（）相同，但是它不从缓冲区移除任何数据。也就是说，它从buf前面复制datlen字节到data处的内存中。如果可用字节少于datlen，函数会复制所有字节。失败时返回-1，否则返回复制的字节数。

int  evbuffer_drain(struct  evbuffer  * buf, size_t len); // evbuffer_drain（）函数的行为与evbuffer_remove（）相同，只是它不进行数据复制：而只是将数据从缓冲区前面移除。成功时返回0，失败时返回-1。

int  evbuffer_remove(struct  evbuffer  * buf, void   * data, size_t datlen); //evbuffer_remove（）函数从buf前面复制和移除datlen字节到data处的内存中。如果可用字节少于datlen，函数复制所有字节。失败时返回-1，否则返回复制了的字节数。

bufferevent_enable(bev, EV_READ | EV_WRITE); // 使 读/写 事件的回调函数生效   


struct evbuffer* bufferevent_get_input(struct bufferevent *bufev); //取出输入缓冲区
struct evbuffer* bufferevent_get_output(struct bufferevent *bufev); //取出输出缓冲区
bufferevent_socket_new()  // 创建基于套接字的bufferevent

bufferevent_socket_connect()

// 原文链接：https://blog.csdn.net/qq769651718/article/details/79431583

evconnlistener_new_bind()

函數原型:struct evconnlistener *evconnlistener_new_bind(struct event_base *base,
                                    evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
                                    const struct sockaddr *sa, int socklen);

第一個參數代表你的主事件，也就是你創建的event_base

第二個參數代表回調函數，觸發可讀事件之后，返回給我們需要的信息

第三個參數代表需要額外傳遞的參數

第四個參數代表標志位 1.LEV_OPT_REUSEABLE 設置地址復用，內部函數原型應該為setsocketopt(sockfd， SOL_SOCKET, SO_RESUEADDR,  on, sizeof(on))

  LEV_OPT_CLOSE_ON_FREE代表連接關閉時同事關閉底層套接字

第五個參數代表監聽的最大數量，如果設置為-1系統將選擇一個合適的數值，如果為-1，內部實現的數值為128(固定的)跟SOMAXCONN一樣的值

第六個參數代表需要綁定的地址

第七個參數代表地址的大小

函數內部創建了一個listenfd根據你的參數是否設置為非阻塞(evutil_make_socket_nonblocking())函數進行設置，這個函數的內部為fcntl函數(flags=fcntl(fd, F_GETFL), flags |= O_NONBLOCK,  fcntl(fd, F_SETFL, flags))進行設置

然后進行地址綁定(bind())，在創建一個evconnlistener *類型的對象用evconnlistener_new()函數來進行創建

evconnlistener_new()函數內部進行監聽(listen()), 創建一個evconnlistener_event的監聽事件，將主事件，listenfd，回調函數，evconnlistener_event進行綁定。這個回調函數為listener_read_cb函數，當着個函數被調用的時候，內部進行accept一個connfd，進而調用我們自己的回調函數read/write等操作











conn_->ConnectionEnable();
class BuffereventWrapper // 处理libevent事件到来后数据的读写
{}


ConnectionLibevent // 包函 BuffereventWrapper 成员变量对象，这个类应该是管理连接的
class ConnectionLibevent : public ConnectionUevent



class ListenerLibevent : public ListenerUevent  //ListenerUevent 里面有管理连接的的一个map typedef std::map<int64_t,ConnectionUeventPtr> ConnectionMap; 
{}


              
                                                               // 可能是最开始就初始化好的
                                                               // 调用函数会传入对应的参数，应该是libevent底层实现的
                                                               ListenerLibevent::AcceptCb(struct evconnlistener* listener,
                                                               evutil_socket_t sockfd,  //应该是对端连接的fd
                                                               struct sockaddr* addr,
                                                               int len,
                                                               void* arg)
ListenerLibevent 服务端设置监听以及对应的evconnlistener_new_bind() ,当有事件到来的时候会调用evconnlistener_new_bind()函数里面设置的一个函数，然后在该函数里面会先获取服务端的loop，就是ListenerLibevent创建时候的loop，然后通过loop从线程池里面获取一个EventLoopLibevent线程，有创建一个ConnectionLibevent对象然后设置对应的回调函数（将ListenerLibevent设置的回调函数给这个创建的新建连接对象），然后用刚才获取的loop对象runinloop执行一个函数ConnectionUevent::ConnectionEstablished 处理，然后该函数里面会调用


ConnectionUeventPtr conn(new ConnectionLibevent(
      io_loop, sockfd, conn_id, "", peer_addr));
  conn->Init();
  pobj->connections_[conn_id] = conn; // 保存在这里，退出后不会析构
  conn->SetConnectionSuccessCb(pobj->connection_success_cb_);  //将ListenerLibevent（服务器端）设置的回调函数给这个创建的新建连接对象   pobj->connection_success_cb_ 是服务器端初始化的时候ListenerLibevent设置的用户回调函数
  conn->SetConnectionClosedCb(pobj->connection_closed_cb_);    // 新建的一个ConnectionLibevent对象的回调函数没有初始化，因为所有的客户端连接到来服务器端需要使用对应的回调函数，所有将ListenerLibevent设置的回调函数给这个创建的新建连接对象
  conn->SetMessageReadCb(pobj->message_read_cb_);
  conn->SetMessageWriteCb(pobj->message_write_cb_);



//Init()函数
  int ConnectionLibevent::Init() {
  struct event_base* base =
      static_cast<EventLoopLibevent*>(loop_)->GetInnerBase();
  std::shared_ptr<ConnectionLibevent> ptr =
      std::dynamic_pointer_cast<ConnectionLibevent>(shared_from_this());
  bev_wrapper_ = new BuffereventWrapper(base, fd_, ptr);
  return 0;
}



ConnectorLibevent  这个是客户端创建连接    ConnectionLibevent是建立的一个连接





evconnlistener_new_bind() //函数详解  https://blog.csdn.net/bestone0213/article/details/46729247?utm_medium=distribute.pc_relevant.none-task-blog-baidujs_title-4&spm=1001.2101.3001.4242




客户端连接上服务端后服务端只会调用connection_success_cb_(ptr)连接成功的回调，不会调用其他的函数，如果客户端向服务端请求读数据的时候  服务端会调用SetMessageReadCb函数，然后处理客户的请求然后在将客户请求的相关数据发送过来，这个时候客户端会处理服务端发送过来的数据。




class EventLoopLibevent: public UeventLoop  //一个EventLoopLibevent应该就是一个loop循环




注册对应的pb然后接收到对应的pb请求后uevent会调用对应注册的pb函数




#include<netinet/in.h>  
#include<sys/socket.h>  
#include<unistd.h>  
  
#include<stdio.h>  
#include<string.h>  
  
#include<event.h>  
#include<listener.h>  
#include<bufferevent.h>  
#include<thread.h>  
  
  
void listener_cb(evconnlistener *listener, evutil_socket_t fd,  
                 struct sockaddr *sock, int socklen, void *arg);  
  
void socket_read_cb(bufferevent *bev, void *arg);  
void socket_error_cb(bufferevent *bev, short events, void *arg);  
  
int main()  
{  
    evthread_use_pthreads();//enable threads  
  
    struct sockaddr_in sin;  
    memset(&sin, 0, sizeof(struct sockaddr_in));  
    sin.sin_family = AF_INET;  
    sin.sin_port = htons(8989);  
  
    event_base *base = event_base_new();  
    evconnlistener *listener  
            = evconnlistener_new_bind(base, listener_cb, base,  
                                      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE,  
                                      10, (struct sockaddr*)&sin,  
                                      sizeof(struct sockaddr_in));  
  
    event_base_dispatch(base);  
  
     evconnlistener_free(listener);  
    event_base_free(base);  
  
    return 0;  
}  
  
  
//有新的客户端连接到服务器  
//当此函数被调用时，libevent已经帮我们accept了这个客户端。该客户端的  
//文件描述符为fd  
void listener_cb(evconnlistener *listener, evutil_socket_t fd,  
                 struct sockaddr *sock, int socklen, void *arg)  
{  
    event_base *base = (event_base*)arg;  
  
    //下面代码是为这个fd创建一个bufferevent  
    bufferevent *bev =  bufferevent_socket_new(base, fd,  
                                               BEV_OPT_CLOSE_ON_FREE);  
  
    bufferevent_setcb(bev, socket_read_cb, NULL, socket_error_cb, NULL);  
    bufferevent_enable(bev, EV_READ | EV_PERSIST);  
}  
  
  
void socket_read_cb(bufferevent *bev, void *arg)  
{  
    char msg[4096];  
  
    size_t len = bufferevent_read(bev, msg, sizeof(msg)-1 );  
  
    msg[len] = '\0';  
    printf("server read the data %s\n", msg);  
  
    char reply[] = "I has read your data";  
    bufferevent_write(bev, reply, strlen(reply) );  
}  
  
  
void socket_error_cb(bufferevent *bev, short events, void *arg)  
{  
    if (events & BEV_EVENT_EOF)  
        printf("connection closed\n");  
    else if (events & BEV_EVENT_ERROR)  
        printf("some other error\n");  
  
    //这将自动close套接字和free读写缓冲区  
    bufferevent_free(bev);  
}  











客户端启动连接：


在基于套接字的bufferevent上启动连接
如果bufferevent的套接字还没有连接上，可以启动新的连接。

接口
int bufferevent_socket_connect(struct bufferevent *bev,
    struct sockaddr *address, int addrlen);
 

address和addrlen参数跟标准调用connect()的参数相同。如果还没有为bufferevent设置套接字，调用函数将为其分配一个新的流套接字，并且设置为非阻塞的。

如果已经为bufferevent设置套接字，调用bufferevent_socket_connect()将告知libevent套接字还未连接，直到连接成功之前不应该对其进行读取或者写入操作。

连接完成之前可以向输出缓冲区添加数据。

如果连接成功启动，函数返回0；如果发生错误则返回-1。

示例
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <string.h>
 
void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
            something here, like start reading or writing. */
    } elseif (events & BEV_EVENT_ERROR) {
         /* An error occured while connecting. */
    }
}
 
int main_loop(void)
{
    struct event_base *base;
    struct bufferevent *bev;
    struct sockaddr_in sin;
 
    base = event_base_new();
 
    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    sin.sin_port = htons(8080); /* Port 8080 */
 
    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
 
    bufferevent_setcb(bev, NULL, NULL, eventcb, NULL);
 
    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        /* Error starting connection */
        bufferevent_free(bev);
        return -1;
    }
 
    event_base_dispatch(base);
    return 0;
}
 

bufferevent_socket_connect()函数由2.0.2-alpha版引入。在此之前，必须自己手动在套接字上调用connect()，连接完成时，bufferevent将报告写入事件。

注意：如果使用bufferevent_socket_connect()发起连接，将只会收到BEV_EVENT_CONNECTED事件。如果自己调用connect()，则连接上将被报告为写入事件。

这个函数在2.0.2-alpha版引入。