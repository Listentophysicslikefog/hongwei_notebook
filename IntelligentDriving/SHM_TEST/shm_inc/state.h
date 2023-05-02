/**
 * @brief state.h
 */
#ifndef STATE_H_
#define STATE_H_

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include "shm_conf.h"
namespace shm::framework::memory
{
    class State
    {
    public:
        /**
         * @brief Construct a new state object
         * @param ceilingMsgSize data size
         */
        explicit State(const uint64_t &ceilingMsgSize);
        /**
         * @brief Construct a new state object
         * @param ceilingMsgSize data size
         * @param blockNum block num
         */
        explicit State(const uint64_t &ceilingMsgSize, const uint32_t blockNum);
        /**
         * @brief destroy the state object
         */
        virtual ~State();
        /**
         * @brief reference count -1
         */
        void DecreaseReferenceCounts()
        {
            uint32_t currentReferenceCount = m_referenceCount.load();
            do
            {
                if (!currentReferenceCount)
                {
                    return;
                }
                // reference decrease
            } while (!m_referenceCount.compare_exchange_strong(
                currentReferenceCount, currentReferenceCount - 1));
        }
        /**
         * @brief reference count +1
         */
        void IncreaseReferenceCounts() { m_referenceCount.fetch_add(1); }
        /**
         * @brief add seq
         * @param diff
         * @return seq
         */
        uint32_t FetchAddSeq(uint32_t diff) { return m_seq.fetch_add(diff); }
        /**
         * @brief get seq
         * @return seq
         */
        uint32_t Seq() { return m_seq.load(); }
        /**
         * @brief Set the Need Remap object
         * @param need
         */
        void SetNeedRemap(bool need) { m_needRemap.store(need); }
        /**
         * @brief get need remap flag
         * @return true
         * @return false
         */
        bool NeedRemap() { return m_needRemap; }
        /**
         * @brief ceil msg size
         * @return m_ceilingMsgSize
         */
        uint64_t CeilingMsgSize() { return m_ceilingMsgSize.load(); }
        /**
         * @brief block num
         * @return m_blockNum
         */
        uint64_t BlockNum() { return m_blockNum.load(); }
        /**
         * @brief block num
         * @return m_referenceCount
         */
        uint32_t ReferenceCount() { return m_referenceCount.load(); }

    private:
        std::atomic<bool> m_needRemap = {false};      // need remap flag
        std::atomic<uint32_t> m_seq = {0};            // seq
        std::atomic<uint32_t> m_referenceCount = {0}; // reference count
        std::atomic<uint64_t> m_ceilingMsgSize;       // ceil msg size
        std::atomic<uint32_t> m_blockNum;             // block num
    };

} // namespace shm::framework::memory
#endif