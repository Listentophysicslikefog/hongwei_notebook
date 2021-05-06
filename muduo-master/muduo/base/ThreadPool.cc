// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/ThreadPool.h"

#include "muduo/base/Exception.h"

#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const string& nameArg)  //构造函数对成员变量进行初始化，互斥锁条件变量的构造，任务队列的默认任务数量为0；
  : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{
}

ThreadPool::~ThreadPool()  //析构函数对线程池的状态进行判断，如果线程池不是关闭状态关闭线程
{
  if (running_)
  {
    stop();
  }
}

//start函数设置线程的数量，并设置线程的状态为running，并且通过 threads_.reserve(numThreads)设置线程池中线程的数量，同时设置线程的回调函数
void ThreadPool::start(int numThreads)  //启动线程池，线程个数固定
{
  assert(threads_.empty());  //初始化线程池，任务线程一定要为空
  running_ = true; // 在 ThreadPool::start() 里对 running_ 赋值不需要加锁，因为赋值之后才起工作线程，pthread_create() 自带 happens-before 语义。
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i+1);
    threads_.emplace_back(new muduo::Thread(
          std::bind(&ThreadPool::runInThread, this), name_+id));  // runInThread 线程池中的线程要执行的函数,  runInThread这个函数是从任务队列里面获取任务的。
    threads_[i]->start(); //启动线程
  }
  if (numThreads == 0 && threadInitCallback_)
  {
    threadInitCallback_();
  }
}

//stop函数，将线程池的状态修改为停止，同时等待所有的线程结束，看实现代码，没有对任务队列中为处理任务数量进行判断，如果停止，应该保证任务队列中的所有任务都处理结束，而muduo库的代码，只是将每个线程手中执行的任务处理完成，因此个人认为stop函数的实现不是很完美
void ThreadPool::stop()  //停止所有的线程
{
  {
  MutexLockGuard lock(mutex_);
  running_ = false;  // 在 ThreadPool::stop() 里对 running_ 赋值需要加锁，因为这时工作线程会读这个变量。  设置为false所有线程会停止
  notEmpty_.notifyAll();
  notFull_.notifyAll();
  }
  for (auto& thr : threads_)
  {
    thr->join();
  }
}

size_t ThreadPool::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return queue_.size();
}

//run函数将外部的任务放入任务队列，同时在放入任务队列时判断任务队列是否已满，如果已满就阻塞等待，直到任务队列有空余的空间
void ThreadPool::run(Task task)  //运行任务，往线程池的任务队列添加任务
{
  if (threads_.empty())
  {
    task(); // 如果工作线程里面没有任务那么直接执行该函数
  }
  else  //如果有任务，那么将现在的任务放到任务队列里，后面工作队列去获取任务，task应该是相应排序的。
  {
    MutexLockGuard lock(mutex_);
    while (isFull() && running_) //如果任务队列满了，那么需要等待，这个条件只是避免虚唤醒
    {
      notFull_.wait();  //条件变量 队列没有满会唤醒的
    }
    if (!running_) return;
    assert(!isFull());

    queue_.push_back(std::move(task));  //添加到任务队列,应该是缓存队列
    notEmpty_.notify();  //条件变量通知任务到来，通知缓存队列不为空
  }
}

//take函数，线程从线程池中取任务如果任务队列任务数量为空，那么就阻塞等待，直到有任务进入任务队列
ThreadPool::Task ThreadPool::take()  // 获取任务
{
  MutexLockGuard lock(mutex_);
  // always use a while-loop, due to spurious wakeup
  while (queue_.empty() && running_)
  {
    notEmpty_.wait();
  }
  Task task;
  if (!queue_.empty())
  {
    task = queue_.front();
    queue_.pop_front();
    if (maxQueueSize_ > 0)
    {
      notFull_.notify();
    }
  }
  return task;
}

bool ThreadPool::isFull() const
{
  mutex_.assertLocked();
  return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()  //任务线程
{
  try
  {
    if (threadInitCallback_)
    {
      threadInitCallback_();
    }
    while (running_)
    {
      Task task(take());  //take()获取任务，初始化task
      if (task)
      {
        task();  //执行task
      }
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw; // rethrow
  }
}



/*

由于muduo库是基于对象编程的设计理念，所以muduo库的任务队列中存放的都是回调函数，线程池的另外一种实现方式就是将muduo库中任务队列换成一个taskbase的基类指针，通过虚函数多态来实现对任务的动态调用，但是如果是这样设计的话，muduo库就必须提供一个taskbase的基类，外部的使用者就必须继承该基类才能够使用muduo库。
各有各的好处吧，如果我们是在一个项目中设计一个ThreadPool的话，个人认为通过基类指针这种方式设计任务队列更好些，如果是封装一个框架类型的话还是muduo库的这种更加合理吧，使外部threadpool的使用者之关系自己的业务设计，而无需与threadpool做匹配。

*/