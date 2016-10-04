/** Generic HTTP code-related stuff
 */

#include "./httpcode.hpp"

namespace utility {

namespace {
struct HttpErrorCategory : public std::error_category {
    virtual const char* name() const noexcept { return "httpCode"; }

    virtual std::string message(int c) const {
        switch (HttpCode(c)) {
        case HttpCode::OK: return "OK";
        case HttpCode::Found: return "Found";
        case HttpCode::NotModified: return "Not Modified";
        case HttpCode::BadRequest: return "Bad Request";
        case HttpCode::NotAuthorized: return "Not Authorized";
        case HttpCode::Forbidden: return "Forbidden";
        case HttpCode::NotFound: return "Not Found";
        case HttpCode::NotAllowed: return "Not Allowed";
        case HttpCode::InternalServerError: return "Internal Server Error";
        case HttpCode::ServiceUnavailable: return "Service Unavailable";
        default: break;
        }
        return "Unknown";
    }
};

HttpErrorCategory httpErrorCategoryInstance;

} // namespace

const std::error_category& httpCodeCategory()
{
    return httpErrorCategoryInstance;
}

} // namespace utility
