/**
 * @file shm_api.h
 */
#ifndef SHM_API_H_
#define SHM_API_H_

#ifdef __cplusplus

#include <memory.h>
#include <cstdint>
#include <string>

#define IMPORT
typedef struct MemBlock_
{
    // 写： 由用户写入表示希望向中间件申请的内存大小，当大小超过内存块大小时报错
    // 读：由用户写入收到消息中携带的大小
    size_t size = 0;
    // 写：由中间件返回可写内存块号
    // 读：由用户写入收到消息中携带的内存块号
    int32_t index = 0;
    // 写/读：由中间件返回的可写/读的起始地址
    uint8_t *buf = nullptr;

} MemBlock;

/**
 * @brief 共享内存创建接口
 *
 * @param configFile 共享内存配置文件
 * @return true/false is success or fail
 */
IMPORT bool ShareMemory_Create(const std::string &configFile);

/**
 * @brief 共享内存销毁接口哦
 *
 */
IMPORT void ShareMemory_Destroy();

/**
 * @brief 为topicName申请共享内存用来写入，内部会加写锁
 *
 * @param[in] topicName topic name
 * @param[out] memblock 内存段描述信息
 * @return true/false is success or fail
 */
IMPORT bool ShareMemory_RequestMemory(const std::string &topicName, MemBlock &memblock);

/**
 * @brief 释放topicName对应的共享内存段的写锁，需要和ShareMemory_RequestMemory成对使用
 *
 * @param[in] topicName topic name
 * @param[out] memblock 内存段描述信息
 * @return true/false is success or fail
 */
IMPORT bool ShareMemory_WriteFinish(const std::string &topicName, const MemBlock &memblock);

/**
 * @brief 为topicName申请共享内存用来读出数据，内部会加读锁
 *
 * @param[in] topicName topic name
 * @param[out] memblock 内存段描述信息
 * @return true/false is success or fail
 */
IMPORT bool ShareMemory_HoldMemory(const std::string &topicName, MemBlock &memblock);

/**
 * @brief 释放topicName对应的共享内存段的读锁，需要和ShareMemory_HoldMemory成对使用
 *
 * @param[in] topicName topic name
 * @param[out] memblock 内存段描述信息
 * @return true/false is success or fail
 */
IMPORT bool ShareMemory_ReleaseMemory(const std::string &topicName, const MemBlock &memblock);
#endif

#endif
