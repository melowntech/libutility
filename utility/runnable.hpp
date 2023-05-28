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
#ifndef shared_utility_runnable_hpp_included_
#define shared_utility_runnable_hpp_included_

#include <memory>
#include <utility>
#include <boost/noncopyable.hpp>

namespace utility {

class Runnable : boost::noncopyable {
public:
    Runnable() = default;
    // Runnable(Runnable &&) = default; // move ctor deleted in noncopyable
    virtual ~Runnable() {}
    virtual bool isRunning() = 0;
    virtual void stop() = 0;

    inline operator bool() { return isRunning(); }

    class Wrapper;

    /** Creates simple shareable runnable.
     */
    static Wrapper simple();
};

class Runnable::Wrapper : public Runnable {
public:
    Wrapper(std::unique_ptr<Runnable> &&wrapped)
        : wrapped_(std::move(wrapped))
    {}

    Wrapper(Wrapper &&o) : wrapped_(std::move(o.wrapped_)) {}

    bool isRunning() { return wrapped_->isRunning(); }
    void stop() { wrapped_->stop(); }

private:
    std::unique_ptr<Runnable> wrapped_;
};

} // namespace utility

#endif // shared_utility_runnable_hpp_included_
