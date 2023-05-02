/**
 * @file state.cpp
 */
#include "state.h"
namespace shm::framework::memory
{
    State::State(const uint64_t &ceilingMsgSize)
        : m_ceilingMsgSize(ceilingMsgSize) {}

    State::State(const uint64_t &ceilingMsgSize, const uint32_t blockNum)
        : m_ceilingMsgSize(ceilingMsgSize), m_blockNum(blockNum) {}

    State::~State() {}

} // namespace shm::framework::memory