// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Types.h"

#include <deque>
#include <vector>

namespace muduo
{

class ThreadPool : noncopyable
{
 public:
  typedef std::function<void ()> Task;

  explicit ThreadPool(const string& nameArg = string("ThreadPool"));  //构造函数对成员变量进行初始化，互斥锁条件变量的构造，任务队列的默认任务数量为0；
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
  void setThreadInitCallback(const Task& cb)
  { threadInitCallback_ = cb; }

  void start(int numThreads);
  void stop();

  const string& name() const
  { return name_; }

  size_t queueSize() const;

  // Could block if maxQueueSize > 0
  // Call after stop() will return immediately.
  // There is no move-only version of std::function in C++ as of C++14.
  // So we don't need to overload a const& and an && versions
  // as we do in (Bounded)BlockingQueue.
  // https://stackoverflow.com/a/25408989
  void run(Task f);

 private:
  bool isFull() const REQUIRES(mutex_);
  void runInThread();
  Task take();

  mutable MutexLock mutex_;
  Condition notEmpty_ GUARDED_BY(mutex_);  //两个条件变量，notEmpty_，notFull_用来判断任务池中任务的数量
  Condition notFull_ GUARDED_BY(mutex_);
  string name_;
  Task threadInitCallback_;  //线程的初始化函数，可以没有，就是不用初始化
  std::vector<std::unique_ptr<muduo::Thread>> threads_; // 存放工作线程  线程队列，内部存储的线程指针
  std::deque<Task> queue_ GUARDED_BY(mutex_);  //任务对列，内部元素类型为typedef boost::function<void ()> Task;是一个任务执行函数
  size_t maxQueueSize_;
  bool running_;  //线程池是否运行
};

}  // namespace muduo

#endif  // MUDUO_BASE_THREADPOOL_H
