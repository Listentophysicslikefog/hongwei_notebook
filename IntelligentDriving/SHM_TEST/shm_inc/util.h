/**
 * @file : util.h
 */

#ifndef CYBER_COMMON_UTIL_H_
#define CYBER_COMMON_UTIL_H_

#include <string>

namespace shm::framework::memory
{

    inline std::size_t Hash(const std::string &key)
    {
        return std::hash<std::string>{}(key);
    }

} // namespace shm::framework

#if !defined(RETURN_VAL_IF_NULL)
#define RETURN_VAL_IF_NULL(ptr, val) \
    if (!ptr)                        \
    {                                \
        return val;                  \
    }
#endif

#endif // CYBER_COMMON_UTIL_H_