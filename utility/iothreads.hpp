#ifndef utility_iothreads_hpp_included_
#define utility_iothreads_hpp_included_

#include <string>
#include <vector>
#include <thread>

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

namespace utility {

class IoThreads : boost::noncopyable {
public:
    IoThreads(const std::string &name, boost::asio::io_context &ioc)
        : name_(name), ioc_(ioc), work_(ioc_)
    {}

    ~IoThreads();

    void start(std::size_t count);

    void stop();

private:
    void worker(const std::string &name);

    std::string name_;
    boost::asio::io_context &ioc_;
    boost::asio::io_context::work work_;

    std::vector<std::thread> workers_;
};

} // namespace utility

#endif // utility_iothreads_hpp_included_
