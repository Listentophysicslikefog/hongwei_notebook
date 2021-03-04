// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "muduo/net/Connector.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/SocketsOps.h"

#include <errno.h>

using namespace muduo;
using namespace muduo::net;

const int Connector::kMaxRetryDelayMs;

//构造函数里面没有创建channel对象在connecting函数中会创建
Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)   //构造函数
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDelayMs)
{
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()  //析构函数
{
  LOG_DEBUG << "dtor[" << this << "]";
  assert(!channel_);
}

void Connector::start()   //可以跨线程调用，是线程安全的
{
  connect_ = true;   //先将connect设置为true
  loop_->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
  //然后在loop所在的io线程中执行 startInLoop
}

void Connector::startInLoop()
{
  loop_->assertInLoopThread();   //断言属于io线程中
  assert(state_ == kDisconnected);  //断言状态为 kDisconnected
  if (connect_)  //如果connect_为true就发起连接
  {
    connect();  //发起连接的函数
  }
  else   //否则不可以发起连接，因为有可能其他的线程调stop使得connect_ = false
  {
    LOG_DEBUG << "do not connect";
  }
}

//可以跨线程调用
void Connector::stop()  //关闭
{
  connect_ = false;  //设置connect_为false

  //在io线程中调用stopInLoop
  loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
  // FIXME: cancel timer
}

void Connector::stopInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnecting)  //如果状态为kConnecting表示在连接中还没有连接成功
  {
    setState(kDisconnected);  //设置为kDisconnected
    int sockfd = removeAndResetChannel();  //从poller中移除关注，并且将channel置空
    retry(sockfd);   //因为当前状态为kDisconnected ，这里并非要重连，只是调用sockets::clse(sockfd)   //
  }
}

void Connector::connect()   //发起连接函数
{
  int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());   //生成非阻塞套接字,返回fd,实际就是调用socket()
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());   //就是对connect()的封装
  int savedErrno = (ret == 0) ? 0 : errno;   //保存错误码
  switch (savedErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);   //说明连接成功，调用connecting()函数，对成功的连接进行处理.
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:     //如果是上面的错误码
      retry(sockfd);     //说明连接暂时失败，但是仍可能成功，需要重连。调用retry()函数重连：
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);   // 彻底失败，返回errno为EACCES等错误码,这种情况只能关掉sockfd，因为再怎么试也成功不了的。
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::restart()  //重启
{
  loop_->assertInLoopThread();
  setState(kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;  //设置connect_为true
  startInLoop();  //执行startInLoop函数进行连接
}

//一旦连接成功就会回调channel的 handleWrite 就处于可写状态 --->我们就去看handleWrite函数
void Connector::connecting(int sockfd)
{
  setState(kConnecting);  //设置状态为kConnecting
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));   //创建一个channel对象并且Channel与sockfd关联起来
  channel_->setWriteCallback(    //设置可写回调函数，这时候如果socket没有错误，sockfd就处于可写状态
      std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
  channel_->setErrorCallback(   //设置错误回调函数
      std::bind(&Connector::handleError, this)); // FIXME: unsafe

  // channel_->tie(shared_from_this()); is not working,
  // as channel_ is not managed by shared_ptr
  channel_->enableWriting();   //让poller 关注可写事件
}

int Connector::removeAndResetChannel()   //从poller中移除关注，并且将channel置空
{
  channel_->disableAll();  //先调用disableAll
  channel_->remove();   //然后再remove
  int sockfd = channel_->fd();
  // Can't reset channel_ here, because we are inside Channel::handleEvent
  //然后调用queueInLoop执行resetChannel
  //不可以在这里重置channell_，因为正在调用Channel::handleEvent从而调用-->handleWrite-->removeAndResetChannel()
  loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe

  //我们实际上还在调用channel的函数还没有返回所以这个函数里不可以重置channell_，这里我们把这个重置函数加入到io线程的队列中
  //这样就保证跳出这个函数后执行resetChannel
  return sockfd;
}

void Connector::resetChannel()
{
  channel_.reset();
}

void Connector::handleWrite()
{
  LOG_TRACE << "Connector::handleWrite " << state_;

  if (state_ == kConnecting)  //如果连接成功后这个channel就没有价值了，就不需要关注了
  {
    int sockfd = removeAndResetChannel();   //从poller中移除关注，并且将channel置空

    //socket可写并不意味着连接一定建立成功
    //getSocketError函数会调用getsockopt(sockfd,SOL_SOCKET,......)再次确认一下是否有错误
    int err = sockets::getSocketError(sockfd);
    if (err)  //有错误
    {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = "
               << err << " " << strerror_tl(err);
      retry(sockfd);  //重连
    }
    else if (sockets::isSelfConnect(sockfd))//如果是自连接
    {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);  //重连
    }
    else  //连接成功
    {
      setState(kConnected);  //设置状态为kConnected
      if (connect_)  //如果connect_状态为true执行回调函数
      {
        newConnectionCallback_(sockfd);  //回调newConnectionCallback_函数
      }
      else //否则关闭套接字
      {
        sockets::close(sockfd);
      }
    }
  }
  else
  {
    // what happened?
    assert(state_ == kDisconnected);
  }
}

void Connector::handleError()  //产生错误的回调函数
{
  LOG_ERROR << "Connector::handleError state=" << state_;
  if (state_ == kConnecting)   
  {
    int sockfd = removeAndResetChannel();   //从poller中移除关注，并且将channel置空
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);  //重新发起连接
  }
}


//重连函数，采用back-off策略重连，也就是退避策略
//也就是重连时间逐渐延长，0.5s,1s,2s,...一直到30s
void Connector::retry(int sockfd)
{
  sockets::close(sockfd);  //先关闭连接
  setState(kDisconnected);  //设置状态
  if (connect_)
  {
    LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
             << " in " << retryDelayMs_ << " milliseconds. ";
    loop_->runAfter(retryDelayMs_/1000.0,   //注册一个定时操作，重连    //隔一段时间后重连，重新启用startInLoop
                    std::bind(&Connector::startInLoop, shared_from_this()));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);    //间隔时间2倍增长
  }
  else
  {     //超出最大重连时间后，输出连接失败
    LOG_DEBUG << "do not connect";
  }
}

