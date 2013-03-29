/**
 * @file scopedguard.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Scoped guard support
 */

#ifndef utility_scopedguard_hpp_included_
#define utility_scopedguard_hpp_included_

#include <functional>
#include <exception>

#include <boost/noncopyable.hpp>

namespace utility {

class ScopedGuard : boost::noncopyable {
public:
    ScopedGuard(std::function<void()> cleanup)
        : cleanup_(cleanup)
    {}

    ~ScopedGuard() {
        if (std::uncaught_exception()) {
            // some other exception -> do not propagate
            try { cleanup_(); } catch (...) {}
        } else {
            // fine to propagate exception
            cleanup_();
        }
    }

    void reset() { cleanup_ = [](){}; }

private:
    std::function<void()> cleanup_;
};

} // namespace utility

#endif // utility_scopedguard_hpp_included_
