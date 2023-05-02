/**
 * @file shm_manager.cpp
 */
#include "shm_manager.h"
#include "shm_log.h"
namespace shm::framework::memory
{
    static void SignalHandler(int signo) { exit(0); }

    ShmManager::ShmManager()
    {
        struct sigaction handler;
        handler.sa_handler = &SignalHandler;
        sigaddset(&handler.sa_mask, SIGQUIT);
        handler.sa_flags = 0;
        sigaction(SIGINT, &handler, NULL);
    }

    ShmManager::~ShmManager() {}

    ShmManager &ShmManager::GetInstance()
    {
        static ShmManager instance;
        return instance;
    }

    bool ShmManager::CreateSegment(const std::string &config)
    {
        printf("%s", __func__);
        if (!ShmConfig::GetInstance().LoadConfig(config))
        {
            printf("%s config file %s load failed", __func__, config.c_str());
            return false;
        }
        return CreateSegmentFromConfig();
    }

    void ShmManager::Destory() { return; } // question or not

    bool ShmManager::CreateSegmentFromConfig()
    {
        for (const auto &iter : ShmConfig::GetInstance().GetShmQosPropMap())
        {
            if (m_segMap.find(iter.first) != m_segMap.end())
            {
                printf("%s segment %s already exit.", __func__, iter.first.c_str());
                continue;
            }
            AllocMemoryBasePtr segment = std::make_shared<AllocMemory>(iter.first, iter.second);
            if (!segment->Create())
            {
                printf("%s segment %s create  failed.", __func__, iter.first.c_str());
                continue;
            }
            printf("%s segment %s create  success.", __func__, iter.first.c_str());
            m_segMap.insert(std::pair<std::string, AllocMemoryBasePtr>(iter.first, segment));
        }
        return (m_segMap.size() > 0);
    }

    bool ShmManager::RequestMemory(const std::string &topicName, MemBlock &memblock)
    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        if (m_segMap.find(topicName) == m_segMap.end())
        {
            printf("%s failed, topic:%s  not config in share memory config file\n", __func__, topicName.c_str());
            return false;
        }
        WritableBlock wb;
        if (!m_segMap[topicName]->AcquireBlockToWrite(memblock.size, &wb))
        {
            printf("%s AcquireBlockToWrite failed, topic:%s  \n", __func__, topicName.c_str());
            return false;
        }
        memblock.index = wb.index;
        memblock.buf = wb.buf;
        return true;
    }

    bool ShmManager::WriteFinish(const std::string &topicName, const MemBlock &memblock)
    {
        if (m_segMap.find(topicName) == m_segMap.end())
        {
            printf("%s failed, topic:%s  not config in share memory config file\n", __func__, topicName.c_str());
            return false;
        }
        WritableBlock wb;
        wb.index = memblock.index;
        return m_segMap[topicName]->ReleaseWrittenBlock(wb);
    }

    bool ShmManager::HoldMemory(const std::string &topicName, MemBlock &memblock)
    {
        if (m_segMap.find(topicName) == m_segMap.end())
        {
            AllocMemoryBasePtr segment = std::make_shared<AllocMemory>(topicName);
            m_segMap.insert(std::pair<std::string, AllocMemoryBasePtr>(topicName, segment));
        }
        ReadableBlock rb;
        rb.index = memblock.index;
        if (!m_segMap[topicName]->AcquireBlockToRead(&rb))
        {
            printf("%s AcquireBlockToRead failed, topic:%s  \n", __func__, topicName.c_str());
            return false;
        }
        memblock.buf = rb.buf;
        return true;
    }

    bool ShmManager::ReleaseMemory(const std::string &topicName, const MemBlock &memblock)
    {
        if (m_segMap.find(topicName) == m_segMap.end())
        {
            printf("%s failed, topic:%s  not config in share memory config file\n", __func__, topicName.c_str());
            return false;
        }
        ReadableBlock rb;
        rb.index = memblock.index;
        return m_segMap[topicName]->ReleaseReadBlock(rb);
    }

} // namespace shm::framework::memory