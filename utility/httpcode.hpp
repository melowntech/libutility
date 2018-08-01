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

namespace utility {

enum class HttpCode {
    unset = 0
    , OK = 200

    , MultipleChoices = 300
    , MovedPermanenty = 301
    , Found = 302
    , SeeOther = 303
    , NotModified = 304
    , UseProxy = 305
    , SwitchProxy = 306
    , TemporaryRedirect = 307
    , PermanentRedrect = 308

    , BadRequest = 400
    , NotAuthorized = 401
    , Forbidden = 403
    , NotFound = 404
    , NotAllowed = 405
    , UnprocessableEntity = 422

    , InternalServerError = 500
    , NotImplemented = 501
    , BadGateway = 502
    , ServiceUnavailable = 503
    , GatewayTimeout = 504

    // synthetic codes, not to be sent to the client
    , RequestAborted = 499
};

} // namespace utility

namespace std {

/** Specialization of is-error-code-enum
 */
template<>
struct is_error_code_enum< ::utility::HttpCode> : public true_type {};

} // namespace std

// now, when we have specialized above template so we can pull all stuff here

#include <exception>

namespace utility {

/** Statically-allocated error-category
 */
const std::error_category& httpCodeCategory();

inline std::error_code make_error_code(HttpCode code) {
    return std::error_code(static_cast<int>(code), httpCodeCategory());
}

inline std::error_code make_http_error_code(int code) {
    return std::error_code(code, httpCodeCategory());
}

inline std::error_condition make_error_condition(HttpCode code) {
    return std::error_condition(static_cast<int>(code), httpCodeCategory());
}

inline std::error_condition make_http_error_condition(int code) {
    return std::error_condition(code, httpCodeCategory());
}

class HttpError : public std::runtime_error {
public:
    explicit HttpError(const std::error_code &code);

    HttpError(const std::error_code &code, const std::string &message);

    HttpError(HttpCode code, const std::string &message);

    const std::error_code& code() const noexcept  { return code_; }

private:
    std::error_code code_;
};

template <HttpCode Code>
struct HttpErrorWithCode : HttpError {
    explicit HttpErrorWithCode(const std::string &message)
        : HttpError(Code, message)
    {}
};

} // namespace utility

#endif // utility_httpcode_hpp_included_
