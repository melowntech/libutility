/**
 * @file utility/align.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_align_hpp_included_
#define utility_align_hpp_included_

#include <cstdlib>

namespace utility {

template <typename T> T align(T offset, std::size_t alignment);


// inlines

template <typename T>
inline T align(T offset, std::size_t alignment)
{
    return (offset + T(alignment - 1)) & T(~(alignment - 1));
}

} // namespace utility

#endif // utility_align_hpp_included_
