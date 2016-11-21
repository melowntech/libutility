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

#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace utility {

std::string urlEncode(const std::string &in, bool plus = true);

std::string urlDecode(const std::string &in);

struct InvalidUri : public std::runtime_error {
    InvalidUri(const std::string &message) : std::runtime_error(message) {}
};

struct InvalidEncoding : public std::runtime_error {
    InvalidEncoding(const std::string &message)
        : std::runtime_error(message) {}
};

struct UriNetloc {
    std::string user;
    std::string password;
    std::string host;
    int port;

    UriNetloc() : port(-1) {}

    bool validNetloc() const { return !host.empty(); }
};

struct UriComponents : UriNetloc {
    std::string scheme;
    std::string path;
    std::string search;
    std::string fragment;

    UriComponents() {}
};

class Uri {
public:
    Uri() {}
    Uri(const std::string &in);
    Uri(UriComponents components) : components_(std::move(components)) {}

    const UriComponents& components() const { return components_; }

    const std::string& scheme() const;
    void scheme(std::string value) { components_.scheme = std::move(value); }
    const std::string& host() const;
    void host(std::string value) { components_.host = std::move(value); }
    int port() const;
    boost::filesystem::path path() const;

    /** Returns slice of path starting at given index.
     *
     * \param index start index
     * \param absolutize make path absolute (starting with /) if true
     */
    boost::filesystem::path path(std::size_t index
                                 , bool absolutize = false) const;

    /** Returns path component at given index.
     *  Returns empty string if index is out of bounds.
     */
    std::string pathComponent(std::size_t index) const;

    /** Returns number of path components.
     */
    std::size_t pathComponentCount() const;

    bool absolutePath() const;

    bool absolute() const;

    /** Reconstructs URI in string representation.
     */
    std::string str() const;

    /** Resolves URI reference.
     *  returns this + relative
     */
    Uri resolve(const Uri &relative) const;

    /** Removes .. and . segments from str.
     */
    static std::string removeDotSegments(const std::string &str);

private:
    UriComponents components_;
};

/** Reconstructs URI in string representation.
 */
inline std::string str(const Uri &uri) { return uri.str(); }

// inlines

inline const std::string& Uri::scheme() const { return components_.scheme; }
inline const std::string& Uri::host() const { return components_.host; }
inline int Uri::port() const { return components_.port; }

inline Uri operator+(const Uri &base, const Uri &relative) {
    return base.resolve(relative);
}

inline bool Uri::absolute() const { return !host().empty(); }
inline boost::filesystem::path Uri::path() const { return components_.path; }

} // namespace utility

#endif // utility_uri_hpp_included_
