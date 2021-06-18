
79495  0.0  0.0 119260  1488 ?        S    4月19   0:00 /bin/csh -c /data/vdbench/vdbench SlaveJvm -m localhost -n localhost-10-210419-11.29.41.957 -l localhost-0 -p 5570

78075  0.0  0.0 119276  1532 ?        S    11:29   0:00 /bin/csh /data/vdbench/vdbench SlaveJvm -m localhost -n localhost-10-210419-11.29.33.455 -l localhost-0 -p 5570


5017  0.0  0.0 119276  1536 ?        S    11:29   0:00 /bin/csh /data/vdbench/vdbench SlaveJvm -m localhost -n localhost-10-210419-11.29.02.496 -l localhost-0 -p 5571

11530  0.0  0.0 119276  1532 ?        S    21:59   0:00 /bin/csh /data/vdbench/vdbench SlaveJvm -m localhost -n localhost-10-210418-21.59.31.309 -l localhost-0 -p 5571



蚂蚁笔记本： https://leanote.com/


#include "uevent.h"

#include <utility>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <memory>
#include "logging.h"
#include "thread.h"
#include "listener_libevent.h"
#include "eventloop_libevent.h"
#include "loop_handle.h"

using namespace uevent;
using namespace base;

std::unique_ptr<ListenerUevent> g_listener;
int numThreads = 10;


class EchoServer : public LoopHandle {
 public:
  static LoopHandle* CreateMyself(UeventLoop* loop) {
    return reinterpret_cast<LoopHandle*>(new EchoServer(loop));
  }
  EchoServer(UeventLoop* loop)
      : name_("henry server") {
  }
  static void ConnectionSuccessHandle(const ConnectionUeventPtr& conn) {
    LoopHandle* loop_handle = conn->GetLoop()->GetLoopHandle();
    EchoServer* pobj = reinterpret_cast<EchoServer*>(loop_handle);
    LOG_INFO << "echo sever name: " << pobj->name_;
    LOG_INFO << "connection from " << conn->GetPeerAddress().ToString()
             << " success";
  }
  static void ConnectionClosedHandle(const ConnectionUeventPtr& conn) {
   // LoopHandle* loop_handle = conn->GetLoop()->GetLoopHandle();
   // EchoServer* pobj = reinterpret_cast<EchoServer*>(loop_handle);
    LOG_INFO << "disconnect from " << conn->GetPeerAddress().ToString();
    g_listener->RemoveConnection(conn); // 验证连接的释放
  }

  static void MessageReadHandle(const ConnectionUeventPtr& conn) {
    LoopHandle* loop_handle = conn->GetLoop()->GetLoopHandle();
    EchoServer* pobj = reinterpret_cast<EchoServer*>(loop_handle);
    LOG_INFO << "echo sever name: " << pobj->name_;
    size_t readable = conn->ReadableLength();
    void* data = malloc(readable);
    conn->ReceiveData(data, readable);
    conn->RemoveData(data, readable);
    LOG_INFO << "Connection Id: " << conn->GetId() << " recv " << readable
             << " bytes";
    conn->SendData(data, readable);
    free(data);
  }
  static void MessageWriteHandle(const ConnectionUeventPtr& conn) {
    LOG_INFO << "MessageWriteHandle";
  }

 private:
  const std::string name_;
  UeventLoop* loop_;
  ListenerUevent* listener_;
};


int main(int argc, char* argv[]) {
  Option option;
  option.loop_strategy = Option::kRoundRobin;
  option.reuse_port = true;
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  LOG_INFO << "sizeof ConnectionUevent = " << sizeof(ConnectionUevent);
  if (argc > 1) {
    numThreads = atoi(argv[1]);
  }
  EventLoopLibevent loop("main_thread",EchoServer::CreateMyself);
  UsockAddress listenAddr("192.168.150.36", 13500, false);
  // unlink("/root/yeheng.sock");
  // UsockAddress listenAddr("/root/yeheng.sock");

  g_listener.reset(new ListenerLibevent(&loop, listenAddr,"EchoListener", option));
  g_listener->SetConnectionSuccessCb(EchoServer::ConnectionSuccessHandle);
  g_listener->SetConnectionClosedCb(EchoServer::ConnectionClosedHandle);
  g_listener->SetMessageReadCb(EchoServer::MessageReadHandle);
  g_listener->SetMessageWriteCb(EchoServer::MessageWriteHandle);
  g_listener->SetCreateLoopHandleCb(EchoServer::CreateMyself);  //相当于 EventLoopThread::threadFunc()，每一个线程会有一个loophandle
  g_listener->SetThreadNum(numThreads);

  g_listener->Start();
}








#include <iostream>
#include <assert.h>
#include <poll.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <string.h>
#include <thread>

static int s_efd = 0;

int createEventfd()
{
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

  std::cout << "createEventfd() fd : " << evtfd << std::endl;

  if (evtfd < 0)
  {
    std::cout << "Failed in eventfd\n";
    abort();
  }

  return evtfd;
}

void testThread()
{
  int timeout = 0;
  while(timeout < 3) {
    sleep(1);
    timeout++;
  }

  uint64_t one = 1;
  ssize_t n = write(s_efd, &one, sizeof one);
  if(n != sizeof one)
  {
    std::cout << " writes " << n << " bytes instead of 8\n";
  }
}

int main()
{
  s_efd = createEventfd();

  fd_set rdset;
  FD_ZERO(&rdset);
  FD_SET(s_efd, &rdset);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  std::thread t(testThread);

  while(1)
  {
    if(select(s_efd + 1, &rdset, NULL, NULL, &timeout) == 0)
    {
      std::cout << "timeout\n";
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      FD_SET(s_efd, &rdset);
        continue;
    }

    uint64_t one = 0;

    ssize_t n = read(s_efd, &one, sizeof one);
    if(n != sizeof one)
    {
      std::cout << " read " << n << " bytes instead of 8\n";
    }

    std::cout << " wakeup ！\n";

    break;
  }

  t.join();
  close(s_efd);

  return 0;
}







