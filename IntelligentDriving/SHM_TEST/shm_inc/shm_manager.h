/**
 * @file shm_manager.h
 */

#ifndef SHM_MANAGER_H_
#define SHM_MANAGER_H_
#include <csignal>
#include <memory>
#include "alloc_memory.h"
#include "shm_api.h"
namespace shm::framework::memory
{
    class ShmManager
    {
    public:
        /**
         * @brief Get the instance object
         * @return ShmManager&
         */
        static ShmManager &GetInstance();
        /**
         * @brief CreateSegment
         * @param config config path
         * @return true/false success or fail
         */
        bool CreateSegment(const std::string &config);

        /**
         * @brief Destory
         */
        void Destory();
        /**
         * @brief request block from segment for a topic
         *
         * @param topicName topic name
         * @param memblock block info
         * @return true/false success or fail
         */
        bool RequestMemory(const std::string &topicName, MemBlock &memblock);

        /**
         * @brief release write lock for a block in topicName segment
         *
         * @param topicName topic name
         * @param memblock block info
         * @return true/false success or fail
         */
        bool WriteFinish(const std::string &topicName, const MemBlock &memblock);

        /**
         * @brief aquire read lock for a block in topicName segment
         *
         * @param topicName topic name
         * @param memblock block info
         * @return true/false success or fail
         */
        bool HoldMemory(const std::string &topicName, MemBlock &memblock);

        /**
         * @brief release  read lock for a  block in topicName segment
         *
         * @param topicName topic name
         * @param memblock block info
         * @return true/false success or fail
         */
        bool ReleaseMemory(const std::string &topicName, const MemBlock &memblock);

    private:
        /**
         * @brief Construct a new shm Manager object
         *
         */
        ShmManager();

        /**
         * @brief dastory the shm Manager object
         *
         */
        virtual ~ShmManager();

        /**
         * @brief Construct a new shm Manager object
         */
        ShmManager(ShmManager &) = delete;
        /**
         * @brief operator=
         * @return ShmManager&
         */
        ShmManager &operator=(ShmManager &) = delete;

        /**
         * @brief Create segment from config
         * @return true/false success or fail
         */
        bool CreateSegmentFromConfig();
        //<topicName ,segment> map
        std::unordered_map<std::string, AllocMemoryBasePtr> m_segMap;
    };
} // namespace shm::framework::memory
#endif