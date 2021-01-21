// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/Timestamp.h"

#include <functional>
#include <memory>

namespace muduo
{
namespace net
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.  //这个类不拥有文件描述符
/// The file descriptor could be a socket,    //文件描述符被socket拥有了 ，文件描述符可以是an eventfd, a timerfd, or a signalfd
/// an eventfd, a timerfd, or a signalfd   
class Channel : noncopyable
{
 public:
  typedef std::function<void()> EventCallback;   //事件的回调处理
  typedef std::function<void(Timestamp)> ReadEventCallback;   //读事件的回调处理，读事件需要多传一个时间戳
                                     //Channel只可以在一个EventLoop线程处理
  Channel(EventLoop* loop, int fd);  //构造函数需要传一个EventLoop，一个EventLoop可以包含多个Channel，但是一个Channel只由一个EventLoop负责
  ~Channel();

  void handleEvent(Timestamp receiveTime);  //handleEvent函数负责对所发生的io事件进行处理
  void setReadCallback(ReadEventCallback cb)  //读的回调函数
  { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb)  //写的回调函数
  { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb)   //关闭的回调函数
  { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb)  //错误的回调函数
  { errorCallback_ = std::move(cb); }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const std::shared_ptr<void>&);

  int fd() const { return fd_; }  //fd_是Channel对应得文件描述符
  int events() const { return events_; }   //保存Channel注册的事件，也就是关注的事件
  void set_revents(int revt) { revents_ = revt; } // used by pollers
  // int revents() const { return revents_; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }  //判断这个events_是否为没有事件

  void enableReading() { events_ |= kReadEvent; update(); }  //关注读事件events_或上kReadEvent，enableReading 会导致channel 的update()，进而导致loop的updateChannel()----> 进而导致poll/epoll的updateChannel(),  这个去文字描述看，后面补充完
  void disableReading() { events_ &= ~kReadEvent; update(); }  //不关注读事件
  void enableWriting() { events_ |= kWriteEvent; update(); }  //关注写事件
  void disableWriting() { events_ &= ~kWriteEvent; update(); }  //不关注写事件了
  void disableAll() { events_ = kNoneEvent; update(); }   //不关注所有事件了
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const { return events_ & kReadEvent; }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  // for debug
  string reventsToString() const;
  string eventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  static string eventsToString(int fd, int ev);

  void update();
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;  //没有事件  cc里面初始化
  static const int kReadEvent;  //读事件 
  static const int kWriteEvent;  //写事件

  EventLoop* loop_;  //本Channel所属的EventLoop
  const int  fd_;
  int        events_;   //注册的事件，也就是关注的事件
  int        revents_; // it's the received event types of epoll or poll   epoll或者poll实际返回的事件，就是存放和接收epoll和poll得返回得事件
  int        index_; // used by Poller.  //截图介绍了
  bool       logHup_;   //用于pollhup，应该是用于输出日志的

  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;   //是否处于处理事件中
  bool addedToLoop_;
  ReadEventCallback readCallback_;  //读事件回调
  EventCallback writeCallback_;   //写事件回调
  EventCallback closeCallback_;  //事件关闭回调
  EventCallback errorCallback_;  //错误事件回调
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_CHANNEL_H



/*

Channel是对io注册事件的响应封装他的 update函数负责注册或者更新io的可读或者可写事件、它的HandleEvent函数负责对所发生的io事件进行处理。



*/