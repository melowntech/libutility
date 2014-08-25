#ifndef shared_utility_procstat_hpp_included_
#define shared_utility_procstat_hpp_included_

#include <cstddef>
#include <vector>

namespace utility {

struct ProcStat
{
    long pid;
    std::size_t rss;

    typedef std::vector<ProcStat> list;
};

typedef std::vector<long> PidList;

ProcStat::list getProcStat(const PidList &pids);

/** Returns self process statistics.
 */
ProcStat getProcStat();

} // namespace utility

#endif // shared_utility_procstat_hpp_included_
