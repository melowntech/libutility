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
        uri.path.assign(in, pos, in.size() - pos);
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
    }

    if (pos == std::string::npos) { return; }

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

namespace {

void join(std::string &out, const std::string &relative) {
    LOG(info4) << "out: <" << out << ">";
    LOG(info4) << "relative: <" << relative << ">";
    if (out.empty() || ba::starts_with(relative, "/")) {
        // relative is absolute path or out is empty: use relative as is
        out = relative;
        return;
    }

    // NB: out is not empty here

    if (out.back() == '/') {
        // out ends with slash -> directory
        out += relative;
        return;
    }

    // not a directory, cut last component and join
    auto prev(out.rfind('/'));
    if (prev == std::string::npos) {
        // no slash at all, replace
        out = relative;
        return;
    }

    out.resize(prev + 1);
    out += relative;
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

} // namespace

Uri Uri::resolve(const Uri &relative) const
{
    return { resolveUri(components_, relative.components_) };
}

bool Uri::absolutePath() const
{
    return ba::starts_with(components_.path, "/");
}

namespace {
typedef std::vector<boost::iterator_range<std::string::const_iterator>> Tokens;
} // namespace

fs::path Uri::path(std::size_t index, bool absolutize) const
{
    Tokens tokens;
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
    Tokens tokens;
    ba::split(tokens, components_.path, ba::is_any_of("/")
              , ba::token_compress_on);
    if (absolutePath()) { ++index; }
    if (index >= tokens.size()) { return {}; }
    const auto &token(tokens[index]);
    return std::string(std::begin(token), std::end(token));
}

} // utility
