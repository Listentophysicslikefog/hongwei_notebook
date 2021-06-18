// Copyright (c) 2019 UCloud All rights reserved.
#include "flow_ctrl.h"

bool TokenBucket::grant(uint64_t token_size) {
  uint64_t now_cycles = XGetTimeCycle();
  tokens_ = std::min(
      capacity_, tokens_ + static_cast<int64_t>(
                               XTimeCycleDiff(now_cycles, cycles_) * rate_));
  XLOG_DEBUG << "After Token=" << tokens_ << ", cap=" << capacity_
             << ", token_size=" << token_size
             << ". now=" << XTimeCycleMicroSeconds(now_cycles)
             << ", base_time=" << XTimeCycleMicroSeconds(cycles_);
  cycles_ = now_cycles;
  if (rate_ == kUnlimitedRate) {
    return true;
  }
  if (tokens_ < 0) {
    return false;
  }
  tokens_ -= token_size;
  return true;
}

void TokenBucket::add(uint64_t refound_size) {
  uint64_t now_cycles = XGetTimeCycle();
  tokens_ = std::min(
      capacity_,
      tokens_ + int64_t(refound_size) +
          static_cast<int64_t>(XTimeCycleDiff(now_cycles, cycles_) * rate_));
  XLOG_DEBUG << "After Token=" << tokens_ << ", cap=" << capacity_
             << ". now=" << XTimeCycleMicroSeconds(now_cycles)
             << ", base_time=" << XTimeCycleMicroSeconds(cycles_);
  cycles_ = now_cycles;
}

void TokenBucket::set_rate(int64_t rate) {
  if (rate < 0 || rate > kUnlimitedRate) {
    XLOG_WARN << "try to set invalid rate: " << rate;
    return;
  }
  uint64_t now_cycles = XGetTimeCycle();
  tokens_ = std::min(
      capacity_, tokens_ + static_cast<int64_t>(
                               XTimeCycleDiff(now_cycles, cycles_) * rate_));
  cycles_ = now_cycles;
  rate_ = rate;
}
