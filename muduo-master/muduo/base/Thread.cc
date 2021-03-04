// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Thread.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Exception.h"
#include "muduo/base/Logging.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace muduo
{
namespace detail
{

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));   //通过系统调用获取tid
}

void afterFork()
{
  muduo::CurrentThread::t_cachedTid = 0;   //当前线程的tid为0
  muduo::CurrentThread::t_threadName = "main";   //当前线程名字为main
  CurrentThread::tid();   //缓存一下
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()  //构造函数
  {
    muduo::CurrentThread::t_threadName = "main";   //主线程的名称为main
    CurrentThread::tid();    //缓存当前线程的tid
    pthread_atfork(NULL, NULL, &afterFork);   //该函数作用可以查询，截图有例子
  }   //afterFork如果调用fork函数成功后，子进程会调用afterFork函数，为什么要这样做：因为在fork之前可能有多个线程，所以fork可能在主线程调用或者子线程中调用
};  //如果fork在子线程调用那么fork得到一个新进程，新进程只有一个执行序列，也就是只有一个线程(调用fork的线程被继承下来)，那么这个线程其实是主线程那么我们希望改变一下这个线程的名称和tid，所以子线程就调用这个函数

ThreadNameInitializer init;   //在detail命名空间里的全局变量，这个函数的构造先于main，一开始就进入构造

struct ThreadData
{
  typedef muduo::Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(ThreadFunc func,
             const string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : func_(std::move(func)),
      name_(name),
      tid_(tid),
      latch_(latch)
  { }

  void runInThread()   //会调用回调函数ThreadFunc
  {
    *tid_ = muduo::CurrentThread::tid();   //会先缓存tid后再返回tid，提高获取tid的效率
    tid_ = NULL;
    latch_->countDown();
    latch_ = NULL;

    muduo::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();   //将线程的名称也缓存起来
    ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);
    try
    {
      func_();   //调用回调函数
      muduo::CurrentThread::t_threadName = "finished";
    }
    catch (const Exception& ex)  //异常捕捉
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw; // rethrow
    }
  }
};

void* startThread(void* obj)  //线程的入口函数
{
  ThreadData* data = static_cast<ThreadData*>(obj);   //转换为基类指针
  data->runInThread();     //入口函数调用runInThread()
  delete data;
  return NULL;
}

}  // namespace detail

void CurrentThread::cacheTid()   //缓存tid
{
  if (t_cachedTid == 0)   //说明我们还没有缓存过,然后获取后缓存到t_cachedTid
  {
    t_cachedTid = detail::gettid();   //detail名称空间里，通过系统调用获取线程的真实pid，也就是tid
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread()  //判断当前线程是否为主线程
{
  return tid() == ::getpid();  //查看tid是否等于当前进程id，如果是那么就是主线程
}

void CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(ThreadFunc func, const string& n)   //构造函数
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadId_);
  }
}

void Thread::setDefaultName()
{
  int num = numCreated_.incrementAndGet();
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()  //启动线程
{
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);  //线程的详细信息
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))  //这样创建一个线程函数 ，执行startThread函数，就是线程的入口函数
  {
    started_ = false;
    delete data; // or no delete?
    LOG_SYSFATAL << "Failed in pthread_create";
  }
  else
  {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()  //在一个线程中，开了另一个线程去干另一件事，使用join函数后，原始线程会等待新线程执行结束之后，再去销毁线程对象。
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}

}  // namespace muduo
