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

#ifndef utility_interprocess_hpp_included_
#define utility_interprocess_hpp_included_

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace utility {

class UniqueInterprocessLock {
public:
    UniqueInterprocessLock(boost::interprocess::interprocess_mutex &mutex)
        : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~UniqueInterprocessLock() { mutex_.unlock(); }

private:
    boost::interprocess::interprocess_mutex &mutex_;
};

namespace shm {

/** Simple shared memory allocator.
 */
class Allocator : boost::noncopyable {
public:
    Allocator(std::size_t size)
        : size_(size + sizeof(boost::interprocess::interprocess_mutex))
        , offset_()
        , mem_(boost::interprocess::anonymous_shared_memory(size_))
        , mutex_(*allocate_unlocked<boost::interprocess::interprocess_mutex>())
    {}

    template <typename T>
    T* allocate(std::size_t count = 1) {
        boost::interprocess::scoped_lock
            <boost::interprocess::interprocess_mutex> guard(mutex_);
        return allocate_unlocked<T>(count);
    }

private:
    template <typename T>
    T* allocate_unlocked(std::size_t count = 1) {
        // TODO: check size
        auto data(static_cast<char*>(mem_.get_address()) + offset_);
        offset_ += sizeof(T) * count;
        return reinterpret_cast<T*>(data);
    }

    std::size_t size_;
    std::size_t offset_;
    boost::interprocess::mapped_region mem_;
    boost::interprocess::interprocess_mutex &mutex_;
};

} // shm

} // namespace utility

#endif // utility_interprocess_hpp_included_
