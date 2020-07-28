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

#include <io.h>

#include <cerrno>
#include <system_error>

#include "dbglog/dbglog.hpp"

#include "../filedes.hpp"

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

Filedes::Filedes(int fd) : fd_(fd)
{
    if (valid())
        _setmode(fd_, _O_BINARY);
}

Filedes::Filedes(int fd, const boost::filesystem::path &path)
    : fd_(fd), path_(path)
{
    if (valid())
        _setmode(fd_, _O_BINARY);
}

Filedes::~Filedes() {
    close();
}

void Filedes::close()
{
    if (valid()) {
        TEMP_FAILURE_RETRY(::_close(fd_));
    }
    fd_ = -1;
}

void Filedes::closeOnExec(bool value)
{
    // UNIMPLEMENTED
    LOG(warn2) << "closeOnExec unimplemented on Windows.";
    (void) value;
}

Filedes Filedes::dup() const
{
    if (fd_ < 0) { return { fd_, path_ }; }
    const auto fd(::_dup(fd_));
    if (fd == -1) {
        std::system_error e(errno, std::system_category());
        LOG(warn2) << "dup(" << fd_ << ") failed: "
                   << e.code() << ", " << e.what() << ">";
    }

    return { fd, path_ };
}

bool Filedes::valid() const
{
    if (fd_ < 0) { return false; }
    // TODO: what to do?
    return true;
}

} // namespace utility
