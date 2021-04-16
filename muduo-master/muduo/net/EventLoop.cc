// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/EventLoop.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Poller.h"
#include "muduo/net/SocketsOps.h"
#include "muduo/net/TimerQueue.h"

#include <algorithm>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{                                     //初始化为空指针，主要调用该头文件就有该变量
__thread EventLoop* t_loopInThisThread = 0;  //__thread已经介绍过了  当前线程EventLoop对象指针，当前线程的局部存储，因为有__thread，否则是所有线程共享

const int kPollTimeMs = 10000;

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}
//用户通过EventLoop创建的对象和上面的EventLoop创建的对象不同，
EventLoop::EventLoop()   //创建对象的时候会调用这个 构造函数
  : looping_(false),    //初始化为false，没有处于事件循环
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    iteration_(0),
    threadId_(CurrentThread::tid()),  //把当前线程的id初始化给threadid,每一个线程最多有一个EvevntLoop对象，主函数也可以直接创建EventLoop对象，因为主函数也是一个主线程
    poller_(Poller::newDefaultPoller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentActiveChannel_(NULL)
{
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread)  //判断当前的线程是否已经有EventLoop对象，如果有就报错， t_loopInThisThread是一个线程独有的，如果一个线程的t_loopInThisThread不为nil，那么这个线程就有一个EventThread对象了，就报错
  {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;  //this是创建EventLoop对象的类的this指针，就是指向当前创建EventLoop对象的指针，就是EventLoop创建的对象本身
  }
  wakeupChannel_->setReadCallback(
      std::bind(&EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()  //析构函数
{
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;  //将这个线程的这个独有变量置空
}

void EventLoop::loop()  //事件循环，该函数不可以跨线程调用，只可以在创建该对象的线程中调用
{
  assert(!looping_);  //断言是否在处于事件循环
  assertInLoopThread();  //断言创建的对象处于创建该对象的线程中  ，断言成功就进行下一步
  looping_ = true;   //把looping_赋值为true
  quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
  LOG_TRACE << "EventLoop " << this << " start looping";   //打印信息，表示开始循环

  while (!quit_)   //应该是让工作线程一直工作，一个while循环
  {
    activeChannels_.clear();  //先清空一下，避免里面有数据， 这个是存放poller返回的活动通道，就是产生事件的通道
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);  //poller_是poller对象的智能指针，封装poll或者epoll。调用poll返回活动的通道  ，传的是 &activeChannels_，将活跃通道存放到这里
    ++iteration_;
    if (Logger::logLevel() <= Logger::TRACE)  //日志级别
    {
      printActiveChannels();  //打印活动通道
    }
    // TODO sort channel by priority
  eventHandling_   = true;  //当前是否处于事件的处理状态，这里赋值为ture，接下来就要处理了
    for (Channel* channel : activeChannels_)   //遍历通道
    {
      currentActiveChannel_ = channel;  //每遍历一个通道，将currentActiveChannel_当前正在处理的活跃通道赋值
      currentActiveChannel_->handleEvent(pollReturnTime_); //调用handleEvent来处理这个通道，handleEvent是Channel的成员
    }
    currentActiveChannel_ = NULL;  //for循环全部处理完后置NULL
    eventHandling_ = false;   //置为false
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread())
  {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb)
{
  if (isInLoopThread())
  {
    cb();
  }
  else
  {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor cb)
{
  {
  MutexLockGuard lock(mutex_);
  pendingFunctors_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_)
  {
    wakeup();
  }
}

size_t EventLoop::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
  return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel)  //loop的updateChannel函数实际是调用了poller_的updateChannel
{
  assert(channel->ownerLoop() == this);   //断言channel所属的对象应该属于本对象
  assertInLoopThread();  //断言在EventLoop线程当中
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_)
  {
    assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()   //创建的EventLoop对象没有在创建该对象的线程中，终止程序
{
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
  MutexLockGuard lock(mutex_);
  functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors)
  {
    functor();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
  for (const Channel* channel : activeChannels_)  //遍历活动通道
  {
    LOG_TRACE << "{" << channel->reventsToString() << "} ";  //输出活动通道
  }
}


//每个线程最多只有一个EventLoop对象
