// Copyright (c) 2020 UCloud All rights reserved.
#ifndef FRAMEWORK_ADAPTER_
#define FRAMEWORK_ADAPTER_

#ifdef ENABLE_UEVENT

#include <uevent/base/ini_config.h>
#include <uevent/base/logging.h>
#include <uevent/base/timestamp.h>

#define XLOG_TRACE LOG_TRACE
#define XLOG_DEBUG LOG_DEBUG
#define XLOG_INFO LOG_INFO
#define XLOG_WARN LOG_WARN
#define XLOG_ERROR LOG_ERROR
#define XLOG_FATAL LOG_FATAL
#define XLOG_SYSERR LOG_SYSERR
#define XLOG_SYSFATAL LOG_SYSFATAL

#else

#include <ustevent/base/ini_config.h>
#include <ustevent/base/logging.h>
#include <ustevent/base/timestamp.h>
#include <ustevent/base/timecycle.h>

#define XLOG_TRACE ULOG_TRACE
#define XLOG_DEBUG ULOG_DEBUG
#define XLOG_INFO ULOG_INFO
#define XLOG_WARN ULOG_WARN
#define XLOG_ERROR ULOG_ERROR
#define XLOG_FATAL ULOG_FATAL
#define XLOG_SYSERR ULOG_SYSERR
#define XLOG_SYSFATAL ULOG_SYSFATAL

#endif

struct XTimeStamp {
  base::Timestamp stamp;
  XTimeStamp() : stamp(base::Timestamp::now()) {}
  XTimeStamp(const XTimeStamp& s) : stamp(s.stamp) {}
  time_t Seconds() { return stamp.secondsSinceEpoch(); }
  int64_t MicroSeconds() { return stamp.microSecondsSinceEpoch(); }
  XTimeStamp& operator=(const XTimeStamp& s) {
    stamp = s.stamp;
    return *this;
  }
};

double XTimeStampDiff(XTimeStamp a, XTimeStamp b);
uint64_t XGetTimeCycle();
double XTimeCycleDiff(uint64_t a, uint64_t b);
uint64_t XTimeCycleSeconds(uint64_t cycles);
uint64_t XTimeCycleMicroSeconds(uint64_t cycles);

#endif  // FRAMEWORK_ADAPTER_
