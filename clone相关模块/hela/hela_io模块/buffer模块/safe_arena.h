// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_COMMON_SAFE_ARENA_H_
#define UDISK_COMMON_SAFE_ARENA_H_

#include "mutex.h"

#include <cstdint>
#include <memory>
#include <queue>

namespace udisk {
namespace common {

template <class T>
class SafeSingleArena {
 public:
  T *Fetch() {
    base::MutexLockGuard lock(mutex_);

    if (queue_.empty()) {
      ++allocate_count_;
      return new T();
    }
    T *front = queue_.front();
    queue_.pop();
    return front;
  }

  bool Push(T *obj) {
    base::MutexLockGuard lock(mutex_);

    if (queue_.size() >= max_size_) {
      delete obj;
      --allocate_count_;
      return false;
    }
    obj->Clear();
    queue_.push(obj);
    return true;
  }

  uint32_t Size() {
    base::MutexLockGuard lock(mutex_);

    return queue_.size();
  }

  uint32_t max_size() const {
    base::MutexLockGuard lock(mutex_);

    return max_size_;
  }

  uint32_t allocate_count() const {
    base::MutexLockGuard lock(mutex_);

    return allocate_count_;
  }

 protected:
  SafeSingleArena(uint32_t size) : max_size_(size), allocate_count_(0) {}
  SafeSingleArena(const SafeSingleArena &);
  SafeSingleArena &operator=(const SafeSingleArena &);
  virtual ~SafeSingleArena() {}

  std::queue<T *> queue_;
  uint32_t max_size_;
  uint32_t allocate_count_;

  base::MutexLock mutex_;
};

template <class T, class T2>
class SafeArenaImpl : public SafeSingleArena<T> {
 public:
  static SafeArenaImpl<T, T2> *GetInstance() { return m_instance_; }

  static void Init(uint32_t size) {
    if (m_instance_)
      return;
    else
      m_instance_ = new SafeArenaImpl<T, T2>(size);
  }

 protected:
  SafeArenaImpl(uint32_t size) : SafeSingleArena<T>(size) {}

  static SafeArenaImpl<T, T2> *m_instance_;
};

template <class T, class T2>
SafeArenaImpl<T, T2> *SafeArenaImpl<T, T2>::m_instance_ = NULL;

}  // end of ns common
}  // end of ns udisk

#endif /* UDISK_COMMON_SAFE_ARENA_H_ */
