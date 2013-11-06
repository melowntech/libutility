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

constexpr int EXEC_FAILED = 255;

void useFd(int dst, int src)
{
    if (src < 0) { return; }
    if (-1 == ::dup2(src, dst)) {
        std::system_error e(errno, std::system_category());
        LOG(warn3) << "dup2(2) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    }
    ::close(src);
}

pid_t execute(const ExecArgs &argv, int ifd = -1, int ofd = -1, int efd = -1)
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
        try {
            useFd(STDIN_FILENO, ifd);
            useFd(STDOUT_FILENO, ofd);
            useFd(STDERR_FILENO, efd);
        } catch (const std::exception &e) {
            std::exit(EXEC_FAILED);
        }

        // child -> exec
        if (::execvpe(argv.argv.front(), &(argv.argv.front())
                      , ::environ) == -1)
        {
            std::system_error e(errno, std::system_category());
            LOG(warn3) << "execve(2) failed: <" << e.code() << ", "
                       << e.what() << ">";
            std::exit(EXEC_FAILED);
        }

        // never reached
    }

    return pid;
}

template <typename File>
int getFd(const boost::optional<File> &f)
{
    if (f) { return f->fd; }
    return -1;
}

int systemImpl(const std::string &program, const Context &ctx)
{
    detail::ExecArgs argv;
    argv.arg(program);
    for (const auto arg : ctx.argv) {
        argv.arg(arg);
    }
    argv.finish();

    auto pid(detail::execute(argv, getFd(ctx.inFile), getFd(ctx.outFile)
                             , getFd(ctx.errFile)));
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

