/**
 * @file alloc_memory.h
 */
#ifndef ALLOC_MEMORY_H_
#define ALLOC_MEMORY_H_
#include <string>
#include "alloc_memory_base.h"
namespace shm::framework::memory
{
    class AllocMemory : public AllocMemoryBase
    {
    public:
        /**
         * @brief Construct a new Posix Segment object for a topic
         * @param topicName topic name
         */
        explicit AllocMemory(const std::string &topicName);

        /**
         * @brief Construct a new Posix Segment object for a topic with qos config
         * @param topicName topic name
         * @param prop qos config property
         */
        explicit AllocMemory(const std::string &topicName, const ShmQosProp &prop);

        /**
         * @brief destory the posix segment object
         */
        virtual ~AllocMemory();

    private:
        /**
         * @brief reset shm memory
         */
        void Reset() override;

        /**
         * @brief remove shm memory
         * @return false/true fail or success
         */
        bool Remove() override;

        /**
         * @brief open shm memory
         * @return false/true fail or success
         */
        bool OpenOnly() override;

        /**
         * @brief create shm memory
         * @return false/true fail or success
         */
        bool Create() override;

        std::string m_shmName; // shm key
        const std::string SHM_KEY_PREFIX = "SHM_";
        const mode_t DEFAULT_PERMISSION = 0644;
    };

} // namespace shm::framework::memory
#endif