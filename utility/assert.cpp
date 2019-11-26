/**
 * Copyright (c) 2019 Melown Technologies SE
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

#include "assert.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <signal.h>
#include <sys/stat.h>
#include <cassert>
#include <unistd.h>

namespace utility {
namespace detail {

    bool isDebuggerPresent() {
#ifdef __GNUC__
        char buf[1024];
        bool debuggerPresent = false;

        int status_fd = open("/proc/self/status", O_RDONLY);
        if (status_fd == -1) {
            return false;
        }

        std::size_t num_read = ::read(status_fd, buf, sizeof(buf));

        if (num_read > 0) {
            static const char TracerPid[] = "TracerPid:";
            char* tracer_pid;

            buf[num_read] = 0;
            tracer_pid = strstr(buf, TracerPid);
            if (tracer_pid) {
                debuggerPresent = atoi(tracer_pid + sizeof(TracerPid) - 1) != 0;
            }
        }
        return debuggerPresent;
#else
        /// \todo implement other platforms
        return false;
#endif
    }

    void doAssert(const char* message, const char* file, const char* func, const int line,
        const char* param) {
        static std::mutex mutex;
        std::unique_lock<std::mutex> lock(mutex);

        std::cout << "============================================================================="
                     "============="
                  << std::endl;
        std::cout << "Assertion failed in " << file << ", executing function " << func
                  << " on line " << line << std::endl;
        std::cout << "Condition: " << message << std::endl;
        if (param != nullptr) {
            std::cout << "Parameter: " << param << std::endl;
        }
        std::cout << "============================================================================="
                     "============="
                  << std::endl;

        if (isDebuggerPresent()) {
            ::raise(SIGTRAP);
        }

        assert(false);
    }

} // namespace detail

} // namespace utility
