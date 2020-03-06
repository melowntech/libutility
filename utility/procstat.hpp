/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef shared_utility_procstat_hpp_included_
#define shared_utility_procstat_hpp_included_

#include <cstddef>
#include <vector>

namespace utility {

struct ProcStat
{
    typedef long Pid;

    typedef long Uid;

    /** Process ID.
     */
    Pid pid;

    /** Parent process ID.
     */
    Pid ppid;

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

typedef std::vector<ProcStat::Pid> PidList;
typedef std::vector<ProcStat::Uid> UidList;

/** Get statistics for given processes. Return all processes if list is empty.
 */
ProcStat::list getProcStat(const PidList &pids);

/** Get statistics for all processes for given users.
 */
ProcStat::list getUserProcStat(const UidList &uids);

/** Get statistics for all processes for given user.
 */
ProcStat::list getUserProcStat(ProcStat::Uid uid);

/** Returns self process statistics.
 */
ProcStat getProcStat();

} // namespace utility

#endif // shared_utility_procstat_hpp_included_
