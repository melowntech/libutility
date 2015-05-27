#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <system_error>

#include "dbglog/dbglog.hpp"

#include "filedes.hpp"

namespace utility {

// implement TEMP_FAILURE_RETRY if not present on platform (via C++11 lambda)
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(operation) [&]()->int {       \
        for (;;) { int e(operation);                     \
            if ((-1 == e) && (EINTR == errno)) continue; \
            return e;                                    \
        }                                                \
    }()
#endif

Filedes::~Filedes() {
    close();
}

void Filedes::close()
{
    if (fd_ >= 0) {
        TEMP_FAILURE_RETRY(::close(fd_));
    }
    fd_ = -1;
}

void Filedes::closeOnExec(bool value)
{
    // no log file -> fine
    if (fd_ < 0) { return; }

    int flags(::fcntl(fd_, F_GETFD, 0));
    if (flags == -1) {
        std::system_error e(errno, std::system_category());
        LOG(warn2) << "fcntl(" << fd_ << ", F_GETFD) failed: <"
                   << e.code() << ", " << e.what() << ">";
        throw e;
    }
    if (value) {
        flags |= FD_CLOEXEC;
    } else {
        flags &= ~FD_CLOEXEC;
    }

    if (::fcntl(fd_, F_SETFD, flags) == -1) {
        std::system_error e(errno, std::system_category());
        LOG(warn2) << "fcntl(" << fd_ << ", F_SETFD) failed: <"
                   << e.code() << ", " << e.what() << ">";
        throw e;
    }
}

} // namespace utility
