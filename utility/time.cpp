#include <sys/time.h>

#include "./time.hpp"

namespace utility {

std::string formatDateTime(const std::time_t t, bool gmt)
{
    std::tm tm;
    if (gmt) {
        ::gmtime_r(&t, &tm);
    } else {
        ::localtime_r(&t, &tm);
    }
    char buf[128];
    ::strftime(buf, sizeof(buf) - 1, "%Y-%m-%d %T", &tm);
    return buf;
}

std::pair<std::uint64_t, std::uint64_t> currentTime()
{
    timeval now;
    ::gettimeofday(&now, 0x0);
    return { now.tv_sec, now. tv_usec };
}

} // namespace utility

