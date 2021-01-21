// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/poller/PollPoller.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Types.h"
#include "muduo/net/Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>

using namespace muduo;
using namespace muduo::net;

PollPoller::PollPoller(EventLoop* loop)
  : Poller(loop)
{
}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  // XXX pollfds_ shouldn't change                   //结构体数组大小   超时时间       通道在这个结构体数组里面
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);  //调用linux的poll函数，第一个参数相当于是结构体的首地址
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0)   //numEvents 返回的事件数目
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);   //将事件numEvents放到通道里面activeChannels然后返回回去
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << " nothing happened";
  }
  else
  {
    if (savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "PollPoller::poll()";
    }
  }
  return now;
}

void PollPoller::fillActiveChannels(int numEvents,
                                    ChannelList* activeChannels) const
{
  for (PollFdList::const_iterator pfd = pollfds_.begin();     // 遍历存放事件的结构体数组 pollfds_
      pfd != pollfds_.end() && numEvents > 0; ++pfd)
  {
    if (pfd->revents > 0)   //revents是linux  poll函数第一个参数的结构体成员变量  revents表示返回的事件
    {
      --numEvents;   //处理一个后，事件的数目减一
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);  //通过文件描述符去查找对应的通道     channels_是map key是文件描述符  value是Channel* 
      assert(ch != channels_.end());   //断言 该文件描述符对应得 通道一定存在
      Channel* channel = ch->second;   //迭代器得second 就是value 
      assert(channel->fd() == pfd->fd);  //断言channel得fd和pfd得fd相等
      channel->set_revents(pfd->revents);  //将pfd的revents就是poll或者epoll返回的实际事件存放到channel的成员变量里
      // pfd->revents = 0;
      activeChannels->push_back(channel);  //将channel存放到  ChannelList里面
    }
  }
}

void PollPoller::updateChannel(Channel* channel)   //注册或者更新通道  比如注册某个事件的可读可写事件
{
  Poller::assertInLoopThread();  //断言在io线程中调用才可以，就是在poll的所属EventLoop线程调用才可以
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0)   //index小于0表示我们注册的是一个新的通道
  {
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());  //新通道先断言在之前存放通道的数组里 查找不到
    struct pollfd pfd;  //准备一个pollfd
    pfd.fd = channel->fd();  //给pollfd类型的结构体pfd成员变量赋值 
    pfd.events = static_cast<short>(channel->events());  //event等于channel的event
    pfd.revents = 0;   //返回的事件先赋值为0
    pollfds_.push_back(pfd); //放到末尾
    int idx = static_cast<int>(pollfds_.size())-1;  //新增加通道的key大于0
    channel->set_index(idx);  //设置index
    channels_[pfd.fd] = channel;  //将新增加的通道存到map里面 key是文件描述符value是channel
  }
  else   //表示更新一个已经存在的通道
  {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());  //断言这个通道已经存在
    assert(channels_[channel->fd()] == channel);  //断言 map里存放的通道等于作为参数传入的通道
    int idx = channel->index();   //获取index  取出channel的下标
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));  //断言通道的下标大于0并且小于存放通道的vector的长度
    struct pollfd& pfd = pollfds_[idx];  //引用不需要拷贝  取出下标为index的pfd
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);  //跟新通道的时候需要  pfd.fd == -channel->fd()-1是因为暂时忽略该文件描述符事件 设置文件描述符为 -hannel->fd()-1
    pfd.fd = channel->fd();  //跟新fd                                                                // 设置为相反数减1是避免0的相反数还是0，所以无法忽略该文件描述符
    pfd.events = static_cast<short>(channel->events());  //更新请求的事件
    pfd.revents = 0;  //更新返回事件设置为0
    if (channel->isNoneEvent())  //将一个通道暂时更改为不关注事件，但不从poll中移除该通道
    {
      // ignore this pollfd
      //暂时忽略该文件描述符事件 这里可以将pfd.fd设置为-1
      pfd.fd = -channel->fd()-1;  //这样是为了removechannel优化
    }
  }
}

void PollPoller::removeChannel(Channel* channel)  //真正移除该事件
{
  Poller::assertInLoopThread();  //断言在io线程中调用才可以，就是在poll的所属EventLoop线程调用才可以
  LOG_TRACE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());  //断言该channel已经在map里存在
  assert(channels_[channel->fd()] == channel); //断言传入的channel和存放该channel的map里value相同
  assert(channel->isNoneEvent());  //断言该channel已经没有关注事件
  int idx = channel->index(); //去出index   在数组的位置
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));//断言通道的下标大于等于0并且小于存放通道的vector的长度
  const struct pollfd& pfd = pollfds_[idx]; (void)pfd;  //取出pfd
  assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());  //断言pfd等于 -channel->fd()-1并且  pfd.events == channel->events()
  size_t n = channels_.erase(channel->fd());  //从map里移除该通道，这里通过key来移除   返回移除的通道个数   返回一定是1
  assert(n == 1); (void)n;   //断言n为1
  if (implicit_cast<size_t>(idx) == pollfds_.size()-1)  //表示该通道是最存放的map的后一
  {
    pollfds_.pop_back();  //直接pop_back()
  }
  else  //否则不是最后一个怎么移除
  {
    //这里的移除算法复杂度是o(1),将待删除的元素与最后一个元素交换再pop_back()
    int channelAtEnd = pollfds_.back().fd;   //获取最后一个元素的fd
    iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);  //将将待删除的元素与最后一个元素交换
    if (channelAtEnd < 0)
    {
      channelAtEnd = -channelAtEnd-1;  //如果小于0我们要把它还原回来    因为后面channelAtEnd会做为数组的下标
    }
    channels_[channelAtEnd]->set_index(idx);  //交换后更新index
    pollfds_.pop_back(); //将最后一个去除
  }
}

