// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/EventLoopThread.h"

#include "muduo/net/EventLoop.h"

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
  : loop_(NULL),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name), //1.  构造一个thread_对象，指定的回调函数是threadFunc ，并且把自身对象this传入
    mutex_(),
    cond_(mutex_),  //2. 初始化条件变量，条件变量总是和互斥量一块使用
    callback_(cb)  // 3. 初始化callback_函数
{
}

EventLoopThread::~EventLoopThread()
{
  exiting_ = true;
  if (loop_ != NULL) // not 100% race-free, eg. threadFunc could be running callback_.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    loop_->quit();  //4. 退出io线程， io线程的loop循环退出，从而退出io线程
    thread_.join(); 
  }
}

EventLoop* EventLoopThread::startLoop()  //5.  启动io线程   启动io线程，并且这个线程成为了io线程          
{
  assert(!thread_.started());
  thread_.start(); // 6. 这个线程启动后会调用这个线程的回调函数，我们绑定的是 threadFunc       这个时候就有两个线程： 1. 调用startLoop()的线程  2. 我们才创建的线程threadFunc()   他们是并发执行的

  EventLoop* loop = NULL;
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL)
    {
      cond_.wait();      //6-->7. 所以用了条件变量，等待 loop_ 不为null ，如果为null就一直等待， loop_ 不为null就表示threadFunc()线程运行起来了
    }
    loop = loop_;
  }

  return loop;  // 8. 返回 EventLoop*
}

void EventLoopThread::threadFunc()
{
  EventLoop loop;

  if (callback_)   // 如果我们传递了这个 初始化函数那么就会先调用这个初始化函数，如果没有传递构造函数默认为空的
  {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;  // loop_ 指针指向一个栈上的对象，threadFunc()函数退出后，这个指针就失效了
                    // 这样没有问题，threadFunc()函数退出后就是意味着线程退出了，线程退出了那么这个eventloopthread线程对象就没有存在的价值了，线程都结束了也不会去防问这个loop_,那么不销毁loop_没有问题
    cond_.notify();
  }

  loop.loop();  //如果这个loop退出后就表示这个io线程就退出了
  //assert(exiting_);
  MutexLockGuard lock(mutex_);
  loop_ = NULL;
}

