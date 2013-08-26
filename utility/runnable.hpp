#ifndef shared_utility_runnable_hpp_included_
#define shared_utility_runnable_hpp_included_

#include <atomic>
#include <boost/noncopyable.hpp>

namespace utility {

class Runnable : boost::noncopyable {
public:
    virtual ~Runnable() {}
    virtual bool isRunning() = 0;
    virtual void stop() = 0;

    inline operator bool() { return isRunning(); }

    class Simple;
};

class Runnable::Simple : public Runnable
{
public:
    Simple() : running_(true) {}
    bool isRunning() { return running_; }
    void stop() { running_ = false; }

private:
    std::atomic_bool running_;
};

} // namespace utility

#endif // shared_utility_runnable_hpp_included_
