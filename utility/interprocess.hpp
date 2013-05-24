#ifndef utility_interprocess_hpp_included_
#define utility_interprocess_hpp_included_

#include <boost/interprocess/sync/interprocess_mutex.hpp>

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

} // namespace utility

#endif // utility_interprocess_hpp_included_
