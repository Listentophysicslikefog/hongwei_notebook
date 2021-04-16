// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_EVENTLOOPTHREAD_H
#define MUDUO_NET_EVENTLOOPTHREAD_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

namespace muduo
{
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),  //默认是空的回调函数，就是没有回调函数
                  const string& name = string());
  ~EventLoopThread();
  EventLoop* startLoop(); // 4. 启动线程，该线程成为了io线程

 private:
  void threadFunc();  // 3. 线程函数    5. 这个线程运行起来后会在函数内部创建一个eventloop对象，让loop_指针指向这个对象

  EventLoop* loop_ GUARDED_BY(mutex_);  // 2. loop_指针指向一个eventloop对象
  bool exiting_;  // 6. 是否退出
  Thread thread_;   // 1. 基于对象编程 包含一个thread 对象
  MutexLock mutex_; 
  Condition cond_ GUARDED_BY(mutex_);  // 7. 条件变量
  ThreadInitCallback callback_;  // 回调函数在eventloop::loop（就是本类的loop_）事件循环之前被调用
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREAD_H





// 任何线程主要创建并运行了eventloop都称为io线程

// io程不一定是主线程

// 一个io线程只可以有一个eventloop  一个程序可以有多个io线程(io线程池管理)， 这些io线程可以由io线程池管理

// muduo并发模型 one loop per thread + threadpool(计算线程)   这个可以用个线程池管理：io线程池 和 计算线程池

// 定义eventloopthread类 封装io线程 ， eventloopthread类主要作用： 1. 创建一个线程  2. 在线程函数中创建一个eventloop对象并调用 eventloop::loop