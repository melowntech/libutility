/**
 * @file detail/streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_streams_hpp_included_
#define utility_streams_hpp_included_

#include "detail/streams.hpp"

/** This module adds support for C++ iostream.
 */
namespace utility {

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT>
inline detail::Expect<CharT> expect(CharT c) {
    return detail::Expect<CharT>{c};
}

} // namespace utility

#endif // utility_streams_hpp_included_
