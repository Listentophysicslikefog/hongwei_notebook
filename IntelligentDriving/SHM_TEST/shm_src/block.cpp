/**
 * @file : block.cpp
 */

#include "block.h"
#include <iostream>
#include "shm_log.h"
namespace shm::framework::memory
{
    const int32_t Block::RW_LOCK_FREE = 0;       // no lock status
    const int32_t Block::WRITE_EXCLUSIVE = -1;   // only write lock
    const int32_t Block::MAX_TRY_LOCK_TIMES = 5; // max try times

    Block::Block() : m_msgSize(0), m_msgInfoSize(0) {}

    Block::~Block() {}

    bool Block::TryLockForWrite()
    {
        printf("%s lock num: %d", __func__, m_lockNum.load());
        int32_t rwLockFree = RW_LOCK_FREE;
        if (!m_lockNum.compare_exchange_weak(rwLockFree, WRITE_EXCLUSIVE, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            return false;
        }
        return true;
    }

    bool Block::TryLockForRead()
    {
        printf("%s lock num: %d", __func__, m_lockNum.load());
        int32_t lockNum = m_lockNum.load();
        if (RW_LOCK_FREE > lockNum)
        {
            printf("block is being written.");
            return false;
        }
        int32_t try_times = 0;
        // increase read lock
        while (!m_lockNum.compare_exchange_weak(lockNum, lockNum + 1, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            ++try_times;
            if (MAX_TRY_LOCK_TIMES == try_times)
            {
                printf("fail to add read lock num, curr num :%d", lockNum);
                return false;
            }
            lockNum = m_lockNum.load();
            if (RW_LOCK_FREE > lockNum)
            {
                printf("block is being written.");
                return false;
            }
        }
        return true;
    }

    void Block::ReleaseWriteLock()
    {
        // release a write lock, decrease reference
        if (m_lockNum < 0)
        {
            m_lockNum.fetch_add(1);
        }
        return;
    }

    void Block::ReleaseReadLock()
    {
        // release a read lock, decrease reference
        if (m_lockNum > 0)
        {
            m_lockNum.fetch_sub(1);
        }
        return;
    }

    bool Block::AnyAlive()
    {
        std::atomic_bool alive{false};
        uint32_t current_pid = getpid();
        for (auto &i : m_pidSet.m_locklessSet)
        {
            uint32_t pid = i.load();
            if (!pid)
            {
                continue;
            }
            if (pid == current_pid)
            {
                // intra-process communication
                // two threads of the same process
                alive = true;
            }
            if (ProcDead(pid))
            {
                while (!m_pidSet.Remove(pid))
                {
                }
                if (m_lockNum > 0)
                {
                    ReleaseReadLock();
                }
                else if (m_lockNum < 0)
                {
                    ReleaseWriteLock();
                }
            }
            else
            {
                alive = true;
            }
        }
        return alive;
    }

} // namespace shm::framework::memory