/**
 * @file shmmutex.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Mutex in shared memory. Forward declaration.
 */

#ifndef utility_shmutexfwd_hpp_included_
#define utility_shmutexfwd_hpp_included_

#include <cstddef>

namespace utility {

class ShmMutex;
template <std::size_t Count> class ShmMutexList;

} // namespace utility

#endif // utility_shmutexfwd_hpp_included_
