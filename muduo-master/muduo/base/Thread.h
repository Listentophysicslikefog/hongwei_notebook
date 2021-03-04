// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include "muduo/base/Atomic.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Types.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace muduo
{

class Thread : noncopyable
{
 public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(ThreadFunc, const string& name = string());
  // FIXME: make it movable in C++11
  ~Thread();

  void start();
  int join(); // return pthread_join()

  bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();

  bool       started_;    //表示线程是否启动
  bool       joined_;
  pthread_t  pthreadId_;  //线程的真实pid
  pid_t      tid_;
  ThreadFunc func_;   //该线程要回调的函数，类型是ThreadFunc
  string     name_;  //线程的名称
  CountDownLatch latch_;    

  static AtomicInt32 numCreated_;  //已经创建线程的个数
};

}  // namespace muduo
#endif  // MUDUO_BASE_THREAD_H


//muduo线程类实现 是基于对象实现
