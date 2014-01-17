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

} // namespace utility

