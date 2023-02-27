#ifndef utility_iothreads_hpp_included_
#define utility_iothreads_hpp_included_

#include <string>
#include <vector>
#include <thread>
#include <functional>

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include "atfork-asio.hpp"

namespace utility {

class IoThreads : boost::noncopyable {
public:
    IoThreads(const std::string &name, boost::asio::io_context &ioc
              , bool forkable = false);

    ~IoThreads();

    /** Callbacks called when I/O thread is started and stopped.
     */
    struct Callbacks {
        /** Callback, valled with thread id.
         */
        using Callback = std::function<void(std::size_t)>;

        Callback start;
        Callback stop;

        Callbacks(Callback start = {}, Callback stop = {})
            : start(start), stop(stop)
        {}
    };

    /** Starts given number of I/O threads. If callbacks.start is a valid
     *  fucntion, each thread calls callbacks.start(id). Thread IDs is a
     *  consecutive index starting from 1.
     */
    void start(std::size_t count, Callbacks callbacks = {});

    /** Stops all running threads started by calling IoThrreads::start(). If
     *  callbacks.stop provided to IoThrreads::start() was a valid function, it
     *  is called with appropriate thread ID from each thread.
     */
    void stop();

private:
    void worker(const std::string &name, std::size_t id
                , Callbacks callbacks);

    std::string name_;
    boost::asio::io_context &ioc_;
    boost::optional<utility::AtForkAsio> af_;
    boost::asio::io_context::work work_;

    std::vector<std::thread> workers_;
};

} // namespace utility

#endif // utility_iothreads_hpp_included_
