/**
 * Copyright (c) 2020 Melown Technologies SE
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

#include <sys/syscall.h>

#ifdef __NR_memfd_create
#  include <linux/memfd.h>
#else
#  error "Missing __NR_memfd_create syscall."
#endif

#include "dbglog/dbglog.hpp"

#include "../format.hpp"
#include "../memoryfile.hpp"

namespace utility {

namespace {

inline int memfd_create(const char *name, unsigned int flags) {
    return ::syscall(__NR_memfd_create, name, flags);
}

inline int memfdFlags(int flags) {
    int out(0);
    if (flags & MemoryFileFlag::closeOnExec) { out |= MFD_CLOEXEC; }
    if (flags & MemoryFileFlag::allowSealing) { out |= MFD_ALLOW_SEALING; }
    return out;
}

} // namespace

Filedes memoryFile(const std::string &name, int flags)
{
    utility::Filedes fd(memfd_create(name.c_str(), memfdFlags(flags)));
    if (!fd) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot create memory fd for name <" << name << ">: <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    const auto path(utility::format("/proc/%s/fd/%s", ::getpid(), fd.get()));
    return Filedes(fd.release(), path);
}

} // namespace utility

