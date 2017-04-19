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
#include <type_traits>

#include <magic.h>

#include "dbglog/dbglog.hpp"

#include "magic.hpp"

namespace utility {

typedef std::remove_pointer< ::magic_t>::type MagicCookie;

Magic::Magic(bool followSymlinks)
    : magic_(magic_open(MAGIC_MIME_TYPE
                        | (followSymlinks ? MAGIC_SYMLINK : 0))
             , [] (magic_t m) {
                 if (m) ::magic_close(m);
             })
{
    auto cookie(std::static_pointer_cast<MagicCookie>(magic_).get());
    if (::magic_load(cookie, nullptr) == -1) {
        LOGTHROW(err1, MagicError)
            << "Cannot load magic database: <"
            << ::magic_error(cookie) << ">.";
    }
}

std::string Magic::mime(const void *buffer, std::size_t length) const
{
    auto cookie(std::static_pointer_cast<MagicCookie>(magic_).get());
    auto res(magic_buffer(cookie, buffer, length));

    if (!res) {
        LOGTHROW(err1, MagicError)
            << "Cannot determine magic of given buffer: <"
            << ::magic_error(cookie) << ">.";
    }

    return res;
}

std::string Magic::mime(const boost::filesystem::path &path) const
{
    auto cookie(std::static_pointer_cast<MagicCookie>(magic_).get());
    auto res(magic_file(cookie, path.string().c_str()));

    if (!res) {
        LOGTHROW(err1, MagicError)
            << "Cannot determine magic of given buffer: <"
            << ::magic_error(cookie) << ">.";
    }

    return res;
}

} // namespace utility
