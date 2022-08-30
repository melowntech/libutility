#include "dbglog/dbglog.hpp"

#include "format.hpp"

#include "iothreads.hpp"

namespace utility {

IoThreads::~IoThreads()
{
    if (!workers_.empty()) { stop(); }
}

void IoThreads::start(std::size_t count)
{
    // make sure threads are released when something goes wrong
    struct Guard {
        Guard(const std::function<void()> &func) : func(func) {}
        ~Guard() { if (func) { func(); } }
        void release() { func = {}; }
        std::function<void()> func;
    } guard([this]() { stop(); });

    for (std::size_t id(1); id <= count; ++id) {
        const std::string &name((count > 1)
                                ? utility::format("%s:%u", name_, id)
                                : name_);
        workers_.emplace_back(&IoThreads::worker, this, name);
    }

    guard.release();
}

void IoThreads::stop()
{
    LOG(info2) << name_ << ": Stopping all I/O threads.";
    ioc_.stop();

    while (!workers_.empty()) {
        workers_.back().join();
        workers_.pop_back();
    }
}

void IoThreads::worker(const std::string &name)
{
    dbglog::thread_id(name);

    LOG(info2) << "I/O thread spawned.";

    for (;; ioc_.reset()) {
        try {
            ioc_.run();
            LOG(info2) << "I/O thread terminated.";
            return;
        } catch (const std::exception &e) {
            LOG(err3)
                << "Uncaught exception in I/O thread: <" << e.what()
                << ">. Going on.";
        }
    }
}

} // namespace utility
