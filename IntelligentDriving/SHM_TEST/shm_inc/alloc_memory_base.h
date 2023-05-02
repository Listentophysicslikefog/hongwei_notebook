/**
 * @file alloc_memory_base.h
 */
#ifndef ALLOC_MEMORY_BAXE_H_
#define ALLOC_MEMORY_BAXE_H_
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "block.h"
#include "shm_api.h"
#include "shm_conf.h"
#include "shm_config.h"
#include "state.h"
#include "util.h"
namespace shm::framework::memory
{
    class AllocMemoryBase;
    using AllocMemoryBasePtr = std::shared_ptr<AllocMemoryBase>;
    struct WritableBlock
    {
        uint32_t index = 0;
        Block *block = nullptr;
        uint8_t *buf = nullptr;
    };

    using ReadableBlock = WritableBlock;
    class AllocMemoryBase
    {
    public:
        /**
         * @brief Construct a new AllocMemoryBase object for a topic
         * @param topicName topic name
         */
        explicit AllocMemoryBase(const std::string &topicName);

        /**
         * @brief Construct a new AllocMemoryBase object for topic name with qos config
         * @param topicName topic name
         * @param prop qos config property
         */
        AllocMemoryBase(const std::string &topicName, const ShmQosProp &prop);

        /**
         * @brief Destory the AllocMemoryBase object
         */
        virtual ~AllocMemoryBase() {}

        /**
         * @brief get block to write
         * @param msgSize data size
         * @param writableBlock block info for writing
         * @return false/true fail or success
         */
        bool AcquireBlockToWrite(const std::size_t msgSize, WritableBlock *const writableBlock);

        /**
         * @brief release write lock to written block
         * @param writableBlock block info for writing
         */
        bool ReleaseWrittenBlock(const WritableBlock &writableBlock);

        /**
         * @brief get block to read
         * @param msgSize data size
         * @param readableBlock block info for reading
         * @return false/true fail or success
         */
        bool AcquireBlockToRead(ReadableBlock *const readableBlock);

        /**
         * @brief release read lock to read block
         * @param readableBlock block info for reading
         */
        bool ReleaseReadBlock(const ReadableBlock &readableBlock);

        /**
         * @brief create shm
         * @return true/false success or fail
         */
        virtual bool Create() = 0;

    protected:
        /**
         * @brief Destroy AllocMemoryBase reference count -1
         * @return true/false success or fail
         */
        virtual bool Destroy();
        /**
         * @brief reset shm
         */
        virtual void Reset() = 0;

        /**
         * @brief shm_unlink
         * @return true/false success or fail
         */
        virtual bool Remove() = 0;
        /**
         * @brief open only shm
         * @return true/false success or fail
         */
        virtual bool OpenOnly() = 0;

        bool m_init;                                             // init flag
        ShmConf m_conf;                                          // shm conf
        std::string m_topicName;                                 // topic name
        State *m_state;                                          // shm state
        Block *m_blocks;                                         // shm blocks
        void *m_managedShm;                                      // shm addr
        std::mutex m_blockBufLock;                               // block buf mutex locks
        std::unordered_map<uint32_t, uint8_t *> m_blockBufAddrs; // block buf addrs
    protected:
        /**
         * @brief reset and open only current shm
         * @return false/true fail or success
         */
        bool Remap();
        /**
         * @brief reset, update shm size,create shm
         *
         * @param msgSize new data size
         * @param blockNum new block num
         * @return false/true fail or success
         */
        bool Recreate(const uint64_t &msgSize, const uint32_t blockNum);

        /**
         * @brief Get the next writable block index object and lock the memory
         * @return uint32_t block index
         */
        uint32_t GetNextWritableBlockIndex();
    };

} // namespace shm::framework::memory
#endif
