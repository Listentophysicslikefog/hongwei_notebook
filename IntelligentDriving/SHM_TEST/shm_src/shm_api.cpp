/**
 * @file shm_api.cpp
 */
#include "shm_api.h"
#include "shm_manager.h"
using namespace shm::framework::memory;

bool ShareMemory_Create(const std::string &configFile)
{
    printf("%s config file %s\n", __func__, configFile.c_str());
    return ShmManager::GetInstance().CreateSegment(configFile);
}

void ShareMemory_Destroy()
{
    printf("%s", __func__);
    ShmManager::GetInstance().Destory();
}

bool ShareMemory_RequestMemory(const std::string &topicName, MemBlock &memblock)
{
    if (ShmManager::GetInstance().RequestMemory(topicName, memblock))
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock success\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return true;
    }
    else
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock failed\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return false;
    }
}

bool ShareMemory_WriteFinish(const std::string &topicName, const MemBlock &memblock)
{
    if (ShmManager::GetInstance().WriteFinish(topicName, memblock))
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock success\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return true;
    }
    else
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock failed\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return false;
    }
}

bool ShareMemory_HoldMemory(const std::string &topicName, MemBlock &memblock)
{
    if (ShmManager::GetInstance().HoldMemory(topicName, memblock))
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock success\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return true;
    }
    else
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock failed\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return false;
    }
}

bool ShareMemory_ReleaseMemory(const std::string &topicName, const MemBlock &memblock)
{
    if (ShmManager::GetInstance().ReleaseMemory(topicName, memblock))
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock success\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return true;
    }
    else
    {
        printf("%s topicName:%s  blockId: %u size:%ld buf: %p  write lock failed\n",
               __func__, topicName.c_str(), memblock.index, memblock.size, memblock.buf);
        return false;
    }
}