// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/Acceptor.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())), //创建一个套接字，就是监听套接字
    acceptChannel_(loop, acceptSocket_.fd()),  //acceptChannel_关注该套接字的事件
    listening_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))  //预先准备一个空闲的文件描述符   这是整数的构造函数
{
  assert(idleFd_ >= 0);  //断言这个大于0
  acceptSocket_.setReuseAddr(true);   //设置地址重复利用
  acceptSocket_.setReusePort(reuseport);  //设置端口重复利用 
  acceptSocket_.bindAddress(listenAddr);   //绑定
  acceptChannel_.setReadCallback(   //设置读的回调函数
      std::bind(&Acceptor::handleRead, this));  //可读事件到来后回调handleRead
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();  //先把所有事件disable掉
  acceptChannel_.remove();   //然后再remove掉
  ::close(idleFd_);  //然后close掉监听套接字
}

void Acceptor::listen()   //监听
{
  loop_->assertInLoopThread();   //断言再io线程中
  listening_ = true;    //监听为true
  acceptSocket_.listen();  //调用套接字的listen函数
  acceptChannel_.enableReading();   //关注它的可读事件
}

void Acceptor::handleRead()   //可读事件到来后回调handleRead
{
  loop_->assertInLoopThread();
  InetAddress peerAddr;   //准备对端地址
  //FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr); //accept获得对端地址存放到peerAddr里
  if (connfd >= 0)  //表示得到一个连接，处理连接
  {
    // string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    if (newConnectionCallback_)   //如果设置了回调函数
    {   //调用回调函数
      newConnectionCallback_(connfd, peerAddr);  // 会传递套接字和对端地址
    }
    else   //如果没有设置回调函数
    {
      sockets::close(connfd);   //关闭连接套接字
    }
  }
  else   //connfd < 0  表示失败了   
  {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE)
    {
      ::close(idleFd_);  //关闭一个空闲文件描述符以便可以处理文件描述符
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);  //接受后就立刻关闭掉
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);   //准备一个空闲的文件描述符
    }
  }
}

