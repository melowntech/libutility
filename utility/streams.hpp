/**
 * @file detail/streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_streams_hpp_included_
#define utility_streams_hpp_included_

#include <string>

#include "detail/streams.hpp"

/** This module adds support for C++ iostream.
 */
namespace utility {

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT>
inline detail::Expect<CharT> expect(CharT c) {
    return {c};
}

template<typename CharT>
inline detail::Match<CharT> match(CharT c) {
    return {c};
}

template<typename T, std::size_t size>
detail::ArrayPrinter<T, size>
arrayPrinter(const T(&data)[size], const std::string &separator = ", ")
{
    return {data, separator};
}

template<typename T, std::size_t size>
detail::ArrayPrinter<T, size>
arrayPrinter(const std::array<T, size> &array
        , const std::string &separator = ", ")
{
    return {&array[0], separator};
}

template <typename Dumpable>
detail::Dumper<Dumpable> dump(const Dumpable &dumpable
                              , const std::string &prefix = "")
{
    return {dumpable, prefix};
}

struct LManip {
    typedef std::function<void (std::ostream&)> type;
    const type &l;
    LManip(const type &l) : l(l) {}
};

inline std::ostream& operator<<(std::ostream &os, const LManip &l)
{
    l.l(os);
    return os;
}

} // namespace utility

#endif // utility_streams_hpp_included_
