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
#include <unistd.h>
#include <sys/types.h>

#include <cerrno>
#include <system_error>

#include <proc/readproc.h>

#include "procstat.hpp"

#include "dbglog/dbglog.hpp"

namespace utility {

std::size_t ProcStat::ClocksPerSecond(::sysconf(_SC_CLK_TCK));

namespace {
std::size_t pageSize(::sysconf(_SC_PAGESIZE));
std::size_t pageSizeKb(pageSize >> 10);

inline int flags(const ::pid_t *pids) {
    int flags(PROC_FILLSTATUS | PROC_FILLMEM);
    if (pids) { flags |= PROC_PID; }
    return flags;
}

struct PidTag {};
struct UidTag {};

typedef std::vector< ::pid_t> Pids;

Pids makePidList(const PidList &pids)
{
    if (pids.empty()) { return {}; }
    std::vector< ::pid_t> pidList(pids.begin(), pids.end());
    pidList.push_back(0);
    return pidList;
}

PROCTAB* openPids(const Pids &pids)
{
    int flags(PROC_FILLSTATUS | PROC_FILLMEM);
    if (!pids.empty()) { flags |= PROC_PID; }

    auto p(::openproc(flags, pids.data()));
    if (!p) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot open /proc: <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
    return p;
}

PROCTAB* openUids(const UidList &uids)
{
    int flags(PROC_FILLSTATUS | PROC_FILLMEM | PROC_UID);
    std::vector< ::uid_t> uidList(uids.begin(), uids.end());

    auto p(::openproc(flags, uids.data(), int(uids.size())));
    if (!p) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot open /proc: <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
    return p;
}

class Table {
public:
    Table(const PidList &pids, const PidTag&)
        : pidList(makePidList(pids))
        , p(openPids(pidList)), proc()
    {}

    Table(const UidList &uids, const UidTag&)
        : p(openUids(uids)), proc()
    {}

    ~Table() {
        if (proc) { ::freeproc(proc); }
        if (p) { ::closeproc(p); }
    }

    const ::proc_t* next() {
        auto item(::readproc(p, proc));
        if (item && !proc) { proc = item; }
        return item;
    }

private:
    Pids pidList;
    ::PROCTAB *p;
    ::proc_t *proc;
};

void fill(ProcStat &ps, const ::proc_t *proc)
{
    ps.pid = proc->tid;
    ps.ppid = proc->ppid;
    ps.rss = proc->vm_rss;
    ps.swap = proc->vm_swap;
    ps.virt = proc->size / pageSizeKb;
    ps.shared = proc->share / pageSizeKb;

    ps.utime = proc->utime;
    ps.stime = proc->stime;
    ps.cutime = proc->cutime;
    ps.cstime = proc->cstime;
}

} // namespace

ProcStat::list getProcStat(const PidList &pids)
{
    Table table(pids, PidTag{});

    ProcStat::list stat;

    auto ipids(pids.begin());
    auto epids(pids.end());
    while (const auto *proc = table.next()) {
        if (ipids != epids) {
            const auto pid(*ipids++);
            if (pid != proc->tid) {
                std::system_error e(ESRCH, std::system_category());
                LOG(err3) << "Cannot get process " << pid << " information.";
                throw e;
            }
        }

        stat.emplace_back();
        fill(stat.back(), proc);
    }

    if (ipids != epids) {
        std::system_error e(ESRCH, std::system_category());
        LOG(err3) << "Not all processes found.";
        throw e;
    }

    return stat;
}

ProcStat::list getUserProcStat(const UidList &uids)
{
    Table table(uids, UidTag{});

    ProcStat::list stat;

    while (const auto *proc = table.next()) {
        stat.emplace_back();
        fill(stat.back(), proc);
    }

    return stat;
}

ProcStat::list getUserProcStat(ProcStat::Uid uid)
{
    return getUserProcStat(UidList{uid});
}

ProcStat::list getUserProcStat()
{
    return getUserProcStat(UidList{::getuid()});
}

ProcStat getProcStat()
{
    return getProcStat(PidList{::getpid()}).front();
}

} // namespace utility
