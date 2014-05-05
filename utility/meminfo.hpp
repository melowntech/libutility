#ifndef utility_meminfo_hpp_included_
#define utility_meminfo_hpp_included_

#include <cstddef>

namespace utility {

struct MemInfo {
    struct Mem {
        std::size_t total;
        std::size_t free;
        std::size_t buffers;

        std::size_t available() const { return free + buffers; }
    };
    Mem ram;
    Mem swap;
};

MemInfo meminfo();

} // namespace utility

#endif // utility_meminfo_hpp_included_
