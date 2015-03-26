#include <unistd.h>

#include <cerrno>

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

} // namespace utility
