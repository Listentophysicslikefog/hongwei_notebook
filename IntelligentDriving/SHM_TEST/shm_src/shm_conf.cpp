/**
 * @file shm_conf.cpp
 */
#include "shm_conf.h"
#include <iostream>

namespace shm::framework::memory
{

    ShmConf::ShmConf()
    {
        m_msgSize = 0;
        m_ceilingMsgSize = DEFAULT_BLOCK_SIZE;
        m_blockBufSize = m_ceilingMsgSize + MESSAGE_INFO_SIZE;
        m_blockNum = DEFAULT_BLOCK_NUM;
        m_managedShmSize = EXTRA_SIZE + STATE_SIZE + (BLOCK_SIZE + m_blockBufSize) * m_blockNum;
    }

    ShmConf::ShmConf(const ShmQosProp &prop) { Update(prop); }

    ShmConf::~ShmConf() {}

    void ShmConf::Update(const ShmQosProp &prop)
    {
        m_msgSize = prop.blockSize;
        m_ceilingMsgSize = GetCeilingMessageSize(prop.blockSize);
        m_blockBufSize = GetBlockBufSize(m_ceilingMsgSize);
        m_blockNum = prop.blockCount;
        m_managedShmSize =
            EXTRA_SIZE + STATE_SIZE + (BLOCK_SIZE + m_blockBufSize) * m_blockNum;
        return;
    }

    void ShmConf::Update(const uint64_t &blockSize, const uint32_t blockCount)
    {
        m_ceilingMsgSize = GetCeilingMessageSize(blockSize);
        m_blockBufSize = GetBlockBufSize(m_ceilingMsgSize);
        m_blockNum = blockCount;
        m_managedShmSize =
            EXTRA_SIZE + STATE_SIZE + (BLOCK_SIZE + m_blockBufSize) * m_blockNum;
        return;
    }

    const uint64_t ShmConf::EXTRA_SIZE = 1024 * 4;
    const uint64_t ShmConf::STATE_SIZE = 1024;
    const uint64_t ShmConf::BLOCK_SIZE = 1024;
    const uint64_t ShmConf::MESSAGE_INFO_SIZE = 1024;

    const uint32_t ShmConf::DEFAULT_BLOCK_NUM = 32;
    const uint32_t ShmConf::DEFAULT_BLOCK_SIZE = 1024 * 1024 * 8;

    const uint32_t ShmConf::BLOCK_NUM_16K = 512;
    const uint64_t ShmConf::MESSAGE_SIZE_16K = 1024 * 16;

    const uint32_t ShmConf::BLOCK_NUM_128K = 128;
    const uint64_t ShmConf::MESSAGE_SIZE_128K = 1024 * 128;

    const uint32_t ShmConf::BLOCK_NUM_1M = 64;
    const uint64_t ShmConf::MESSAGE_SIZE_1M = 1024 * 1024;

    const uint32_t ShmConf::BLOCK_NUM_8M = 32;
    const uint64_t ShmConf::MESSAGE_SIZE_8M = 1024 * 1024 * 8;

    const uint32_t ShmConf::BLOCK_NUM_16M = 16;
    const uint64_t ShmConf::MESSAGE_SIZE_16M = 1024 * 1024 * 16;

    const uint32_t ShmConf::BLOCK_NUM_MORE = 8;
    const uint64_t ShmConf::MESSAGE_SIZE_MORE = 1024 * 1024 * 32;

    uint64_t ShmConf::GetCeilingMessageSize(const uint64_t &realMsgSize)
    {
        uint64_t ceilingMsgSize = MESSAGE_SIZE_16K;
        if (realMsgSize <= MESSAGE_SIZE_16K)
        {
            ceilingMsgSize = MESSAGE_SIZE_16K;
        }
        else if (realMsgSize <= MESSAGE_SIZE_128K)
        {
            ceilingMsgSize = MESSAGE_SIZE_128K;
        }
        else if (realMsgSize <= MESSAGE_SIZE_1M)
        {
            ceilingMsgSize = MESSAGE_SIZE_1M;
        }
        else if (realMsgSize <= MESSAGE_SIZE_8M)
        {
            ceilingMsgSize = MESSAGE_SIZE_8M;
        }
        else if (realMsgSize <= MESSAGE_SIZE_16M)
        {
            ceilingMsgSize = MESSAGE_SIZE_16M;
        }
        else
        {
            ceilingMsgSize = MESSAGE_SIZE_MORE;
        }
        return ceilingMsgSize;
    }

    uint64_t ShmConf::GetBlockBufSize(const uint64_t &ceilingMsgSize)
    {
        return ceilingMsgSize + MESSAGE_INFO_SIZE;
    }

    uint32_t ShmConf::GetBlockNum(const uint64_t &ceilingMsgSize)
    {
        uint32_t num = 0;
        switch (ceilingMsgSize)
        {
        case MESSAGE_SIZE_16K:
            num = BLOCK_NUM_16K;
            break;
        case MESSAGE_SIZE_128K:
            num = BLOCK_NUM_128K;
            break;
        case MESSAGE_SIZE_1M:
            num = BLOCK_NUM_1M;
            break;
        case MESSAGE_SIZE_8M:
            num = BLOCK_NUM_8M;
            break;
        case MESSAGE_SIZE_16M:
            num = BLOCK_NUM_16M;
            break;
        case MESSAGE_SIZE_MORE:
            num = BLOCK_NUM_MORE;
            break;
        default:
            printf("unknown ceiling_msg_size[%ld]", ceilingMsgSize);
            break;
        }
        return num;
    }

} // namespace shm::framework::memory
