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

#include <optional>

#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"

#include "format.hpp"

namespace utility {

/** RAII class for temporarily setting dbglog thread ID.
 */
class LogThreadId {
public:
    struct Append {};

    LogThreadId(const std::optional<std::string> &id);
    LogThreadId(Append, const std::optional<std::string> &id);

    LogThreadId(const std::string &id);
    LogThreadId(Append, const std::string &id);

    template <typename ...Args>
    LogThreadId(const std::string &format, Args &&...args);
    template <typename ...Args>
    LogThreadId(Append, const std::string &format, Args &&...args);

    template <typename T> LogThreadId(const T &value);
    template <typename T> LogThreadId(Append, const T &value);

    ~LogThreadId();

private:
    std::string saved_;
};

#define UTILITY_LOGSETID(...) \
    utility::LogThreadId DBGLOG_ADD_LINE_NO(utility_LogThreadId_)(__VA_ARGS__)

#define UTILITY_LOGAPPENDID(...)                                        \
    utility::LogThreadId DBGLOG_ADD_LINE_NO(utility_LogThreadId_) \
        (utility::LogThreadId::Append{}, __VA_ARGS__)

// implementation

inline LogThreadId::LogThreadId(const std::optional<std::string> &id)
    : saved_(dbglog::thread_id())
{
    if (id) { dbglog::thread_id(*id); }
}

inline LogThreadId::LogThreadId(Append, const std::optional<std::string> &id)
    : saved_(dbglog::thread_id())
{
    if (id) { dbglog::thread_id(saved_ + "/" + *id); }
}

inline LogThreadId::LogThreadId(const std::string &id)
    : saved_(dbglog::thread_id())
{
    dbglog::thread_id(id);
}

inline LogThreadId::LogThreadId(Append, const std::string &id)
    : LogThreadId(dbglog::thread_id() + "/" + id)
{}

template <typename ...Args>
LogThreadId::LogThreadId(const std::string &format, Args &&...args)
    : LogThreadId(utility::format(format, std::forward<Args>(args)...))
{}

template <typename ...Args>
LogThreadId::LogThreadId(Append a, const std::string &format, Args &&...args)
    : LogThreadId(a, utility::format(format, std::forward<Args>(args)...))
{}

template <typename T>
LogThreadId::LogThreadId(const T &value)
    : LogThreadId(boost::lexical_cast<std::string>(value))
{}

template <typename T>
LogThreadId::LogThreadId(Append a, const T &value)
    : LogThreadId(a, boost::lexical_cast<std::string>(value))
{}

inline LogThreadId::~LogThreadId() {
    try {
        dbglog::thread_id(saved_);
    } catch (...) {}
}

} // namespace utility

#endif // utility_logging_hpp_included_
