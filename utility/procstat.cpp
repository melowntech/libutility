#include <sys/types.h>

#include <cerrno>
#include <system_error>

#include <proc/readproc.h>

#include "./procstat.hpp"

#include "dbglog/dbglog.hpp"

namespace utility {

ProcStat::list getProcStat(const PidList &pids)
{
    std::vector< ::pid_t> pidList(pids.begin(), pids.end());
    pidList.push_back(0);

    struct Table {
        Table(const ::pid_t *pids)
            : p(::openproc(PROC_FILLSTATUS | PROC_PID, pids))
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
        stat.push_back(ps);
    }
    return stat;
}

ProcStat getProcStat()
{
    return getProcStat({::getpid()}).front();
}

} // namespace utility
