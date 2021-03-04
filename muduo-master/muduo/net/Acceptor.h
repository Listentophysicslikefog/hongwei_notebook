// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <functional>

#include "muduo/net/Channel.h"
#include "muduo/net/Socket.h"

namespace muduo
{
namespace net
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : noncopyable
{
 public:
  typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }   //设置回调函数

  void listen();

  bool listening() const { return listening_; }  //监听socket

  // Deprecated, use the correct spelling one above.
  // Leave the wrong spelling here in case one needs to grep it for error messages.
  // bool listenning() const { return listening(); }

 private:
  void handleRead();

  EventLoop* loop_;   //channel所属的EventLoop
  Socket acceptSocket_;  // 是监听套接字也就是服务端套接字
  Channel acceptChannel_;  //用来观察 上面套接字的事件的可读事件，可读事件活跃状态时poller::poll返回这个活跃的通道，活跃通道回调Accptor::HandleRead()-->调用accept(2)来接收新的连接并且调用用户的callback 。 这个channel还可以设置一个可读事件的回调函数
  NewConnectionCallback newConnectionCallback_;
  bool listening_;   //是否处于监听状态
  int idleFd_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_ACCEPTOR_H
