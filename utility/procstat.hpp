#ifndef shared_utility_procstat_hpp_included_
#define shared_utility_procstat_hpp_included_

#include <cstddef>
#include <vector>

namespace utility {

struct ProcStat
{
    long pid;

    /** Resident memory in KiB.
     */
    std::size_t rss;

    /** Virtual memory in KiB.
     */
    std::size_t virt;

    /** Swapped memory in KiB.
     */
    std::size_t swap;

    /** Shared memory in KiB.
     */
    std::size_t shared;

    /** Real memory occupied by this process (resident + swapped).
     */
    std::size_t occupies() const { return rss + swap; }

    typedef std::vector<ProcStat> list;
};

typedef std::vector<long> PidList;

ProcStat::list getProcStat(const PidList &pids);

/** Returns self process statistics.
 */
ProcStat getProcStat();

} // namespace utility

#endif // shared_utility_procstat_hpp_included_
