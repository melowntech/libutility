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

#ifndef utility_httpcode_hpp_included_
#define utility_httpcode_hpp_included_

#include <system_error>
#include <exception>

#include "./format.hpp"

namespace utility {

enum class HttpCode {
    unset = 0
    , OK = 200

    , Found = 302
    , NotModified = 304

    , BadRequest = 400
    , NotAuthorized = 401
    , Forbidden = 403
    , NotFound = 404
    , NotAllowed = 405

    , InternalServerError = 500
    , ServiceUnavailable = 503

    // synthetic codes, not to be sent to the client
    , RequestAborted = 499
};

/** Statically-allocated error-category
 */
const std::error_category& httpCodeCategory();

inline std::error_code make_error_code(HttpCode code) {
    return std::error_code(static_cast<int>(code), httpCodeCategory());
}

inline std::error_condition make_error_condition(HttpCode code) {
    return std::error_condition(static_cast<int>(code), httpCodeCategory());
}

class HttpError : public std::runtime_error {
public:
    explicit HttpError(std::error_code code)
        : std::runtime_error(format("HTTP error <%s>", code.message()))
        , code_(code)
    {}
    HttpError(std::error_code code, const std::string &message)
        : std::runtime_error(message), code_(code)
    { }

    const std::error_code& code() const noexcept  { return code_; }

private:
    std::error_code code_;
};

} // namespace utility

namespace std {

/** Specialization.
 */
template<>
struct is_error_code_enum< ::utility::HttpCode> : public true_type {};

} // namespace std

#endif // utility_httpcode_hpp_included_
