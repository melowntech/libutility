/**
 * @file utility/asyncqueue.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_asyncqueue_hpp_included_
#define utility_asyncqueue_hpp_included_

#include <functional>

namespace utility {

class AsyncQueue {
public:
    typedef std::function<void()> Operation;

    virtual ~AsyncQueue() {}
    void post(const Operation &op) const;

private:
    virtual void post_impl(const Operation &op) const = 0;
};

// inlines

inline void AsyncQueue::post_impl(const Operation &op) const
{
    return post_impl(op);
}

} // namespace utility

#endif // utility_asyncqueue_hpp_included_
