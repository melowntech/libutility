/**
 * @file detail/streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_detail_streams_hpp_included_
#define utility_detail_streams_hpp_included_

#include <cctype>
#include <cstddef>
#include <array>
#include <istream>
#include <ostream>

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

template<typename T, std::size_t size>
struct ArrayPrinter {
    const T *data;
    ArrayPrinter(const T *data, const std::string &separator)
        : data(data), separator(separator)
    {}

    friend std::ostream&
    operator<<(std::ostream &os, const ArrayPrinter<T, size> &a)
    {
        bool first(true);
        std::for_each(a.data, a.data + size
                      , [&](const T &d) -> void {
                          if (first) { first = false; }
                          else { os << a.separator; }
                          os << d;
                      });

        return os;
    }

    const std::string separator;
};

} } // namespace utility::detail

#endif // utility_detail_streams_hpp_included_
