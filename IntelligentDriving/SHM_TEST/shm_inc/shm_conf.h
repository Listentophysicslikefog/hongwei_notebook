/**
 * @file shm_conf.h
 */
#ifndef SHM_CONF_H_
#define SHM_CONF_H_

#include <cstdint>
#include <string>
#include "shm_config.h"

namespace shm::framework::memory
{

    class ShmConf
    {
    public:
        /**
         * @brief Construct a new Shm conf object
         */
        ShmConf();
        /**
         * @brief Construct a new shm Conf object with qos property
         * @param prop qos config property
         */
        explicit ShmConf(const ShmQosProp &prop);
        /**
         * @brief Destory the Shm Conf object
         */
        virtual ~ShmConf();
        /**
         * @brief update shm config with qos config property
         * @param prop qos config property
         */
        void Update(const ShmQosProp &prop);

        /**
         * @brief update shm config with block size and block count
         * @param blockSize blockSize
         * @param blockCount blockCount
         */
        void Update(const uint64_t &blockSize, const uint32_t blockCount);

        /**
         * @brief get block size(data size after align)
         * @return uint64_t&
         */
        const uint64_t CeilingMsgSize() { return m_ceilingMsgSize; }

        /**
         * @brief get block buffer size(block buffer size = block size + message)
         * @return const uint64_t&
         */
        const uint64_t BlockBufSize() { return m_blockBufSize; }

        /**
         * @brief get block num
         * @return const uint32_t&
         */
        const uint64_t BlockNum() { return m_blockNum; }

        /**
         * @brief msgSize
         * @return const uint32_t
         */
        const uint32_t MsgSize() { return m_msgSize; }
        /**
         * @brief get segment size
         * @return const uint64_t
         */
        const uint64_t &ManagedShmSize() { return m_managedShmSize; }

    private:
        /**
         * @brief calculate block size depends on data size(block size = data size aligend)
         * @param realMsgSize data size
         * @return uint64_t
         */
        uint64_t GetCeilingMessageSize(const uint64_t &realMsgSize);
        /**
         * @brief calculate block buf size depends on data size(block buf size = block size + message info size)
         * @param ceilingMsgSize block size
         * @return uint64_t
         */
        uint64_t GetBlockBufSize(const uint64_t &ceilingMsgSize);

        /**
         * @brief selected block num depends on block size
         * @param ceilingMsgSize block size
         * @return uint32_t
         */
        uint32_t GetBlockNum(const uint64_t &ceilingMsgSize);

        uint64_t m_ceilingMsgSize; // block size
        uint64_t m_msgSize;        // block size
        uint64_t m_blockBufSize;   // block buf size
        uint32_t m_blockNum;       // block num in shm memory
        uint64_t m_managedShmSize; // shm memory size

        // Extra size, Byte
        static const uint64_t EXTRA_SIZE;
        // State size, Byte
        static const uint64_t STATE_SIZE;
        // Block size, Byte
        static const uint64_t BLOCK_SIZE;
        // Message info size, Byte
        static const uint64_t MESSAGE_INFO_SIZE;
        // default messages 32M
        static const uint32_t DEFAULT_BLOCK_SIZE;
        static const uint32_t DEFAULT_BLOCK_NUM;
        // For message 0-10K
        static const uint32_t BLOCK_NUM_16K;
        static const uint64_t MESSAGE_SIZE_16K;
        // For message 10K-100K
        static const uint32_t BLOCK_NUM_128K;
        static const uint64_t MESSAGE_SIZE_128K;
        // For message 100K-1M
        static const uint32_t BLOCK_NUM_1M;
        static const uint64_t MESSAGE_SIZE_1M;
        // For message 1M-6M
        static const uint32_t BLOCK_NUM_8M;
        static const uint64_t MESSAGE_SIZE_8M;
        // For message 6M-10M
        static const uint32_t BLOCK_NUM_16M;
        static const uint64_t MESSAGE_SIZE_16M;
        // For message 10M+
        static const uint32_t BLOCK_NUM_MORE;
        static const uint64_t MESSAGE_SIZE_MORE;
    };

} // namespace shm::framework::memory

#endif // SHM_CONF_H_