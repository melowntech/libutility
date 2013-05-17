#include <iostream>
#include <cerrno>
#include <cstdlib>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "dbglog/dbglog.hpp"

#include "process.hpp"

namespace utility { namespace detail {
struct ExecArgs {
    typedef std::vector<char*> Argv;
    Argv argv;

    ~ExecArgs() {
        for (auto arg : argv) { std::free(arg); }
    }

    void arg(const std::string &arg) {
        argv.push_back(strdup(arg.c_str()));
    }

    void finish() { argv.push_back(nullptr); }
};

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const ExecArgs &a)
{
    bool first = true;
    for (const auto arg : a.argv) {
        os << (first ? "" : " ") << arg;
        first = false;
    }
    return os;
}


pid_t execute(const ExecArgs &argv)
{
    LOG(info2) << "Executing: " << argv;

    auto pid(::fork());
    if (-1 == pid) {
        // Oops, failed
        std::system_error e(errno, std::system_category());
        LOG(warn3) << "fork(2) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    } else if (0 == pid) {
        // child -> exec
        if (::execvpe(argv.argv.front(), &(argv.argv.front())
                      , ::environ) == -1)
        {
            std::system_error e(errno, std::system_category());
            LOG(warn3) << "execve(2) failed: <" << e.code() << ", "
                       << e.what() << ">";
            std::exit(-1);
        }

        // never reached
    }

    return pid;
}

int systemImpl(const std::string &program
               , const std::vector<std::string> &args)
{
    detail::ExecArgs argv;
    argv.arg(program);
    for (const auto arg : args) {
        argv.arg(arg);
    }
    argv.finish();

    auto pid(detail::execute(argv));
    LOG(info2) << "running under pid: " << pid;

    int status;
    for (;;) {
        auto res(::waitpid(pid, &status, 0x0));
        if (res < 0) {
            if (EINTR == errno) { continue; }

            std::system_error e(errno, std::system_category());
            LOG(warn3) << "waitpid(2) failed: <" << e.code()
                       << ", " << e.what() << ">";
            throw e;
        }
        break;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

int spawnImpl(const std::function<int ()> &func)
{
    auto pid(::fork());
    if (-1 == pid) {
        // Oops, failed
        std::system_error e(errno, std::system_category());
        LOG(warn3) << "fork(2) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    } else if (0 == pid) {
        std::exit(func());
    }

    return pid;
}

} } // namespace utility::detail

