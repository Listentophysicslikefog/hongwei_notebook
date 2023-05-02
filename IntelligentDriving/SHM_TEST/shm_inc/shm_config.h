/**
 * @brief shm_confiig.h
 */
#ifndef SHM_CONFIG_H_
#define SHM_CONFIG_H_
#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include "shm_log.h"
namespace shm::framework::memory
{
    // qos config property parsed from user yaml config file
    struct ShmQosProp
    {
        // topic name
        std::string topicName = "";
        // block size
        uint64_t blockSize = 0;
        // block count
        uint32_t blockCount = 0;
        ShmQosProp() {}
        ShmQosProp &operator=(const ShmQosProp &other)
        {
            this->topicName = other.topicName;
            this->blockSize = other.blockSize;
            this->blockCount = other.blockCount;
            return *this;
        }
    };

    class ShmConfig
    {
    public:
        /**
         * @brief Get Instance object
         * @param path
         * @return ShmConfig&
         */
        static ShmConfig &GetInstance();
        /**
         * @brief load config file
         * @param path config file path
         * @return true/false success or fail
         */
        bool LoadConfig(const std::string &path);
        /**
         * @brief Get The Shm Qos Prop Map Object
         * @return std::unordered_map<std::string, ShmQosProp> &
         */
        inline std::unordered_map<std::string, ShmQosProp> &GetShmQosPropMap()
        {
            return m_propMap;
        }

    private:
        /**
         * @brief Construct a new share memory config
         */
        ShmConfig();
        /**
         * @brief Destory the share memory config object
         */
        virtual ~ShmConfig();
        /**
         * @brief Construct a new share memory config object
         */
        ShmConfig(ShmConfig &) = delete;
        /**
         * @brief
         * @return ShmCOnfig&
         */
        ShmConfig &operator=(ShmConfig &) = delete;
        /**
         * @brief parse config file and store content in ShmConfig
         * @param node Yaml node
         * @return true/false success or fail
         */
        bool ParseConfig(const YAML::Node &node);
        //<topicName, qos config> map
        std::unordered_map<std::string, ShmQosProp> m_propMap;
        // 255 bytes
        const uint8_t MAX_FILE_LEN = 255;
        // 32MB
        const uint32_t MAX_BLOCK_SIZE = 32 * 1024 * 1024;
        // 32
        const uint32_t MAX_BLOCK_COUNT = 32;
        // 10 count. total memory = max segment count * max segment size
        const uint8_t MAX_SEGMENT_COUNT = 10;
        const char *PROP_TOPIC_NAME = "TopicName";
        const char *PROP_SHARE_MEMORY = "ShareMemory";
        const char *PROP_BLOCK_SIZE = "BlockSize";
        const char *PROP_BLOCK_COUNT = "BlockCount";
    };

} // namespace shm::framework::memeory
#endif