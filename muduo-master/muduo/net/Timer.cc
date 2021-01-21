// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/Timer.h"

using namespace muduo;
using namespace muduo::net;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
  if (repeat_) // 是重复的计时器
  {//重新计算下一个超时时刻
    expiration_ = addTime(now, interval_);  //将当前时间加上超时时间间隔
  }
  else  //不是重复的定时器
  {
    expiration_ = Timestamp::invalid();  //下一个超时时刻是非法时间
  }
}
