#ifndef shared_utility_runnable_hpp_included_
#define shared_utility_runnable_hpp_included_

#include <memory>
#include <utility>
#include <boost/noncopyable.hpp>

namespace utility {

class Runnable : boost::noncopyable {
public:
    Runnable() = default;
    Runnable(Runnable &&) = default;
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
