/**
 * @file shmmutex.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Mutex in shared memory.
 */

#ifndef utility_shmutex_hpp_included_
#define utility_shmutex_hpp_included_

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>

namespace utility {

class ShmMutex {
public:
    typedef boost::interprocess::interprocess_mutex LockType;
    typedef boost::interprocess::scoped_lock<LockType> ScopedLock;

    ShmMutex()
        : mem_(boost::interprocess::anonymous_shared_memory(sizeof(LockType)))
        , lock_(* new (mem_.get_address()) LockType())
    {}

    operator LockType&() { return lock_; }

private:
    boost::interprocess::mapped_region mem_;
    LockType &lock_;
};

} // namespace utility

#endif // utility_shmutex_hpp_included_
