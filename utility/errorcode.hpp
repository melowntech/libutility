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
/** Generic HTTP code-related stuff
 */

#ifndef utility_errorcode_hpp_included_
#define utility_errorcode_hpp_included_

#include <system_error>
#include <future>

#include "./httpcode.hpp"

namespace utility {

void throwErrorCode(const std::error_code &ec) noexcept(false);

void throwErrorCode(const std::error_code &ec, std::string message)
    noexcept(false);

// inlines

inline void throwErrorCode(const std::error_code &ec) noexcept(false)
{
    const auto &cat(ec.category());
    if (cat == std::system_category()) {
        throw std::system_error(ec);
    }
    if (cat == std::future_category()) {
        throw std::future_error(ec);
    }
    if (cat == httpCodeCategory()) {
        throw HttpError(ec);
    }

    // unknown category, use system error
    throw std::system_error(ec);
}

inline void throwErrorCode(const std::error_code &ec, std::string message)
    noexcept(false)
{
    const auto &cat(ec.category());
    if (cat == std::system_category()) {
        throw std::system_error(ec, std::move(message));
    }
    if (cat == std::future_category()) {
        throw std::future_error(ec);
    }
    if (cat == httpCodeCategory()) {
        throw HttpError(ec, std::move(message));
    }

    // unknown category, use system error
    throw std::system_error(ec, std::move(message));
}

} // namespace std

#endif // utility_errorcode_hpp_included_

