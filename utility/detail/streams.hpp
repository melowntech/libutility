/**
 * @file detail/streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_detail_streams_hpp_included_
#define utility_detail_streams_hpp_included_

#include <cctype>
#include <istream>

namespace utility { namespace detail {

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT> struct Expect { CharT c; };

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, const Expect<CharT> &ce)
{
    bool skipws(is.flags() & std::ios::skipws);
    for (;;) {
        auto c(is.get());
        if (c == ce.c) {
            // matched
            return is;
        }

        if (skipws && std::isspace(c)) {
            // skip ws
            continue;
        }
        // failed
        is.setstate(std::ios::failbit);

        break;
    }
    return is;
}

} } // namespace utility::detail

#endif // utility_detail_streams_hpp_included_
