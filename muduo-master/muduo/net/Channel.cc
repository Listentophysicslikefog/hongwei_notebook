// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"

#include <sstream>

#include <poll.h>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;  //没有事件
const int Channel::kReadEvent = POLLIN | POLLPRI; //读事件  POLLPRI表示紧急数据
const int Channel::kWriteEvent = POLLOUT;  //写事件

Channel::Channel(EventLoop* loop, int fd__)  //构造函数
  : loop_(loop),
    fd_(fd__),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{
}

Channel::~Channel()
{
  assert(!eventHandling_);
  assert(!addedToLoop_);
  if (loop_->isInLoopThread())
  {
    assert(!loop_->hasChannel(this));
  }
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Channel::update()  //update函数，就是调用EventLoop对象loop的updateChannel
{
  addedToLoop_ = true;
  loop_->updateChannel(this);//loop调用它的成员函数
}

void Channel::remove()
{
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)  //事件到来会调用handleEvent函数
{
  std::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      handleEventWithGuard(receiveTime);
    }
  }
  else
  {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
  eventHandling_ = true;  //是否处于处理事件中，开始处理赋值为true
  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN))   // 判断返回的事件revents_   读的时候有 POLLHUP表示socket的另一端关闭时,或读到文件结尾，会收到pollhup事件就关闭  。判断返回的事件revents_   要确保读的时候没有POLLIN，读的时候没有这个事件的
  {
    if (logHup_)   //这个值为true就打印日志
    {
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (closeCallback_) closeCallback_();   //调用close
  }

  if (revents_ & POLLNVAL)  //表示文件描述符没有打开或者不是一个合法的文件描述符
  {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";  //记录日志，服务器要一直工作
  }

  if (revents_ & (POLLERR | POLLNVAL))  //表示错误的，就回调错误的函数
  {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))  //POLLIN  POLLPRI表示可读事件  POLLRDHUP  表示对等方关闭连接 我们都把这个表示为刻度事件
  {
    if (readCallback_) readCallback_(receiveTime);   //我们调用回调函数
  }
  if (revents_ & POLLOUT)  //POLLOUT 表示可写事件发生
  {
    if (writeCallback_) writeCallback_();   //调用可写事件的回调函数
  }
  eventHandling_ = false;//是否处于处理事件中，处理完毕赋值为false
}

string Channel::reventsToString() const   //将事件转为字符串
{
  return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
  return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)  //将事件转换为字符串
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
    oss << "IN ";
  if (ev & POLLPRI)
    oss << "PRI ";
  if (ev & POLLOUT)
    oss << "OUT ";
  if (ev & POLLHUP)
    oss << "HUP ";
  if (ev & POLLRDHUP)
    oss << "RDHUP ";
  if (ev & POLLERR)
    oss << "ERR ";
  if (ev & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}
