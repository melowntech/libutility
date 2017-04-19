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

#include "./procstat.hpp"

#include "dbglog/dbglog.hpp"

namespace utility {

namespace {
std::size_t pageSize(::sysconf(_SC_PAGESIZE));
std::size_t pageSizeKb(pageSize >> 10);
}

ProcStat::list getProcStat(const PidList &pids)
{
    std::vector< ::pid_t> pidList(pids.begin(), pids.end());
    pidList.push_back(0);

    struct Table {
        Table(const ::pid_t *pids)
            : p(::openproc(PROC_FILLSTATUS | PROC_FILLMEM | PROC_PID, pids))
        {
            if (!p) {
                std::system_error e(errno, std::system_category());
                LOG(err3) << "Cannot open /proc: <"
                          << e.code() << ", " << e.what() << ">.";
                throw e;
            }
        }

        ~Table() {
            ::closeproc(p);
        }
        ::PROCTAB *p;
    } table(pidList.data());

    ::proc_t *proc(nullptr);

    ProcStat::list stat;

    for (const auto &pid : pids) {
        errno = 0;
        proc = ::readproc(table.p, proc);
        if (!proc) {
            if (!errno) { errno = ESRCH; }
            std::system_error e(errno, std::system_category());
            LOG(err3) << "Cannot read from /proc (pid " << pid << "): <"
                      << e.code() << ", " << e.what() << ">.";
            throw e;
        }
        ProcStat ps;
        ps.pid = proc->tid;
        ps.rss = proc->vm_rss;
        ps.swap = proc->vm_swap;
        ps.virt = proc->vsize / pageSizeKb;
        ps.shared = proc->share / pageSizeKb;
        stat.push_back(ps);
    }
    return stat;
}

ProcStat getProcStat()
{
    return getProcStat({::getpid()}).front();
}

} // namespace utility
