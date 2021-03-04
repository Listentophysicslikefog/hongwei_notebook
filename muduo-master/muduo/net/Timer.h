// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include "muduo/base/Atomic.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"

namespace muduo
{
namespace net
{

///
/// Internal class for timer event.
///
class Timer : noncopyable
{
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)// 构造函数
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(s_numCreated_.incrementAndGet())   //序号原子加一，即使是多线程也保证是唯一的
  { }

  void run() const
  {
    callback_();
  }

  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  void restart(Timestamp now);// 重启

  static int64_t numCreated() { return s_numCreated_.get(); }

 private:
  const TimerCallback callback_;  //定时器回调函数
  Timestamp expiration_;   //下一次的超时时刻，就是下一次到达定时的时间
  const double interval_;  //超时时间间隔，如果是一次性定时器，该值为0
  const bool repeat_;   // 是否重复
  const int64_t sequence_;  // 定时器序号，每一个定时器都有唯一的

  static AtomicInt64 s_numCreated_;   //定时器计数，当前已创建的定时器数量   原子操作
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TIMER_H
