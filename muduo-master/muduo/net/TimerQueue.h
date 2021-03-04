// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "muduo/base/Mutex.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.  一定是线程安全的，可以跨线程调用，通常被其他线程调用
  //其他线程表示不在所属的EventLoop对象的线程     一个TimerQueue属于一个EvenLoop对象(在某一个io线程创建的)
  TimerId addTimer(TimerCallback cb,   //添加一个定时器
                   Timestamp when,
                   double interval);

  void cancel(TimerId timerId);   //取消一个定时器

 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  typedef std::pair<Timestamp, Timer*> Entry; // key: Timestamp  value: 一个Timer定时器对象  便于快速查找已到期的定时器
  typedef std::set<Entry> TimerList;  //将pair存放到set  key : Entry   TimerList按照Timestamp排序
  typedef std::pair<Timer*, int64_t> ActiveTimer;   //和Entry保存的是同一个东西，定时器列表
  typedef std::set<ActiveTimer> ActiveTimerSet;   //只不过这个是按照对象地址排序Timer*

  void addTimerInLoop(Timer* timer); //只可以在其所属线程调用，所以不许加锁
  void cancelInLoop(TimerId timerId); // 只可以在其所属线程调用
  // called when timerfd alarms
  void handleRead();  //当定时器事件到来时可读事件产生，会回调这个函数
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);  //返回超时定时器列表
  void reset(const std::vector<Entry>& expired, Timestamp now);  //如果时重置定时器，我们需要重置

  bool insert(Timer* timer);

  EventLoop* loop_;    //所属的EventLoop
  const int timerfd_;  //timer_create创建的定时器文件描述符
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;   //按照时间排序

  // for cancel()
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_; /* atomic */   //是否处于调用处理槽式的定时器当中
  ActiveTimerSet cancelingTimers_;  //保存被取消的定时器
};

}  // namespace net
}  // namespace muduo
#endif  // MUDUO_NET_TIMERQUEUE_H
