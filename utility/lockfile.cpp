#include <new>

#include <sys/stat.h>
#include <unistd.h>

#include <pthread.h>

#include "lockfile.hpp"

namespace utility {

LockFiles::pointer lockFiles;

void LockFiles::fork_prepare()
{
    // lock mapLock before fork
    mapLock_.lock();
}

void LockFiles::fork_parent()
{
    // unlock it after fork
    mapLock_.unlock();
}

void LockFiles::fork_child()
{
    // we need to re-initialize the mutex (ugly as hell, I DO know...)
    new ((void*) &mapLock_) std::mutex();

    // destroy all files
    map_.clear();
}

struct LockFilesInitializer {
    static LockFilesInitializer instance;
    LockFilesInitializer();

    void fork_prepare() { lockFiles->fork_prepare(); }

    void fork_parent() { lockFiles->fork_parent(); }

    void fork_child() { lockFiles->fork_child(); }
};
LockFilesInitializer instance;

extern "C" {

void fork_prepare()
{
    LockFilesInitializer::instance.fork_prepare();
}

void fork_parent()
{
    LockFilesInitializer::instance.fork_parent();
}

void fork_child()
{
    LockFilesInitializer::instance.fork_child();
}

LockFilesInitializer::LockFilesInitializer() {
    utility::lockFiles.reset(new LockFiles);
    ::pthread_atfork(&utility::fork_prepare, &utility::fork_parent
                     , &utility::fork_child);
}

} // extern C

class LockFiles::Lock::Internals {
public:
    Internals(const boost::filesystem::path &path
              , ino_t inode, int fd
              , const LockFiles::wpointer &locker)
        : path_(path), inode_(inode), fd_(fd), locker_(locker)
    {}

    ~Internals() {
        auto locker(locker_.lock());
        if (locker) {
            locker->destroy(inode_);
        }

        if (-1 == ::close(fd_)) {
            std::system_error e(errno, std::system_category());
            LOG(err3) << "Cannot close lock " << path_ << " file: <"
                      << e.code() << ", " << e.what() << ">.";
        }
    }

    void lock();
    void unlock();

private:
    boost::filesystem::path path_;
    ino_t inode_;
    int fd_;
    LockFiles::wpointer locker_;
};

LockFiles::Lock::Lock(const std::shared_ptr<Lock::Internals> &lock)
    : lock_(lock)
{}

void LockFiles::Lock::lock() { lock_->lock(); }
void LockFiles::Lock::unlock() { lock_->unlock(); }

// implement TEMP_FAILURE_RETRY if not present on platform (via C++11 lambda)
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(operation) [&]()->int {       \
        for (;;) { int e(operation);                     \
            if ((-1 == e) && (EINTR == errno)) continue; \
            return e;                                    \
        }                                                \
    }()
#endif

void LockFiles::Lock::Internals::lock()
{
    LOG(debug) << "Locking " << path_ << ".";

    struct ::flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    // TODO: check for EINTR

    auto res(TEMP_FAILURE_RETRY(::fcntl(fd_, F_SETLK, &lock)));
    if (-1 == res) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot lock file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
}

void LockFiles::Lock::Internals::unlock()
{
    LOG(debug) << "Unlocking " << path_ << ".";

    struct ::flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    auto res(TEMP_FAILURE_RETRY(::fcntl(fd_, F_SETLK, &lock)));
    if (-1 == res) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot unlock file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }
}

LockFiles::Lock LockFiles::create(const boost::filesystem::path &path)
{
    auto self(shared_from_this());

    // open file and determine its inode
    auto fd(::open(path.string().c_str(), O_RDWR));
    if (-1 == fd) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot open lock file " << path << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    struct stat buf;
    if (-1 == ::fstat(fd, &buf)) {
        std::system_error e(errno, std::system_category());
        LOG(err3) << "Cannot stat lock file " << path << ": <"
                  << e.code() << ", " << e.what() << ">.";
        ::close(fd);
        throw e;
    }

    // lock access to map
    std::unique_lock<std::mutex> guard(mapLock_);

    auto inode(buf.st_ino);
    auto fmap(map_.find(inode));
    if (fmap != map_.end()) {
        // already in lock map
        auto l(fmap->second.lock());
        if (l) { return Lock(l); }
        // stale entry -> remove
        map_.erase(fmap);
    }

    // new entry
    auto lock(std::make_shared<Lock::Internals>(path, inode, fd, self));

    map_.insert(std::make_pair(inode, lock));

    return Lock(lock);
}

void LockFiles::destroy(ino_t inode)
{
    // remove inode info from lock map
    std::unique_lock<std::mutex> guard(mapLock_);
    map_.erase(inode);
}

} // namespace utility
