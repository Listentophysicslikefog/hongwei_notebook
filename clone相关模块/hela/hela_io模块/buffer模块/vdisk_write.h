// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_COMMON_VDISK_WRITE_H_
#define UDISK_COMMON_VDISK_WRITE_H_
#include <stdint.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include "safe_arena.h"

namespace udisk {
namespace common {

class StrBuffer {
 public:
  void Clear() {
    uint32_t max_head = m_str_.size() > 256u ? 256u : m_str_.size();  //256u 无符号类型的256
    memset(&m_str_[0], 0, max_head);
    m_str_.clear();
  }

  char* data() { return &m_str_[0]; }

  void resize(uint32_t size) { m_str_.resize(size); }

  uint32_t Size() { return m_str_.size(); }

  std::string m_str_;
};

static inline void SafePutPureStrBuffer(StrBuffer* buffer) {
  SafeSingleArena<StrBuffer>* arena =
      SafeArenaImpl<StrBuffer, void>::GetInstance();
  arena->Push(buffer);
}

}  // namespace common
}  // namespace udisk

#endif /* UDISK_COMMON_VDISK_WRITE_H_ */
