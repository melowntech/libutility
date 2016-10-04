#include <cctype>
#include <iterator>
#include <boost/filesystem.hpp>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "dbglog/dbglog.hpp"

#include "./uri.hpp"

namespace fs = boost::filesystem;
namespace ba = boost::algorithm;

namespace utility {

namespace {

const char *alphabet("0123456789abcdef");

char hexAsNumber(char value)
{
    if ((value >= '0') && (value <= '9')) {
        return value - '0';
    }

    if ((value >= 'a') && (value <= 'f')) {
        return 10 + value - 'a';
    }

    if ((value >= 'A') && (value <= 'F')) {
        return 10 + value - 'A';
    }

    LOGTHROW(err1, InvalidEncoding)
        << "Invalid URL encoding (" << value << " is not a hex character).";
    throw;
}

} // namespace

std::string urlEncode(const std::string &in, bool plus)
{
    std::string out;
    for (char c : in) {
        if (std::isalnum(c)) {
            out.push_back(c);
        } else if (plus && (c == ' ')) {
            out.push_back('+');
        } else {
            out.push_back('%');
            out.push_back(alphabet[(c >> 4) & 0x0f]);
            out.push_back(alphabet[c & 0x0f]);
        }
    }
    return out;
}

std::string urlDecode(const std::string &in)
{
    std::string out;
    out.reserve(in.size());

    const char* i(in.data());
    const char* e(i + in.size());
    while (i != e) {
        auto c(*i++);
        if (c != '%') {
            // raw character
            out.push_back(c);
            continue;
        }

        // encoded character
        if (i == e) {
            LOGTHROW(err1, InvalidEncoding)
                << "Invalid URL encoding (no character after % sign).";
        }

        // first byte
        c = *i++;
        if (i == e) {
            LOGTHROW(err1, InvalidEncoding)
                << "Invalid URL encoding (only one character after % sign).";
        }
        char tmp(hexAsNumber(c) << 4);

        // second byte
        c = *i++;
        tmp += hexAsNumber(c);
        out.push_back(tmp);
    }

    return out;
}

namespace detail {

inline bool isSchemeChar(char c)
{
    switch (c) {
    case '+': case '-': case '.': return true;
    }
    return std::isalnum(c);
}

typedef std::pair<const char*, const char*> Range;

Range range(const std::string &str, std::string::size_type start = 0
            , std::string::size_type end = std::string::npos)
{
    const auto size(str.size());
    if (start > size) { start = size; }
    if (end > size) { end = size; }

    if (start > end) { std::swap(start, end); }
    const char* data(str.data());
    return std::pair<const char*, const char*>(data + start, data + end);
}

inline bool empty(const Range &range) {
    return range.first == range.second;
}

inline std::string::size_type size(const Range &range) {
    return range.second - range.first;
}

std::ostream& operator<<(std::ostream &os, Range range)
{
    while (range.first < range.second) {
        os << *range.first++;
    }
    return os;
}

const char* find(const Range &r, char c)
{
    const char *pos(r.first);
    for (; pos < r.second; ++pos) {
        if (c == *pos) { return pos; }
    }
    return pos;
}

void parseHost(UriNetloc &nl, const std::string &in, const Range &r)
{
    auto colon(find(r, ':'));
    if (colon == r.second) {
        nl.host.assign(r.first, r.second);
        return;
    }

    nl.host.assign(r.first, colon);

    std::string port(colon + 1, r.second);

    try {
        nl.port = boost::lexical_cast<int>(port);
    } catch (const boost::bad_lexical_cast&) {
        LOGTHROW(err1, InvalidUri)
            << "<" << in << ">: empty port <" << port << ">.";
    }
}

std::string::size_type parseNetloc(UriNetloc &nl, const std::string &in
                                   , std::string::size_type pos)
{
    auto delim(in.find_first_of("/?#", pos));
    if (delim == std::string::npos) {
        return delim;
    }

    if (delim != pos) {
        auto netloc(range(in, pos, delim));

        auto atsign(find(netloc, '@'));
        if (atsign == netloc.second) {
            // no atsign, just host
            parseHost(nl, in, netloc);
            return delim;
        }

        // set host
        parseHost(nl, in, Range(atsign + 1, netloc.second));

        // try to split user:password
        Range up(netloc.first, atsign);
        auto colon(find(up, ':'));
        nl.user.assign(up.first, colon);

        if (colon == up.second) {
            // just username
            return delim;
        }

        // usename and password
        nl.password.assign(colon + 1, up.second);
    }
    return delim;
}

void parseFromFragment(UriComponents &uri, const std::string &in
                       , std::string::size_type pos)
{
    uri.fragment.assign(in, pos, in.size() - pos);
}

void parseFromSearch(UriComponents &uri, const std::string &in
                     , std::string::size_type pos)
{
    auto delim(in.find('#', pos));
    if (delim == std::string::npos) {
        // just search
        uri.search.assign(in, pos, in.size() - pos);
        return;
    }

    // search + something
    uri.search.assign(in, pos, delim - pos);
    parseFromFragment(uri, in, delim + 1);
}

void parseFromPath(UriComponents &uri, const std::string &in
                   , std::string::size_type pos)
{
    auto delim(in.find_first_of("?#", pos));
    if (delim == std::string::npos) {
        // just path
        uri.path.assign(in, pos, in.size() - pos);
        return;
    }

    // path + something
    uri.path.assign(in, pos, delim - pos);

    if (in[delim] == '?') {
        parseFromSearch(uri, in, delim + 1);
        return;
    }

    parseFromFragment(uri, in, delim + 1);
}

void parseAfterScheme(UriComponents &uri, const std::string &in
                      , std::string::size_type pos = 0)
{
    if (ba::starts_with(range(in, pos), "//")) {
        // netloc
        pos = parseNetloc(uri, in, pos + 2);

        if (pos == std::string::npos) { return; }
    }

    switch (in[pos]) {
    case '/':
        parseFromPath(uri, in, pos);
        break;

    case '?':
        parseFromSearch(uri, in, pos + 1);
        break;

    case '#':
        parseFromFragment(uri, in, pos + 1);
        break;

    default:
        // anything else -> path
        parseFromPath(uri, in, pos);
        break;
    }
}

} // namespace detail

//--- Public Interface --------------------------------------------------------
Uri::Uri(const std::string &in)
{
   auto colon(in.find(':'));
    if (!colon) {
        LOGTHROW(err1, InvalidUri)
            << "<" << in << ">: empty schema.";
    }
    if (colon == std::string::npos) {
        detail::parseAfterScheme(components_, in);
        return;
    }

    for (std::string::size_type i(0); i < colon; ++i) {
        if (!detail::isSchemeChar(in[i])) {
            detail::parseAfterScheme(components_, in);
            return;
        }
    }

    ba::to_lower_copy
        (std::back_inserter(components_.scheme), detail::range(in, 0, colon));

    detail::parseAfterScheme(components_, in, colon + 1);
}

Uri parseUri(const std::string &in) {
    return Uri(in);
}

std::string Uri::str() const
{
    std::ostringstream os;

    // schema
    if (!components_.scheme.empty()) {
        os << components_.scheme << ':';
    }

    // netloc
    if (!components_.host.empty()) {
        os << "//";
        if (!components_.user.empty()) {
            os << components_.user;
            if (!components_.password.empty()) {
                os << ':' << components_.password;
            }
            os << '@';
        }

        os << components_.host;
        if (components_.port >= 0) {
            os << ':' << components_.port;
        }
    }

    // path
    if (!components_.path.empty()) {
        os << components_.path;
    }

    // search
    if (!components_.search.empty()) {
        os << '?' << components_.search;
    }

    // fragment
    if (!components_.fragment.empty()) {
        os << '#' << components_.fragment;
    }

    return os.str();
}

std::string Uri::removeDotSegments(const std::string &str)
{
    typedef detail::Range Range;
    typedef std::vector<Range> Ranges;
    auto in(detail::range(str));
    Ranges out;
    const std::string slash("/");

    /** RFC 3986, 5.2.4
        2. While the input buffer is not empty, loop as follows:
     */
    while (!detail::empty(in)) {
        auto is(detail::size(in));

        /** A.  If the input buffer begins with a prefix of "../" or "./",
            then remove that prefix from the input buffer; otherwise,
         */
        if (ba::starts_with(in, "../")) {
            in.first += 3;
            continue;
        }
        if (ba::starts_with(in, "./")) {
            in.first += 2;
            continue;
        }

        /** B.  if the input buffer begins with a prefix of "/./" or "/.",
            where "." is a complete path segment, then replace that
           prefix with "/" in the input buffer; otherwise,
        */
        if (ba::starts_with(in, "/.")
            && ((is == 2) || (*(in.first + 2) == '/')))
        {
            // remove /.
            in.first += 2;
            if (detail::empty(in)) { out.push_back(detail::range(slash)); }
            continue;
        }

        /** C.  if the input buffer begins with a prefix of "/../" or "/..",
            where ".." is a complete path segment, then replace that
            prefix with "/" in the input buffer and remove the last
            segment and its preceding "/" (if any) from the output
            buffer; otherwise,
        */
        if (ba::starts_with(in, "/..")
            && ((is == 3) || (*(in.first + 3) == '/')))
        {
            // remove /..
            in.first += 3;
            // remove last element from output
            out.pop_back();

            if (detail::empty(in)) { out.push_back(detail::range(slash)); }
            continue;
        }

        /** D.  if the input buffer consists only of "." or "..", then remove
            that from the input buffer; otherwise,
         */
        if (ba::equals(in, "..") || ba::equals(in, ".")) {
            break;
        }

        /** E.  move the first path segment in the input buffer to the end of
            the output buffer, including the initial "/" character (if
            any) and any subsequent characters up to, but not including,
           the next "/" character or the end of the input buffer.
        */
        // move end after initial slash (if present)
        const char *end((*in.first == '/') ? in.first + 1 : in.first);
        // find second slash
        for (; end < in.second; ++end) {
            if ('/' == *end) { break; }
        }

        out.push_back(Range(in.first, end));
        in.first = end;
    }

    /** RFC 3986, 5.2.4:
        3.  Finally, the output buffer is returned as the result of
        remove_dot_segments.
    */
    std::string ostr;
    for (const auto &r : out) {
        ostr.insert(ostr.end(), r.first, r.second);
    }
    return ostr;
}

namespace detail {

typedef std::vector<boost::iterator_range<std::string::const_iterator>> Tokens;

void join(std::string &out, const std::string &relative) {
    if (out.empty() || ba::starts_with(relative, "/")) {
        // relative is absolute path or out is empty: use relative as is
        out = relative;
        return;
    }

    // NB: out is not empty here
    if (out.back() == '/') {
        // out ends with slash -> directory
        Uri::removeDotSegments(out + relative);
        return;
    }

    // not a directory, cut last component and join
    auto prev(out.rfind('/'));
    if (prev == std::string::npos) {
        // no slash at all, replace
        Uri::removeDotSegments(relative);
        return;
    }

    out.resize(prev + 1);
    out = Uri::removeDotSegments(out + relative);
}

UriComponents resolveUri(const UriComponents &base, const UriComponents &uri)
{
    // uri with scheme -> return uri
    if (!uri.scheme.empty()) { return uri; }

    if (uri.validNetloc()) {
        // uri has netloc
        if (base.scheme.empty()) {
            // scheme-less base, just return
            return uri;
        }

        // copy + scheme from base
        auto copy(uri);
        copy.scheme = base.scheme;
        return copy;
    }

    auto out(base);

    if (!uri.path.empty()) {
        join(out.path, uri.path);

        // clone search and fragment
        out.search = uri.search;
        out.fragment = uri.fragment;
        return out;
    }

    if (!uri.search.empty()) {
        // clone search and fragment
        out.search = uri.search;
        out.fragment = uri.fragment;
        return out;
    }

    if (!uri.fragment.empty()) {
        // clone fragment
        out.fragment = uri.fragment;
        return out;
    }

    // nothing to do
    return out;
}

} // namespace detail

Uri Uri::resolve(const Uri &relative) const
{
    return { detail::resolveUri(components_, relative.components_) };
}

bool Uri::absolutePath() const
{
    return ba::starts_with(components_.path, "/");
}

fs::path Uri::path(std::size_t index, bool absolutize) const
{
    detail::Tokens tokens;
    ba::split(tokens, components_.path, ba::is_any_of("/")
              , ba::token_compress_on);
    if (absolutePath()) { ++index; }
    if (index >= tokens.size()) { return {}; }

    fs::path out;
    if (absolutize) { out /= "/"; }

    for (auto itokens(tokens.begin() + index), etokens(tokens.end());
         itokens != etokens; ++itokens)
    {
        out /= fs::path(std::begin(*itokens), std::end(*itokens));
    }
    return out;
}

std::string Uri::pathComponent(std::size_t index) const
{
    detail::Tokens tokens;
    ba::split(tokens, components_.path, ba::is_any_of("/")
              , ba::token_compress_on);
    if (absolutePath()) { ++index; }
    if (index >= tokens.size()) { return {}; }
    const auto &token(tokens[index]);
    return std::string(std::begin(token), std::end(token));
}

} // utility
