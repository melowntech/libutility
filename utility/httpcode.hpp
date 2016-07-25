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
    , NotFound = 404
    , NotAllowed = 405

    , InternalServerError = 500
    , ServiceUnavailable = 503
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
