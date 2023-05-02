/**
 * @file atomic_rw_lock.h
 */
#ifndef ATOMIC_RW_LOCK_H_
#define ATOMIC_RW_LOCK_H_
#include <unistd.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "rw_lock_guard.h"
namespace shm::framework::memory
{

    class AtomicRWLock
    {
        friend class ReadLockGuard<AtomicRWLock>;
        friend class WriteLockGuard<AtomicRWLock>;

    public:
        // not locked
        static const int32_t RW_LOCK_FREE = 0;
        // write locked
        static const int32_t WRITE_EXCLUSIVE = -1;
        // max times to try read  lock
        static const uint32_t MAX_RETRY_TIMES = 5;
        /**
         * @brief construct a new Atomic R W Lock object
         */
        AtomicRWLock();

        /**
         * @brief Construct a new Atomic R W Lock object with flag
         * @param writeFirst flag that should be writed first of not
         */
        explicit AtomicRWLock(const bool writeFirst) : m_writeFirst(writeFirst) {}

    private:
        // all these function only can used by  ReadLockGuard/WriteLockGuard
        /**
         * @brief Read Lock
         */
        void ReadLock();
        /**
         * @brief Write Lock
         */
        void WriteLock();
        /**
         * @brief Read Unlock
         */
        void ReadUnlock();
        /**
         * @brief Write Unlock
         */
        void WriteUnlock();

        AtomicRWLock(const AtomicRWLock &other) = delete;
        AtomicRWLock &operator=(const AtomicRWLock &other) = delete;
        // if there is write lock waited
        std::atomic<uint32_t> m_writeLockWaiteNum = {0};
        // current lock state
        std::atomic<int32_t> m_lockNum = {0};
        //  flag that should be writed first of not
        bool m_writeFirst = true;
    };

    inline void AtomicRWLock::ReadLock()
    {
        uint32_t retryTimes = 0;
        int32_t lock_num = m_lockNum.load();
        if (m_writeFirst)
        {
            do
            {
                // if current has write lock or write lock waited, don't allow new
                // read lock
                while (lock_num < RW_LOCK_FREE || m_writeLockWaiteNum.load() > 0)
                {
                    if (++retryTimes == MAX_RETRY_TIMES)
                    {
                        // release self time slice for saving cpu
                        std::this_thread::yield();
                        retryTimes = 0;
                    }
                    lock_num = m_lockNum.load();
                }
                // read lock +1
            } while (!m_lockNum.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel, std::memory_order_relaxed));
        }
        else
        {
            do
            {
                // if current has write lock  don't allow new
                // read lock
                while (lock_num < RW_LOCK_FREE)
                {
                    if (++retryTimes == MAX_RETRY_TIMES)
                    {
                        // saving cpu
                        std::this_thread::yield();
                        retryTimes = 0;
                    }
                    lock_num = m_lockNum.load();
                }
                // read lock +1
            } while (!m_lockNum.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel, std::memory_order_relaxed));
        }
        return;
    }

    inline void AtomicRWLock::WriteLock()
    {
        int32_t rwLockFree = RW_LOCK_FREE;
        uint32_t retryTimes = 0;
        // set flag that there is write lock waited
        m_writeLockWaiteNum.fetch_add(1);
        // retry to write lock
        while (!m_lockNum.compare_exchange_weak(rwLockFree, WRITE_EXCLUSIVE, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            // rwLockFree will change after CAS fail, so init agin
            rwLockFree = RW_LOCK_FREE;
            if (++retryTimes == MAX_RETRY_TIMES)
            {
                // release self time slice for saving cpu
                std::this_thread::yield();
                retryTimes = 0;
            }
        }
        // write lock success, no write lock waited
        m_writeLockWaiteNum.fetch_sub(1);
        return;
    }

    inline void AtomicRWLock::ReadUnlock()
    {
        // read unlock
        m_lockNum.fetch_sub(1);
        return;
    }

    inline void AtomicRWLock::WriteUnlock()
    {
        // write unlock
        m_lockNum.fetch_add(1);
        return;
    }

} // namespace shm::framework::memory

#endif