// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include "muduo/base/Mutex.h"

#include <pthread.h>

namespace muduo
{

class Condition : noncopyable
{
 public:
  explicit Condition(MutexLock& mutex)   //构造函数
    : mutex_(mutex)
  {
    MCHECK(pthread_cond_init(&pcond_, NULL));   //初始化条件变量
  }

  ~Condition()  //析构函数
  {
    MCHECK(pthread_cond_destroy(&pcond_));   //销毁条件变量
  }

  void wait()  //等待函数
  {
    MutexLock::UnassignGuard ug(mutex_);
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));   //等待函数调用pthread_cond_wait()
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(double seconds);   //等待超时时间

  void notify()  //信号函数调用的signal
  {
    MCHECK(pthread_cond_signal(&pcond_));   //传入条件变量
  }

  void notifyAll()  //broadcast函数调用broadcast
  {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;   //条件变量
};

}  // namespace muduo

#endif  // MUDUO_BASE_CONDITION_H



//条件变量使用，一般需要和mutuex配合
/*

线程 ：A
锁住 mutuex
    while(条件)  //while避免误唤醒，其实不是通过while阻塞的
      等待  //等待信号，这里不会一直运行while循环，等待条件变量
解锁 mutuex

线程B：
锁住 mutuex
更改条件
signal或broadcast   //发送信号

解锁 mutex




*/
