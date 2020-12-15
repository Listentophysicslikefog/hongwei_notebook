/*
*  epoll_create:创建保存epoll文件描述符的空间      epoll_ctl: 向空间注册并注销文件描述符   epoll_wait:  等待文件描述符发生变化
*  epoll方式下面由操作系统负责保存监视对象文件描述符，所以需要向操作系统请求创建保存文件描述符的空间，使用epoll_create函数
*  epoll添加和删除描述符是通过epoll_ctl函数请求操作系统完成
*  epoll通过epoll_event将发生事件的文件描述符单独集中到一起，声明足够大的epoll_event结构体数组后，传递给epoll_wait函数时，发生事件的文件描述符
*  信息将被填入该数组，因此无需像select那样对所有的文件描述符进行循环
*  epoll_event用于保存发生事件的文件描述符。也可以在epoll列程中注册文件描述符时，用于注册关注的事件
*/

/*  
 这里的int只是决定保存epoll文件描述符的空间的大小，也叫epoll列程，但是linux 2.6.8后不用传，它可以根据实际情况来自己调整大小
int epoll_create(int size)    //成功返回epoll的文件描述符，失败返回-1 ，由操作系统管理，也需要调用close函数，必须调用close()关闭，否则可能导致fd被耗尽。



epoll的事件注册函数，它不同于select()是在监听事件时告诉内核要监听什么类型的事件，而是在这里先注册要监听的事件类型。
生成epoll列程后，应该在它的内部注册 监视对象文件描述符 ，使用epoll_ctl函数
int epoll_ctl(int epfd,int op,int fd,struct epoll_event* event)  //成功返回epoll的文件描述符，失败返回-1 
epfd: 用于注册监视对象的epoll列程的文件描述符     op: 用于指定监视对象的添加、删除或更改操作
fd: 需要注册的监视对象文件描述符                  event:  监视对象的事件类型

例子： epoll_ctl(A,EPOLL_CTL_ADD,B,C)   //表示 epoll列程A中注册文件描述符B（就是添加），主要目的是监视参数中的C事件
例子： epoll_ctl(A,EPOLL_DEL_ADD,B,NULL)  //表示从例程A中删除文件描述符B，从监视对象中删除时，不需要监视的事件类型

第二个参数的含义： EPOLL_CTL_ADD:  将文件描述符注册到epoll列程       EPOLL_CTL_DEL: 从epoll列程中删除文件描述符
                  EPOLL_CTL_MOD: 更改注册的文件描述符的关注事件的发生情况

epoll_event用于保存发生事件的文件描述符。也可以在epoll列程中注册文件描述符时，用于注册关注的事件
例子：struct epoll_event event;
      event.events=EPOLLIN;   //发生需要读取数据的情况(事件)时
      event.data.fd=sockfd;   //列程中注册文件描述符时，用于注册关注的事件
      epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event)
//该例子表示：将sockfd注册到epoll列程epfd中，并且在需要读取数据的情况下产生相应的事件

epoll_event的成员events中可以保存的常量及所指的事件类型如下：
EPOLLIN: 需要读取数据的情况
EPOLLOUT:输出缓冲为空，可以立即发送数据的情况
EPOLLRPI:收到OOB数据的情况
EPOLLRDHUP: 断开连接或半关闭的情况，在边缘触发的方式非常有用
EPOLLERR: 发生错误的情况
EPOLLET:以边缘触发的方式得到事件通知
EPOLLONESHOT: 发生一次事件后，相应的文件描述符不再收到事件通知。

epoll_ctl函数的第二个参数传递EPOLL_CTL_MOD,再次设置事件
可以通过位或运算同时传递多个上述参数



epoll_wait,epoll相关函数默认最后调用该函数
int epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout)   //成功返回epoll的文件描述符，失败返回-1
epfd  :表示事件发生监视范围的epoll列程的文件描述符      events : 保存发生事件的文件描述符集合的结构体地址
maxevents： 第二个参数可以保存的最大事件数      timeout：超时时间，-1为一直等待
第二个参数的缓冲区需要动态分配：
例子：   int  event_cnt;
        struct epoll_event* ep_events;
        ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE)   //EPOLL_SIZE是常宏量
       event_cnt=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1)
调用函数后，返回发生事件的文件描述符，同时在第二个参数指向的缓冲中保存发生事件的文件描述符集合。所以无需向select那样循环所有文件描述符




*/


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/epoll.h>

#define BUF_SIZE   100
#define EPOLL_SIZE 50
void error_handling(char* buf);

int main(int arg,char* argv[]){
      int serv_sock,clnt_sock;
      struct sockaddr_in serv_adr,clnt_adr;
      socklen_t adr_sz;
      int str_len,i;
      char buf[BUF_SIZE];

      struct epoll_event* ep_events;
      struct epoll_event  event;
      int epfd,event_cnt;
      if (arg!=2){

      printf("usage:%s <port>\n",argv[0]);
      exit(1);
      }
      serv_sock=socket(PF_INET,SOCK_STREAM,0);
      memset(&serv_adr,0,sizeof(serv_adr));
      serv_adr.sin_family=AF_INET;
      serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
      serv_adr.sin_port=htons(atoi(argv[1]));

      if (bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1){
           error_handling("bind() error!");
      }
      if(listen(serv_sock,5)==-1){
           error_handling("listen() error!");
      }
      epfd=epoll_create(EPOLL_SIZE);
      ep_events=(epoll_event*)(malloc(sizeof(struct epoll_event)*EPOLL_SIZE));

      event.events=EPOLLIN;
      event.data.fd=serv_sock;
      epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event);

       while(1){
           event_cnt=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
           if(event_cnt==-1){
                 puts("epoll_wait() error!!");
                 break;
           }

           for(i=0;i<event_cnt;i++){
                 if (ep_events[i].data.fd==serv_sock){
                       adr_sz=sizeof(clnt_adr);
                       clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_adr,&adr_sz);
                       event.events=EPOLLIN;
                       event.data.fd=clnt_sock;
                       epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);
                       printf("connect client: %d\n",clnt_sock);
                 }
                 else{
                       str_len=read(ep_events[i].data.fd,buf,BUF_SIZE);
                       if(str_len==0)   //close request
                       {
                             epoll_ctl(epfd,EPOLL_CTL_DEL,ep_events[i].data.fd,NULL);
                             close(ep_events[i].data.fd);
                             printf("close client: %d\n",ep_events[i].data.fd);
                       }else{
                             write(ep_events[i].data.fd,buf,str_len);
                       }
                 }
           }
      }
      close(serv_sock);
      close(epfd);
           return 0;

}

 void error_handling(char * buf){
           fputs(buf,stderr);
           fputc('\n',stderr);
           exit(1);
}