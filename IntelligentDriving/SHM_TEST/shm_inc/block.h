/**
 * @file : block.h
 */
#ifndef BLOCK_H_
#define BLOCK_H_
#include <unistd.h>
#include <cstdint>
#include "lockless.h"
#include "macros.h"
namespace shm::framework::memory
{

    class Block
    {
        friend class AllocMemoryBase;

    public:
        /**
         * @brief Construct a new Block object
         */
        Block();

        /**
         * @brief Destory the Block object
         */
        virtual ~Block();

        /**
         * @brief get data size
         * @return uint64_t
         */
        uint64_t MsgSize() const { return m_msgSize; }

        /**
         * @brief Set The Msg Size object
         * @param msgSize data size
         */
        void SetMsgSize(const uint64_t &msgSize) { m_msgSize = msgSize; }

        /**
         * @brief get message info size
         * @return uint64_t message info size
         */
        uint64_t MsgInfoSize() const { return m_msgInfoSize; }

        /**
         * @brief Set the msg info size object
         * @param msgInfoSize message info size
         */
        void SetMsgInfoSize(uint64_t msgInfoSize)
        {
            m_msgInfoSize = msgInfoSize;
        }

        static const int32_t RW_LOCK_FREE;       // no lock status
        static const int32_t WRITE_EXCLUSIVE;    // only write lock
        static const int32_t MAX_TRY_LOCK_TIMES; // max try times
    private:
        /**
         * @brief try write lock
         * @return true success
         * @return false failed
         */
        bool TryLockForWrite();

        /**
         * @brief try read lock
         * @return true success
         * @return false failed
         */
        bool TryLockForRead();

        /**
         * @brief release write lock
         */
        void ReleaseWriteLock();
        /**
         * @brief release read lock
         */
        void ReleaseReadLock();

        /**
         * @brief ckeep && heck pid_set alive
         * @param NA
         * @return true: intra-process communication
         *         false: all process dead
         */
        bool AnyAlive();

        /**
         * @brief insert pid
         * @param pid
         * @return true:
         * @return false: probed Elem diff with all set
         */
        bool Insert(uint32_t pid) { return m_pidSet.Insert(pid); }

        const static uint8_t MAX_PROCESS_NUM = 64;
        common::LocklessSet<MAX_PROCESS_NUM> m_pidSet;
        volatile std::atomic<int32_t> m_lockNum = {0};
        uint64_t m_msgSize;
        uint64_t m_msgInfoSize;
    };

} // namespace shm::framework memory
#endif