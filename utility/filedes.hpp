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
#ifndef utility_filesed_hpp_included_
#define utility_filesed_hpp_included_

#include <utility>
#include <boost/filesystem/path.hpp>

namespace utility {

/** Simple wrapper around file descriptor.
 */
class Filedes {
public:
    Filedes() : fd_(-1) {}
    explicit Filedes(int fd) : fd_(fd) {}

    Filedes(int fd, const boost::filesystem::path &path)
        : fd_(fd), path_(path)
    {}

    Filedes(Filedes &&other)
        : fd_(other.fd_), path_(std::move(other.path_))
    {
        other.fd_ = -1;
    }

    Filedes& operator=(Filedes &&other) {
        std::swap(fd_, other.fd_);
        path_ = std::move(other.path_);
        return *this;
    }

    ~Filedes();

    int get() const { return fd_; }
    const boost::filesystem::path& path() const { return path_; }

    operator int() const { return get(); }
    operator bool() const { return fd_ >= 0; }

    int release() {
        auto fd(fd_);
        fd_ = -1;
        path_.clear();
        return fd;
    }

    /** Closes underlying file descriptor and invalidates this object.
     */
    void close();

    void closeOnExec(bool value);

    /** Duplicates file descriptor (via dup).;
     */
    Filedes dup() const;

    /** Check fd validity.
     */
    bool valid() const;

    Filedes(const Filedes&) = delete;
    Filedes& operator=(const Filedes&) = delete;

private:
    int fd_;
    boost::filesystem::path path_;
};

} // namespace utility

#endif // utility_filesed_hpp_included_
