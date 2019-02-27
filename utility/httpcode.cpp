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

#include "httpcode.hpp"

#include "format.hpp"

namespace utility {

namespace {
struct HttpErrorCategory : public std::error_category {
    virtual const char* name() const noexcept { return "httpCode"; }

    virtual std::string message(int c) const {
        switch (HttpCode(c)) {
        case HttpCode::OK: return "OK";

        case HttpCode::MultipleChoices: return "Multiple Choices";
        case HttpCode::MovedPermanenty: return "Moved Permanenty";
        case HttpCode::Found: return "Found";
        case HttpCode::SeeOther: return "See Other";
        case HttpCode::NotModified: return "Not Modified";
        case HttpCode::UseProxy: return "Use Proxy";
        case HttpCode::SwitchProxy: return "Switch Proxy";
        case HttpCode::TemporaryRedirect: return "Temporary Redirect";
        case HttpCode::PermanentRedrect: return "Permanent Redrect";

        case HttpCode::BadRequest: return "Bad Request";
        case HttpCode::NotAuthorized: return "Not Authorized";
        case HttpCode::Forbidden: return "Forbidden";
        case HttpCode::NotFound: return "Not Found";
        case HttpCode::NotAllowed: return "Not Allowed";
        case HttpCode::UnprocessableEntity: return "Unprocessable Entity";

        case HttpCode::InternalServerError: return "Internal Server Error";
        case HttpCode::NotImplemented: return "Not Implemented";
        case HttpCode::BadGateway: return "Bad Gateway";
        case HttpCode::ServiceUnavailable: return "Service Unavailable";
        case HttpCode::GatewayTimeout: return "Gateway Timeout";

        default: break;
        }
        return format("HTTP status=%d", c);
    }
};

HttpErrorCategory httpErrorCategoryInstance;

} // namespace

HttpError::HttpError(const std::error_code &code)
    : std::runtime_error(format("HTTP error <%s>", code.message()))
    , code_(code)
{}

HttpError::HttpError(const std::error_code &code, const std::string &message)
    : std::runtime_error(message), code_(code)
{}

HttpError::HttpError(HttpCode code, const std::string &message)
    : std::runtime_error(message), code_(make_error_code(code))
{}

const std::error_category& httpCodeCategory()
{
    return httpErrorCategoryInstance;
}

} // namespace utility
