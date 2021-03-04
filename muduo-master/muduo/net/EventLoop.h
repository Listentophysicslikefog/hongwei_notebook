// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "muduo/base/Mutex.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/TimerId.h"

namespace muduo
{
namespace net
{

class Channel;   //前向声明
class Poller;  //前向声明  
class TimerQueue;  //前向声明

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop : noncopyable
{
 public:
  typedef std::function<void()> Functor;

  EventLoop();    //构造函数
  ~EventLoop();  // force out-line dtor, for std::unique_ptr members.   析构函数

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();   //事件循环

  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  ///
  /// Time when poll returns, usually means data arrival.
  ///
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const { return iteration_; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(Functor cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(Functor cb);

  size_t queueSize() const;

  // timers

  ///
  /// Runs callback at 'time'.
  /// Safe to call from other threads.
  ///
  TimerId runAt(Timestamp time, TimerCallback cb);   // 在某个时刻运行定时器
  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  ///
  TimerId runAfter(double delay, TimerCallback cb);    //过一段时间运行定时器
  ///
  /// Runs callback every @c interval seconds.
  /// Safe to call from other threads.
  ///
  TimerId runEvery(double interval, TimerCallback cb);  // 每隔一段时间运行定时器
  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  void cancel(TimerId timerId);   // 取消定时器

  // internal usage
  void wakeup();
  void updateChannel(Channel* channel);  //在poller中添加或者跟新通道
  void removeChannel(Channel* channel);   //从poller中移除通道
  bool hasChannel(Channel* channel);

  // pid_t threadId() const { return threadId_; }
  void assertInLoopThread()
  {
    if (!isInLoopThread())  //判断是否处于在创建该对象的线程当中
    {
      abortNotInLoopThread();   //如果不是就终止程序，如果是就什么都不执行
    }
  }
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }   //通过线程id和当前的EventLoop对象所属的线程id比较是否相同
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return eventHandling_; }

  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead();  // waked up
  void doPendingFunctors();

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel*> ChannelList;   //typedef vector数组

  bool looping_; /* atomic */    //判断是否处于事件循环
  std::atomic<bool> quit_;    //是否退出
  bool eventHandling_; /* atomic */    //当前是否处于事件的处理状态
  bool callingPendingFunctors_; /* atomic */
  int64_t iteration_;
  const pid_t threadId_;      //线程id，记录当前对象属于哪一个线程id
  Timestamp pollReturnTime_;    //调用poller函数所返回的时间戳
  std::unique_ptr<Poller> poller_;     //poller的生存周期由EventLoop控制，这里有智能指针就不需要调用delete
  std::unique_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  std::unique_ptr<Channel> wakeupChannel_;
  boost::any context_;

  // scratch variables
  ChannelList activeChannels_;    //poller返回的活动通道，就是产生事件的通道
  Channel* currentActiveChannel_;   //当前正在处理的活跃通道

  mutable MutexLock mutex_;
  std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_H
