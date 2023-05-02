/**
 * @file :lockless.h
 */

#ifndef INCLUDE_LOCKLESS_H_
#define INCLUDE_LOCKLESS_H_
#include <array>
#include <atomic>
#include <cstring>

namespace shm::framework::common
{
    template <uint32_t Size>
    class LocklessSet
    {
    public:
        LocklessSet();
        LocklessSet &operator=(const LocklessSet &);
        /**
         * @brief insert pid
         * @param pid
         * @return true
         * @return false: probed elem diff with all set
         */
        bool Insert(uint32_t elem);
        /**
         * @brief remove pid
         * @param pid
         * @return true :success
         * @return false: remove fail
         */
        bool Remove(uint32_t elem);

        std::array<std::atomic_uint32_t, Size> m_locklessSet = {};
    };

    template <uint32_t Size>
    LocklessSet<Size>::LocklessSet() = default;

    template <uint32_t Size>
    LocklessSet<Size> &
    LocklessSet<Size>::operator=(const LocklessSet &set)
    {
        for (uint32_t idx = 0; idx < Size; ++idx)
        {
            m_locklessSet[idx].store(set.m_locklessSet[idx].load());
        }
        return *this;
    }

    template <uint32_t Size>
    bool LocklessSet<Size>::Insert(uint32_t elem)
    {
        for (uint32_t idx = 0; idx < Size; ++idx)
        {
            auto probedElem = m_locklessSet[idx].load();
            if (probedElem != elem)
            {
                if (probedElem != 0)
                {
                    continue;
                }
                uint32_t exp = 0;
                if (m_locklessSet[idx].compare_exchange_strong(exp, elem))
                {
                    return true;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                // alread record current pid, no need to insert
                return true;
            }
        }
        return false;
    }

    template <uint32_t Size>
    bool LocklessSet<Size>::Remove(uint32_t elem)
    {
        for (uint32_t idx = 0; idx < Size; ++idx)
        {
            auto probedElem = m_locklessSet[idx].load();
            if (probedElem == elem)
            {
                return m_locklessSet[idx].compare_exchange_strong(elem, 0);
            }
        }
        return false;
    }

} // namespace shm

#endif