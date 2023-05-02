/**
 * @file :macros.h
 */
#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

#include <sys/stat.h>
#include <iostream>
#include <string>
#include <chrono>

#define TIMESCALE std::chrono::microseconds
#define TIMESCALE_COUNT 1e6
#define TIMESCALE_NAME "us"
#define KERNEL_DEFAULT_PROCESS 0
namespace shm::framework::common
{
    /**
     * @brief get current time
     * @param NA
     * @return current time uint us
     */
    inline uint64_t CurrentTime()
    {
        auto timeSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
        auto castedTime = std::chrono::duration_cast<TIMESCALE>(timeSinceEpoch);
        return castedTime.count();
    }
} // namespace shm

/**
 * @brief get lock with no-blocking mode
 * @param NA
 * @return true: check process alive
 * @return false: check process dead
 */
inline bool ProcDead(__pid_t proc)
{
    if (KERNEL_DEFAULT_PROCESS == proc)
    {
        return false;
    }
    const std::string pidPath = "/proc/" + std::to_string(proc);
    struct stat sts
    {
    };
    return ((stat(pidPath.c_str(), &sts) < 0) && (ENOENT == errno));
}

#endif
