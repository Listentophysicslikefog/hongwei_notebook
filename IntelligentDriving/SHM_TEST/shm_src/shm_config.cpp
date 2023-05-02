/**
 * @file shm_config.cpp
 */
#include "shm_config.h"
#include "shm_log.h"
namespace shm::framework::memory
{
    ShmConfig &ShmConfig::GetInstance()
    {
        static ShmConfig instance;
        return instance;
    }

    ShmConfig::ShmConfig() {}
    ShmConfig::~ShmConfig() {}
    bool ShmConfig::LoadConfig(const std::string &path)
    {
        if (!path.size() || (MAX_FILE_LEN < path.size()))
        {
            printf("path name invaild");
            return false;
        }
        YAML::Node node = YAML::LoadFile(path);
        if (node.IsNull())
        {
            printf("YAML::LoadFile fail %s", path.c_str());
            return false;
        }

        return ParseConfig(node);
    }

    bool ShmConfig::ParseConfig(const YAML::Node &node)
    {
        for (uint8_t i = 0; i < node.size(); ++i)
        {
            ShmQosProp prop;
            if (!node[i][PROP_TOPIC_NAME])
            {
                printf("%s prop %s not configed", __func__, PROP_TOPIC_NAME);
                continue;
            }
            // get topic name
            if ((!node[i][PROP_SHARE_MEMORY]))
            {
                printf("%s prop %s not configed", __func__, PROP_SHARE_MEMORY);
                continue;
            }
            if ((!node[i][PROP_SHARE_MEMORY][PROP_BLOCK_SIZE]))
            {
                printf("%s prop %s not configed", __func__, PROP_BLOCK_SIZE);
                continue;
            }
            if ((!node[i][PROP_SHARE_MEMORY][PROP_BLOCK_COUNT]))
            {
                printf("%s prop %s not configed", __func__, PROP_BLOCK_COUNT);
                continue;
            }

            // get topic name
            prop.topicName = node[i][PROP_TOPIC_NAME].as<std::string>().c_str();
            // get share memory block size
            prop.blockSize = (MAX_BLOCK_SIZE < node[i][PROP_SHARE_MEMORY][PROP_BLOCK_SIZE].as<uint64_t>())
                                 ? MAX_BLOCK_SIZE
                                 : node[i][PROP_SHARE_MEMORY][PROP_BLOCK_SIZE].as<uint64_t>();

            // get share memory block count
            prop.blockCount = (MAX_BLOCK_COUNT < node[i][PROP_SHARE_MEMORY][PROP_BLOCK_COUNT].as<uint32_t>())
                                  ? MAX_BLOCK_COUNT
                                  : node[i][PROP_SHARE_MEMORY][PROP_BLOCK_COUNT].as<uint32_t>();
            m_propMap.insert(std::pair<std::string, ShmQosProp>(prop.topicName, prop));
        }
        return true;
    }

} // namespace shm::framework::memory