/**
 * @file alloc_memory_base.cpp
 */
#include "alloc_memory_base.h"
#include <iostream>
#include "shm_conf.h"
#include "shm_log.h"
#include "util.h"
namespace shm::framework::memory
{
    AllocMemoryBase::AllocMemoryBase(const std::string &topicName)
        : m_init(false), m_conf(), m_topicName(topicName), m_state(nullptr),
          m_blocks(nullptr), m_managedShm(nullptr), m_blockBufLock(),
          m_blockBufAddrs() {}

    AllocMemoryBase::AllocMemoryBase(const std::string &topicName, const ShmQosProp &prop)
        : m_init(false), m_conf(prop), m_topicName(topicName), m_state(nullptr),
          m_blocks(nullptr), m_managedShm(nullptr), m_blockBufLock(),
          m_blockBufAddrs() {}

    bool AllocMemoryBase::AcquireBlockToWrite(const std::size_t msgSize, WritableBlock *const writableBlock)
    {
        printf("%s \n", __func__);
        RETURN_VAL_IF_NULL(writableBlock, false);
        if (!m_init)
        {
            printf("share memory has not be created.");
            return false;
        }
        /*
        bool result = true;
        if(m_state->need_remap()){
            resut = Remap();
        }
        */
        if (msgSize > m_conf.CeilingMsgSize())
        {
            printf("msgSize: %ld , larger than current shm_buffer size: %ld , in share memory yaml config file,AcquireBlockToWrite failed.",
                   msgSize, m_conf.CeilingMsgSize());
            return false;
            // result = Recreate(msgSize);
        }
        /*
          if(!result){
            printf("segment update failed.");
            return false;
          }
        */
        uint32_t index = GetNextWritableBlockIndex();
        writableBlock->index = index;
        writableBlock->block = &m_blocks[index];
        writableBlock->buf = m_blockBufAddrs[index];
        // 加锁成功，插入当前进程id
        return m_blocks[index].Insert(getpid());
    }

    bool AllocMemoryBase::ReleaseWrittenBlock(const WritableBlock &writableBlock)
    {
        printf("%s \n", __func__);
        auto index = writableBlock.index;
        if (index >= m_conf.BlockNum())
        {
            return false;
        }
        m_blocks[index].ReleaseWriteLock();
        return true;
    }

    bool AllocMemoryBase::AcquireBlockToRead(ReadableBlock *const readableBlock)
    {
        printf("%s \n", __func__);
        RETURN_VAL_IF_NULL(readableBlock, false);
        if (!m_init && !OpenOnly())
        {
            printf("failed to open shared memory, can't read now.");
            return false;
        }

        auto index = readableBlock->index;
        if (index >= m_conf.BlockNum())
        {
            printf("invalid block index: %d", index);
            return false;
        }

        bool result = true;
        if (m_state->NeedRemap())
        {
            result = Remap();
        }
        if (!result)
        {
            printf("segment update failed.");
            return false;
        }

        if (!m_blocks[index].TryLockForRead())
        {
            // 对当前块加锁失败，判断当前块上进程是否都存活，存活则返回失败
            if (m_blocks[index].AnyAlive())
            {
                return false;
                // 释放死掉的进程加的锁后再次尝试加锁
            } // 这里是不是有问题 if 和else if是不是只可以走一个，释放死掉的进程加的锁后 走不到 再次尝试加锁 ？？ todo
            else if (!m_blocks[index].TryLockForRead())
            {
                return false;
            }
        }
        readableBlock->block = m_blocks + index;
        readableBlock->buf = m_blockBufAddrs[index];
        // 加锁成功，插入当前进程id
        return m_blocks[index].Insert(getpid());
    }

    bool AllocMemoryBase::ReleaseReadBlock(const ReadableBlock &readableBlock)
    {
        printf("%s \n", __func__);
        auto index = readableBlock.index;
        if (index >= m_conf.BlockNum())
        {
            return false;
        }
        m_blocks[index].ReleaseReadLock();
        return true;
    }

    bool AllocMemoryBase::Destroy()
    {
        if (!m_init)
        {
            return true;
        }
        m_init = false;
        try
        {
            m_state->DecreaseReferenceCounts();
            uint32_t reference_counts = m_state->ReferenceCount();
            if (!reference_counts)
            {
                return Remove();
            }
        }
        catch (...)
        {
            printf("execption.");
            return false;
        }
        printf("destory success.");
        return true;
    }

    bool AllocMemoryBase::Remap()
    {
        m_init = false;
        printf("before reset.");
        Reset();
        printf("after reset.");
        return OpenOnly();
    }

    bool AllocMemoryBase::Recreate(const uint64_t &msgSize, const uint32_t blockNum)
    {
        m_init = false;
        Reset();
        Remove();
        m_conf.Update(msgSize, blockNum);
        if (!Create())
        {
            return false;
        }
        m_state->SetNeedRemap(true);
        return true;
    }

    uint32_t AllocMemoryBase::GetNextWritableBlockIndex()
    {
        const auto block_num = m_conf.BlockNum();
        while (true)
        {
            // index +1
            uint32_t try_idx = m_state->FetchAddSeq(1) % block_num;
            if (m_blocks[try_idx].TryLockForWrite())
            {
                return try_idx;
            }
            else
            {
                // 对当前块加锁失败，判断当前块上的进程是否都存活
                if (m_blocks[try_idx].AnyAlive())
                {
                    // 进程存活，进行下次循环尝试下一块
                    continue;
                }
                else
                {
                    // 释放死掉的进程加的锁，再次尝试加锁
                    if (m_blocks[try_idx].TryLockForWrite())
                    {
                        return try_idx;
                    }
                }
            }
        }
        return 0;
    }
} // namespace shm::framework::memory