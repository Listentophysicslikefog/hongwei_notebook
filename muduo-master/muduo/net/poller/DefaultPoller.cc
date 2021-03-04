// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/Poller.h"
#include "muduo/net/poller/PollPoller.h"
#include "muduo/net/poller/EPollPoller.h"

#include <stdlib.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{     
  if (::getenv("MUDUO_USE_POLL"))  //https://baike.baidu.com/item/getenv/935515?fr=aladdin
  {
    return new PollPoller(loop);
  }
  else
  {
    return new EPollPoller(loop);   //默认使用epoll， 需要传递一个  EventLoop 对象
  }
}
