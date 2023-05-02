/**
 * @file alloc_memory.cpp
 */
#include "alloc_memory_base.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "alloc_memory.h"
#include "block.h"
#include "shm_conf.h"
#include "shm_log.h"
namespace shm::framework::memory
{

    AllocMemory::AllocMemory(const std::string &topicName)
        : AllocMemoryBase(topicName)
    {
        m_shmName = SHM_KEY_PREFIX + std::to_string(Hash(topicName));
        printf("%s topic %s m_shmName = %s \n", __func__, topicName.c_str(), m_shmName.c_str());
    }

    AllocMemory::AllocMemory(const std::string &topicName, const ShmQosProp &prop)
        : AllocMemoryBase(topicName, prop)
    {
        m_shmName = SHM_KEY_PREFIX + std::to_string(Hash(topicName));
        printf("%s topic %s m_shmName = %s \n", __func__, topicName.c_str(), m_shmName.c_str());
    }

    AllocMemory::~AllocMemory() { Destroy(); }

    bool AllocMemory::Create()
    {
        if (m_init)
        {
            return true;
        }
        // create m_managedShm
        int fd = shm_open(m_shmName.c_str(), O_RDWR | O_CREAT | O_EXCL, DEFAULT_PERMISSION);
        if (0 > fd)
        {
            if (EEXIST == errno)
            {
                printf("shm already exist, recreate. msg size :%lu  blockNum: %u", m_conf.MsgSize(), m_conf.BlockNum());
                return Recreate(m_conf.MsgSize(), m_conf.BlockNum());
            }
            else
            {
                printf("create shm failed,error :%s.", strerror(errno));
                return false;
            }
        }

        if (0 > ftruncate(fd, m_conf.ManagedShmSize()))
        {
            printf("ftruncate failed: %s.", strerror(errno));
            close(fd);
            return false;
        }
        // attach m_managedShm
        m_managedShm = mmap(nullptr, m_conf.ManagedShmSize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // offset 0
        if (MAP_FAILED == m_managedShm)
        {
            printf("attach shm failed: %s.", strerror(errno));
            close(fd);
            shm_unlink(m_shmName.c_str());
            return false;
        }
        close(fd);

        // create field m_state
        m_state = new (m_managedShm) State(m_conf.MsgSize(), m_conf.BlockNum());
        if (!m_state)
        {
            printf("create state failed.");
            munmap(m_managedShm, m_conf.ManagedShmSize());
            m_managedShm = nullptr;
            shm_unlink(m_shmName.c_str());
            return false;
        }
        m_conf.Update(m_state->CeilingMsgSize(), m_state->BlockNum());
        // create field m_blocks
        m_blocks = new (static_cast<char *>(m_managedShm) + sizeof(State)) Block[m_conf.BlockNum()];
        if (!m_blocks)
        {
            printf("create blocks failed.");
            m_state->~State();
            m_state = nullptr;
            munmap(m_managedShm, m_conf.ManagedShmSize());
            m_managedShm = nullptr;
            shm_unlink(m_shmName.c_str());
            return false;
        }
        // create block buf
        uint32_t i = 0;
        for (; i < m_conf.BlockNum(); ++i)
        {
            uint8_t *addr = new (static_cast<char *>(m_managedShm) + sizeof(State) +
                                 m_conf.BlockNum() * sizeof(Block) + i * m_conf.BlockBufSize())
                uint8_t[m_conf.BlockBufSize()];
            if (!addr)
            {
                break;
            }
            std::lock_guard<std::mutex> lg(m_blockBufLock);
            m_blockBufAddrs[i] = addr;
        }
        if (i != m_conf.BlockNum())
        {
            m_state->~State();
            m_state = nullptr;
            m_blocks = nullptr;
            {
                std::lock_guard<std::mutex> lg(m_blockBufLock);
                m_blockBufAddrs.clear();
            }
            munmap(m_managedShm, m_conf.ManagedShmSize());
            m_managedShm = nullptr;
            shm_unlink(m_shmName.c_str());
            return false;
        }
        m_state->IncreaseReferenceCounts();
        m_init = true;
        return true;
    }

    bool AllocMemory::OpenOnly()
    {
        if (m_init)
        {
            return true;
        }
        // get m_managedShm
        int fd = shm_open(m_shmName.c_str(), O_RDWR, 0644);
        if (0 > fd)
        {
            printf("m_shmName get shm failed :%s", strerror(errno));
            return false;
        }
        struct stat file_attr;
        if (0 > fstat(fd, &file_attr))
        {
            printf("fstat failed : %s", strerror(errno));
            close(fd);
            return false;
        }
        // attach m_managedShm
        m_managedShm = mmap(nullptr, file_attr.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (MAP_FAILED == m_managedShm)
        {
            printf("attach shm failed: %s", strerror(errno));
            close(fd);
            return false;
        }
        close(fd);
        // get field m_state
        m_state = reinterpret_cast<State *>(m_managedShm);
        if (!m_state)
        {
            printf("get state failed: %s.", strerror(errno));
            munmap(m_managedShm, file_attr.st_size);
            m_managedShm = nullptr;
            return false;
        }
        m_conf.Update(m_state->CeilingMsgSize(), m_state->BlockNum());

        // get field m_blocks
        m_blocks = reinterpret_cast<Block *>(static_cast<char *>(m_managedShm) + sizeof(State));
        if (!m_blocks)
        {
            printf("get blocks failed: %s.", strerror(errno));
            m_state = nullptr;
            munmap(m_managedShm, m_conf.ManagedShmSize());
            m_managedShm = nullptr;
            return false;
        }
        // get block buf
        uint32_t i = 0;
        for (; i < m_conf.BlockNum(); ++i)
        {
            uint8_t *addr = reinterpret_cast<uint8_t *>(static_cast<char *>(m_managedShm) + sizeof(State) +
                                                        m_conf.BlockNum() * sizeof(Block) + i * m_conf.BlockBufSize());
            std::lock_guard<std::mutex> lg(m_blockBufLock);
            m_blockBufAddrs[i] = addr;
        }

        if (i != m_conf.BlockNum())
        {
            printf("open only failed.");
            m_state->~State();
            m_state = nullptr;
            m_blocks = nullptr;
            {
                std::lock_guard<std::mutex> lg(m_blockBufLock);
                m_blockBufAddrs.clear();
            }
            munmap(m_managedShm, m_conf.ManagedShmSize());
            m_managedShm = nullptr;
            shm_unlink(m_shmName.c_str());
            return false;
        }
        m_state->IncreaseReferenceCounts();
        m_init = true;
        return true;
    }

    bool AllocMemory::Remove()
    {
        if (shm_unlink(m_shmName.c_str()) < 0)
        {
            printf("shm_unlink failed: %s.", strerror(errno));
            return false;
        }
        return true;
    }

    void AllocMemory::Reset()
    {
        m_state = nullptr;
        m_blocks = nullptr;
        {
            std::lock_guard<std::mutex> lg(m_blockBufLock);
            m_blockBufAddrs.clear();
        }
        if (m_managedShm)
        {
            munmap(m_managedShm, m_conf.ManagedShmSize());
            m_managedShm = nullptr;
        }
        return;
    }

} // namespace shm::framework::memory