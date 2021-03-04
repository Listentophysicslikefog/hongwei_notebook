// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo
{

class CountDownLatch : noncopyable
{
 public:
 
  explicit CountDownLatch(int count);   //构造函数

  void wait();    //等待

  void countDown();   //计数器减一，减为0的时候就发起通知，向所有线程发起通知

  int getCount() const;   //获取计数器值

 private:
  mutable MutexLock mutex_;
  Condition condition_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);   //计数器  count_
};

}  // namespace muduo
#endif  // MUDUO_BASE_COUNTDOWNLATCH_H



//该类既可以用于所有子线程等待主线程发起起跑命令也可以用于主线程等待子线程初始化完毕才开始工作