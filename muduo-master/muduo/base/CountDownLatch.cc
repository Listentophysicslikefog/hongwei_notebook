// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/CountDownLatch.h"

using namespace muduo;

CountDownLatch::CountDownLatch(int count)  //构造函数实现
  : mutex_(),  //构造一个mutex_对象  
    condition_(mutex_),  //把mutuex_对象传到condition_里面，condition_不负责mutuex_对象的生存周期
    count_(count)   //初始化计数
{
}

void CountDownLatch::wait()  //等待，就是条件变量的使用
{
  MutexLockGuard lock(mutex_);
  while (count_ > 0)  //当count_大于0我们就等待，这个while只是避免虚唤醒
  {
    condition_.wait();   //其实是通过这个来阻塞等待的
  }
}

void CountDownLatch::countDown()
{
  MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0)  //如果count==0就通知所有的等待线程，应该是通知该条件变量释放了
  {
    condition_.notifyAll();
  }
}

int CountDownLatch::getCount() const  //获取count_值  const成员函数是不可以改变非const成员变量的
{
  MutexLockGuard lock(mutex_);   //这里MutexLockGuard的lock改变了mutex_，是因为添加了mutable，mutable MutexLock mutex_;
  return count_;
}

