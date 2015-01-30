#ifndef utility_filesed_hpp_included_
#define utility_filesed_hpp_included_

#include <utility>

namespace utility {

class Filedes {
public:
    Filedes() : fd_(-1) {}
    Filedes(int fd) : fd_(fd) {}
    Filedes(Filedes &&other) : fd_(other.fd_) { other.fd_ = -1; }

    Filedes& operator=(Filedes &&other) {
        std::swap(fd_, other.fd_);
        return *this;
    }

    ~Filedes();
    // ~Filedes() { if (fd_ >= 0) { TEMP_FAILURE_RETRY(::close(fd_)); } }

    int get() const { return fd_; }

    operator int() const { return get(); }

    int release() { auto fd(fd_); fd_ = -1; return fd; }

private:
    Filedes(const Filedes&) = delete;
    Filedes& operator=(const Filedes&) = delete;
    int fd_;
};

} // namespace utility

#endif // utility_filesed_hpp_included_
