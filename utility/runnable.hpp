#ifndef shared_utility_runnable_hpp_included_
#define shared_utility_runnable_hpp_included_

#include <memory>
#include <boost/noncopyable.hpp>

namespace utility {

class Runnable : boost::noncopyable {
public:
    virtual ~Runnable() {}
    virtual bool isRunning() = 0;
    virtual void stop() = 0;

    inline operator bool() { return isRunning(); }

    /** Creates simple shareable runnable.
     */
    static std::unique_ptr<Runnable> simple();
};

} // namespace utility

#endif // shared_utility_runnable_hpp_included_
