// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/TcpServer.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Acceptor.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "muduo/net/SocketsOps.h"

#include <stdio.h>  // snprintf

using namespace muduo;
using namespace muduo::net;

//构造函数
TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const string& nameArg,
                     Option option)
  : loop_(CHECK_NOTNULL(loop)),    //CHECK_NOTNULL 检查loop不是一个空指针 如果是null那么输出日志终止程序
    ipPort_(listenAddr.toIpPort()),  //根据地址可以获得端口号
    name_(nameArg),   //名称
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),   //acceptor对象的初始化 在h文件可以看到是使用智能指针管理这个对象的
    threadPool_(new EventLoopThreadPool(loop, name_)),  
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)  //下一个连接id
{
  //当一个连接事件到来的时候poll的pollpoller的poll函数返回的通道调用了通道的handleEvent然后调用了下面的
  //Acceptor::handleRead函数中会调用 TcpServer::newConnection   _1对应的是socket文件描述符，_2对应的是对方的地址（inetAddress）
  acceptor_->setNewConnectionCallback(  //对这个acceptor设置一个回调函数
      std::bind(&TcpServer::newConnection, this, _1, _2));
}   //在构造函数中给acceptor绑定了一个newConnection回调函数，所以当有客户端连接触发accept时，会调用该函数。

TcpServer::~TcpServer()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto& item : connections_)
  {
    TcpConnectionPtr conn(item.second);
    item.second.reset();
    conn->getLoop()->runInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() //多次调用无害 启动线程池
{
  if (started_.getAndSet(1) == 0)  //？？？？？
  {
    threadPool_->start(threadInitCallback_);

    assert(!acceptor_->listening());  //判断是否处于监听状态
    loop_->runInLoop(
        std::bind(&Acceptor::listen, get_pointer(acceptor_)));  //get_pointer返回acceptor_的原生指针  通过原生指针调用listen函数
  }
}
 

/*
在TcpServer::newConnection()中，我们把TcpServer::removeConnection()注册到TcpConnection的setCloseCallback上，用于接收连接断开的消息。
在新连接到达时，Acceptor会回调newConnection( )，然后TcpServer创建TcpConnection对象，并加入到Map当中；在连接断开时，TcpConnection会回调removeConnection()，并从Map中移除。
*/
//因为TcpServer的生命周期一般情况下都大于TcpConnection，且～TcpServer()中会对连接进行关闭，因此该函数是安全的。
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
  loop_->assertInLoopThread();  //断言在io线程中
  EventLoop* ioLoop = threadPool_->getNextLoop();   //使用round-robin选取一个I/O loop
   
   //name_构造
  char buf[64];
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  ++nextConnId_;  //下一个连接id++
  string connName = name_ + buf;  //连接的名称

  LOG_INFO << "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toIpPort();

   //构造本地地址
  InetAddress localAddr(sockets::getLocalAddr(sockfd));  //构造地址 通过套接字我们可以得到一个地址
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop,    //连接   //创建一个连接对象
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));
  connections_[connName] = conn;  //将连接存放到连接列表里面    然后设置回调函数
  conn->setConnectionCallback(connectionCallback_);  //应用只会去调用TcpServer的callback  //将用户提供的connectionCallback_等原样传给TcpConnection，TcpServer持有TcpConnection的shared_ptr
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(  //将TcpServer的removeConnection设置到TcpConnection的关闭回调函数中
      std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
      //connectEstablished会调用用户提供的ConnectionCallback
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
  //调用TcpConenction:：connectEstablished函数内部会将use_count加一然后减一，此处仍为2
  //但是本函数结束后conn对象会析构掉，所以引用计数为1，仅剩connections_列表中存活一个
}


/*
在此处将原本使用一个函数即可的removeConnection函数拆分成两个『removeConnection()』和『removeConnectionInLoop()』
目的在于回调CloseCallback时，TcpConnection会在自己的ioLoop线程调用removeConnection()，拆分后则将其移到TcpServer的loop_线程进行。

*/
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  // FIXME: unsafe
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
}
/*

在TcpServer::removeConnectionInLoop()倒数第三行，将connectDestroyed()移交回TcpConnection的ioLoop，目的是让connectDestroyed()调用的connectionCallback_始终在其ioLoop回调，方便客户端代码编写。
因为将conn从Map中移除，conn的引用计数已经降低为1。将TcpConnection的生命期长到调用connectDestroyed()的时候。

*/
