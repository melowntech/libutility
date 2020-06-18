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

#ifndef utility_logging_hpp_included_
#define utility_logging_hpp_included_

#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"

#include "format.hpp"

namespace utility {

/** RAII class for temporarily setting dbglog thread ID.
 */
class LogThreadId {
public:
    LogThreadId(const std::string &id);

    template <typename ...Args>
    LogThreadId(const std::string &format, Args &&...args);

    template <typename T> LogThreadId(const T &value);

    ~LogThreadId();

private:
    std::string saved_;
};

// implementation

inline LogThreadId::LogThreadId(const std::string &id)
    : saved_(dbglog::thread_id())
{
    dbglog::thread_id(id);
}

template <typename ...Args>
LogThreadId::LogThreadId(const std::string &format, Args &&...args)
    : saved_(dbglog::thread_id())
{
    dbglog::thread_id(utility::format(format, std::forward<Args>(args)...));
}

template <typename T>
LogThreadId::LogThreadId(const T &value)
    : saved_(dbglog::thread_id())
{
    dbglog::thread_id(boost::lexical_cast<std::string>(value));
}

inline LogThreadId::~LogThreadId() {
    try {
        dbglog::thread_id(saved_);
    } catch (...) {}
}

} // namespace utility

#endif // utility_logging_hpp_included_
