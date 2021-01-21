// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "muduo/net/TcpClient.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Connector.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/SocketsOps.h"

#include <stdio.h>  // snprintf

using namespace muduo;
using namespace muduo::net;

// TcpClient::TcpClient(EventLoop* loop)
//   : loop_(loop)
// {
// }

// TcpClient::TcpClient(EventLoop* loop, const string& host, uint16_t port)
//   : loop_(CHECK_NOTNULL(loop)),
//     serverAddr_(host, port)
// {
// }

namespace muduo
{
namespace net
{
namespace detail
{

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
  //connector->
}

}  // namespace detail
}  // namespace net
}  // namespace muduo

//构造函数创建了一个Connector，毕竟是TcpClient嘛，需要一个东西来发起连接。并且注册连接成功的回调函数。 
TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr,
                     const string& nameArg)
  : loop_(CHECK_NOTNULL(loop)),
    connector_(new Connector(loop, serverAddr)),  // 重要的成员函数是connector_(用于发起连接)、connection_(当连接成功建立后，创建TcpConnection对象用于通信)
    name_(nameArg),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    retry_(false),
    connect_(true),
    nextConnId_(1)
{
  //连接建立成功回调函数
  connector_->setNewConnectionCallback(          //一旦连接建立连接，回调newConnection
      std::bind(&TcpClient::newConnection, this, _1));
  // FIXME setConnectFailedCallback
  LOG_INFO << "TcpClient::TcpClient[" << name_
           << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()  //析构函数
{
  LOG_INFO << "TcpClient::~TcpClient[" << name_
           << "] - connector " << get_pointer(connector_);
  TcpConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(mutex_);
    unique = connection_.unique();
    conn = connection_;  //将connection_保存到conn
  }
  if (conn)  //如果以已经建立连接
  {
    assert(loop_ == conn->getLoop());
    // FIXME: not 100% safe, if we are in different thread
    //CloseCallback的回调函数设置为detail::removeConnection然后调用 runInLoop
    CloseCallback cb = std::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(  //调用runInLoop 将cb设置进去
        std::bind(&TcpConnection::setCloseCallback, conn, cb));
    if (unique)
    {
      conn->forceClose();
    }
  }
  else  //connector处于未连接状态，将connector_停止即可
  {
    connector_->stop();
    // FIXME: HACK
    loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
  }
}

void TcpClient::connect()
{
  // FIXME: check state
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect()  //用于连接已经建立的情况下关闭连接
{
  connect_ = false;

  {
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      connection_->shutdown();
    }
  }
}

//停止connector_ 可能连接还没有建立成功 ，正处于发起连接的状态时  stop
void TcpClient::stop()
{
  connect_ = false;
  connector_->stop();
}



//该函数创建一个堆上局部TcpConnection对象，并用TcpClient的智能指针connection_保存起来，这样本函数中conn即便析构掉，connection_依然维护该连接。
//然后设置各种回调函数。由于为了避免busy loop，在Connector中一旦连接成功，我们取消关注sockfd的可写事件。并且本函数使用conn->connectEstablished()内部会关注可读事件：
void TcpClient::newConnection(int sockfd)
{
  loop_->assertInLoopThread();
  InetAddress peerAddr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary

  //创建一个TcpConnection对象，已连接的对象，智能指针。
  //根据Connector中的handleWrite()函数，连接建立后会把sockfd从poller中移除，以后不会再关注可写事件了
  //否则会出现busy loop，因为已连接套接字一直处于可写状态，对应水平触发来说
  TcpConnectionPtr conn(new TcpConnection(loop_,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));

  conn->setConnectionCallback(connectionCallback_);  //设置连接建立回调函数
  conn->setMessageCallback(messageCallback_);   //消息到达回调函数
  conn->setWriteCompleteCallback(writeCompleteCallback_); //数据发送完毕回调函数
  conn->setCloseCallback(  //关闭回调函数
      std::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
  {
    MutexLockGuard lock(mutex_);
    connection_ = conn;   //保存TcpConnection
  }
  conn->connectEstablished();  //这里会关注可读事件，并且回调connectionCallback_，
}

//连接断开函数
void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    MutexLockGuard lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();  //重置
  }
 //I/O线程中调用connectDestroyed销毁
  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_)   //是否发起重连
  {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toIpPort();
    connector_->restart();  //这里是指连接建立成功后又建立的重连
  }
}

