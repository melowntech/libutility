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

#ifndef _WIN32
#include <pthread.h>
#endif

#include "dbglog/dbglog.hpp"

#include "thread.hpp"

namespace utility { namespace thread {

void setName(const std::string &name)
{
#ifndef _GNU_SOURCE
    LOG(warn3) << "pthread_setname_np unsupported";
#else
    auto useName((name.size() > 15) ? name.substr(0, 15) : name);
    if (::pthread_setname_np(::pthread_self(), useName.c_str())) {
        LOG(warn3) << "pthread_setname_np failed";
    } else {
        LOG(info4) << "set name to: <" << useName << ">";
    }
#endif
}

void appendName(const std::string &name)
{
#ifndef _GNU_SOURCE
    LOG(warn3) << "pthread_setname_np unsupported";
#else
    // fetch original name
    char buf[16];
    if (::pthread_getname_np(::pthread_self(), buf, sizeof(buf))) {
        LOG(warn3) << "pthread_getname_np failed";
        return;
    }

    auto useName(buf + name);
    if (useName.size() > 15) { useName = useName.substr(0, 15); }

    if (::pthread_setname_np(::pthread_self(), useName.c_str())) {
        LOG(warn3) << "pthread_setname_np failed";
    }
#endif
}

} } // namespace utility::thread
