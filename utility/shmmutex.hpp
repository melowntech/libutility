/**
 * @file shmmutex.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Mutex in shared memory.
 */

#ifndef utility_shmutex_hpp_included_
#define utility_shmutex_hpp_included_

#include <cstddef>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>

#include "shmmutexfwd.hpp"

namespace utility {

class ShmMutex {
public:
    typedef boost::interprocess::interprocess_mutex LockType;
    typedef boost::interprocess::scoped_lock<LockType> ScopedLock;

    ShmMutex()
        : mem_(boost::interprocess::anonymous_shared_memory(sizeof(LockType)))
        , lock_(new (mem_.get_address()) LockType)
    {}

    operator LockType&() { return *lock_; }

private:
    boost::interprocess::mapped_region mem_;
    LockType *lock_;
};

template <std::size_t Count>
class ShmMutexList {
public:
    ShmMutexList()
        : mem_(boost::interprocess::anonymous_shared_memory
               (sizeof(ShmMutex::LockType) * Count))
        , locks_(static_cast<ShmMutex::LockType*>(mem_.get_address()))
    {
        for (std::size_t i(0); i < Count; ++i) {
            new (static_cast<void*>(locks_ + i)) ShmMutex::LockType;
        }
    }

    template <std::size_t Index>
    ShmMutex::LockType& lock() { return locks_[Index]; }

private:
    boost::interprocess::mapped_region mem_;
    ShmMutex::LockType *locks_;
};

} // namespace utility

#endif // utility_shmutex_hpp_included_
