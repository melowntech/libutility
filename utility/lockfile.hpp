#ifndef utility_lockfile_hpp_included_
#define utility_lockfile_hpp_included_

#include <sys/types.h>

#include <cstdlib>
#include <memory>
#include <map>
#include <system_error>
#include <mutex>

#include <boost/filesystem/path.hpp>

#include "dbglog/dbglog.hpp"

namespace utility {

class LockFiles : public std::enable_shared_from_this<LockFiles> {
public:
    typedef std::shared_ptr<LockFiles> pointer;
    typedef std::weak_ptr<LockFiles> wpointer;

    class ScopedLock;

    class Lock {
    public:
        Lock() {}

    private:
        class Internals;
        friend class LockFiles;
        friend class ScopedLock;

        Lock(const std::shared_ptr<Lock::Internals> &lock);

        void lock();
        void unlock();

        std::shared_ptr<Internals> lock_;
    };

    class ScopedLock {
    public:
        ScopedLock(Lock &lock);
        ~ScopedLock();

    private:
        Lock &lock_;
    };

    Lock create(const boost::filesystem::path &path);

private:
    friend class Lock::Internals;
    friend class LockFilesInitializer;
    LockFiles() {}

    void destroy(ino_t inode);

    void fork_prepare();
    void fork_parent();
    void fork_child();

    typedef std::map<ino_t, std::weak_ptr<Lock::Internals> > map;
    map map_;
    std::mutex mapLock_;
};

extern LockFiles::pointer lockFiles;

inline LockFiles::ScopedLock::ScopedLock(Lock &lock)
    : lock_(lock) { lock_.lock(); }
inline LockFiles::ScopedLock::~ScopedLock() {
    try {
        lock_.unlock();
    } catch (const std::exception &e) {
        LOG(fatal) << "Failed to unlock lock: <" << e.what()
                   << ">! Bailing out.";
        std::abort();
    }
}

} // namespace utility

#endif // utility_lockfile_hpp_included_
