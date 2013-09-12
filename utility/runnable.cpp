#include <atomic>

#include <boost/interprocess/anonymous_shared_memory.hpp>

#include "runnable.hpp"

namespace bi = boost::interprocess;

namespace utility {

namespace detail {

class Simple : public Runnable {
public:
    Simple()
        : mem_(bi::anonymous_shared_memory(sizeof(std::atomic_bool)))
        , terminated_(*new (mem_.get_address()) std::atomic_bool(false))
    {}

    bool isRunning() { return !terminated_; }
    void stop() { terminated_ = true; }

private:
    bi::mapped_region mem_;
    std::atomic_bool &terminated_;
};

} // namespace detail

Runnable::Wrapper Runnable::simple()
{
    return Wrapper(std::unique_ptr<Runnable>(new detail::Simple));
}

} // namespace utility
