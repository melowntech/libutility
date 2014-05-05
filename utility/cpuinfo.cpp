#include <boost/thread.hpp>

namespace utility {

std::size_t cpuCount()
{
    // TODO: use sched_getaffinity instead
    auto hc(boost::thread::hardware_concurrency());
    if (!hc) { return 1; }
    return hc;
}

} // namespace utility
