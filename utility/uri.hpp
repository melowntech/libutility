#ifndef utility_uri_hpp_included_
#define utility_uri_hpp_included_

/*
 * Uri parsing
 *
 * Based on code from https://github.com/CovenantEyes/uri-parser (BSD-licenced)
 *
 * Modified by Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * TODO: decode/encode url-encoded characters in path etc
 */

#include <string>
#include <sstream>
#include <cstdlib>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace utility {

struct Uri {
    std::string schema;
    std::string user;
    std::string password;
    std::string host;
    std::string path;
    std::string search;
    int port;

    Uri() : port(-1) {}
    Uri(std::string in);
    std::string join() const;
};

/** Parses URI, effectively calls Uri(in)
 */
Uri parseUri(std::string in);

/** Returns new URI by joinin base with uri.
*/
Uri join(const Uri &base, const Uri &uri);

std::string join(const Uri &uri);

std::string urlEncode(const std::string &in, bool plus = true);

namespace detail {

//--- Helper Functions --------------------------------------------------------
inline std::string tailSlice(std::string &subject, const std::string &delimiter
                             , bool keep_delim = false)
{
    // Chops off the delimiter and everything that follows
    // (destructively) returns everything after the delimiter
    auto delimiterLocation = subject.find(delimiter);

    if (delimiterLocation == std::string::npos) {
        return {};
    }

    auto start(keep_delim ? delimiterLocation
               : delimiterLocation + delimiter.length());
    auto end(subject.length() - start);
    auto output(subject.substr(start, end));
    subject = subject.substr(0, delimiterLocation);

    return output;
}

inline std::string headSlice(std::string &subject
                             , const std::string &delimiter)
{
    // Chops off the delimiter and everything that precedes (destructively)
    // returns everthing before the delimeter
    auto delimiterLocation(subject.find(delimiter));

    if (delimiterLocation == std::string::npos) {
        return {};
    }

    auto output(subject.substr(0, delimiterLocation));
    subject = subject.substr
        (delimiterLocation + delimiter.length()
         , subject.length() - (delimiterLocation + delimiter.length()));
    return output;
}


//--- Extractors -----------------------
inline int extractPort(std::string &hostport) {
    try {
        return boost::lexical_cast<int>(tailSlice(hostport, ":"));
    } catch (const boost::bad_lexical_cast&) {
        return -1;
    }
}

inline std::string extractPath(std::string &in) {
    return tailSlice(in, "/", true);
}

inline std::string extractSchema(std::string &in) {
    return headSlice(in, "://");
}

inline std::string extractSearch(std::string &in) {
    return tailSlice(in, "?");
}

inline std::string extractPassword(std::string &userpass) {
    return tailSlice(userpass, ":");
}

inline std::string extractUserpass(std::string &in) {
    return headSlice(in, "@");
}

} // namespace detail

//--- Public Interface --------------------------------------------------------
inline Uri::Uri(std::string in)
    : port(-1)
{
    schema
        = boost::algorithm::to_lower_copy(detail::extractSchema(in));
    search = detail::extractSearch(in);
    path = detail::extractPath(in);
    std::string userpass = detail::extractUserpass(in);
    password = detail::extractPassword(userpass);
    user = userpass;
    port = detail::extractPort(in);
    host = boost::algorithm::to_lower_copy(in);
}

inline Uri parseUri(const std::string &in) {
    return Uri(in);
}

inline std::string Uri::join() const
{
    std::ostringstream os;

    if (!host.empty()) {
        os << schema << "://";
    }
    if (!user.empty()) {
        os << user;
        if (!password.empty()) {
            os << ':' << password;
        }
        os << '@';
    }
    if (!host.empty()) {
        os << host;
        if (port > 0) {
            os << ':' << port;
        }
    }
    if (!path.empty()) {
        if (path[0] != '/') { os << '/'; };
        os << path;
    }

    if (!search.empty()) {
        os << '?' << search;
    }

    return os.str();
}

inline std::string join(const Uri &uri) { return uri.join(); }

} // namespace utility

#endif // utility_uri_hpp_included_
