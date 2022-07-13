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

#include "stringview.hpp"

namespace utility {

std::string urlEncode(const std::string &in, bool plus = true);

std::string urlDecode(const std::string &in);

std::string urlDecode(std::string::const_iterator begin
                      , std::string::const_iterator end);

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
    Uri& scheme(std::string value);
    const std::string& host() const;
    Uri& host(std::string value);
    int port() const;
    boost::filesystem::path path() const;
    const std::string& user() const;
    const std::string& password() const;

    /** Drops authentization info (username and password) from URI.
     */
    Uri& dropAuthInfo(bool justPassword = false);

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

    static std::string joinAndRemoveDotSegments(std::string a
                                                , const std::string &b);

private:
    UriComponents components_;
};


std::string str(const Uri &uri);

// query parsing

struct QueryKeyValue {
    StringView key;
    StringView value;

    typedef std::vector<QueryKeyValue> list;

    QueryKeyValue() = default;
    QueryKeyValue(StringView key) : key(std::move(key)) {}
    QueryKeyValue(StringView key, StringView value)
        : key(std::move(key)), value(std::move(value))
    {}

    /** Splits single argument into key/value pair
     */
    static QueryKeyValue split(const StringView &arg);

    /** Splits query to key/value list.
     *
     *  NB: elements are held as iterators to original string; do not use
     *  key/value list after string destruction.
     *
     *  NB: elements are left intact, URL encoding is NOT removed.
     */
    static QueryKeyValue::list splitQuery(const std::string &query);
};

/** Wrapper around parsed query string. Holds string views to original string,
 *  do not use after strings destruction.
 */
class QueryString {
public:
    QueryString(const std::string &query)
        : query_(query), kvl_(QueryKeyValue::splitQuery(query_))
    {
        unescape();
    }

    typedef QueryKeyValue::list::const_iterator iterator;
    typedef iterator const_iterator;
    const_iterator begin() const { return kvl_.begin(); }
    const_iterator end() const { return kvl_.end(); }

    std::string get(const std::string &key
                    , const std::string &defaultValue) const;

private:
    void unescape();
    void unescape(StringView &what);

    const std::string query_;
    QueryKeyValue::list kvl_;
    std::vector<std::string> storage_;
};

// inlines

/** Reconstructs URI in string representation.
 */
inline std::string str(const Uri &uri) { return uri.str(); }

inline const std::string& Uri::scheme() const { return components_.scheme; }
inline Uri& Uri::scheme(std::string value) {
    components_.scheme = std::move(value);
    return *this;
}

inline const std::string& Uri::host() const { return components_.host; }
inline Uri& Uri::host(std::string value) {
    components_.host = std::move(value);
    return *this;
}

inline int Uri::port() const { return components_.port; }

inline const std::string& Uri::user() const { return components_.user; }
inline const std::string& Uri::password() const {
    return components_.password;
}

inline Uri& Uri::dropAuthInfo(bool justPassword) {
    if (!justPassword) { components_.user.clear(); }
    components_.password.clear();
    return *this;
}

inline Uri operator+(const Uri &base, const Uri &relative) {
    return base.resolve(relative);
}

inline bool Uri::absolute() const { return !host().empty(); }
inline boost::filesystem::path Uri::path() const { return components_.path; }


inline std::string urlDecode(const std::string &in)
{
    return urlDecode(in.begin(), in.end());
}


} // namespace utility

#endif // utility_uri_hpp_included_
