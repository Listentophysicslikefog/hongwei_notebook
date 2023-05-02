/**
 * @file rw_lock_guard.h
 */
#ifndef RW_LOCK_GUARD_H_
#define RW_LOCK_GUARD_H_
#include <unistd.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
namespace shm::framework::memory
{

    /**
     * @brief read lock
     * @param RWLock
     */
    template <typename RWLock>
    class ReadLockGuard
    {
    public:
        explicit ReadLockGuard(RWLock &lock) : m_rwLock(lock)
        {
            m_rwLock.ReadLock();
        }
        ~ReadLockGuard() { m_rwLock.ReadUnlock(); }

    private:
        ReadLockGuard(const ReadLockGuard &other) = delete;
        ReadLockGuard &operator=(const ReadLockGuard &other) = delete;
        RWLock &m_rwLock;
    };

    /**
     * @brief write lock
     * @param RWLock
     */
    template <typename RWLock>
    class WriteLockGuard
    {
    public:
        explicit WriteLockGuard(RWLock &lock) : m_rwLock(lock)
        {
            m_rwLock.WriteLock();
        }
        ~WriteLockGuard() { m_rwLock.WriteUnlock(); }

    private:
        WriteLockGuard(const WriteLockGuard &other) = delete;
        WriteLockGuard &operator=(const WriteLockGuard &other) = delete;
        RWLock &m_rwLock;
    };

} // namespace shm::framework::memory

#endif