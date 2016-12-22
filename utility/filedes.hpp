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
