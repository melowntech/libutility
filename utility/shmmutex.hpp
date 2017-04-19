/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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
