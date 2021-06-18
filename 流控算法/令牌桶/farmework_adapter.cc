// Copyright (c) 2020 UCloud All rights reserved.
#include "framework_adapter.h"

double XTimeStampDiff(XTimeStamp a, XTimeStamp b) {
  return base::timeDifference(a.stamp, b.stamp);
}

#ifdef ENABLE_UEVENT
// 通过timestamp模拟timecycle实现，效率稍微低一些

uint64_t XGetTimeCycle() {
  return base::Timestamp::now().microSecondsSinceEpoch();
}

double XTimeCycleDiff(uint64_t a, uint64_t b) {
  int64_t diff = a - b;
  return static_cast<double>(diff) / 1000000;
}

uint64_t XTimeCycleSeconds(uint64_t cycles) { return cycles / 1000000; }

uint64_t XTimeCycleMicroSeconds(uint64_t cycles) { return cycles; }

#else

uint64_t XGetTimeCycle() { return base::TimeCycle::GetTimeCycle(); }

double XTimeCycleDiff(uint64_t a, uint64_t b) {
  int64_t diff = a - b;
  return static_cast<double>(diff) / base::TimeCycle::tsc_rate();
}

uint64_t XTimeCycleSeconds(uint64_t cycles) {
  return cycles / base::TimeCycle::tsc_rate();
}

uint64_t XTimeCycleMicroSeconds(uint64_t cycles) {
  return cycles / (base::TimeCycle::tsc_rate() / 1000000);
}

#endif
