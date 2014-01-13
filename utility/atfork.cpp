#include <pthread.h>

#include <algorithm>

#include "dbglog/dbglog.hpp"

#include "atfork.hpp"

namespace utility {

std::vector<AtFork::Entry> AtFork::entries_;

AtFork AtFork::init_;

void AtFork::add(const void *id, const Callback &cb)
{
    entries_.emplace_back(id, cb);
}

void AtFork::remove(const void *id)
{
    // remove ios
    auto nend(std::remove_if(entries_.begin(), entries_.end()
                             , [id](const Entry &e) {
                                 return e.id == id;
                             }));
    // trim
    entries_.resize(nend - entries_.begin());
}

void AtFork::run(Event event)
{
    for (auto &entry : entries_) {
        entry.cb(event);
    }
}

extern "C" {
    void utility_signalhandler_atfork_pre() {
        LOG(info1) << "utility_signalhandler_atfork_pre";
        AtFork::run(AtFork::prepare);
    }

    void utility_signalhandler_atfork_parent() {
        LOG(info1) << "utility_signalhandler_atfork_parent";
        AtFork::run(AtFork::parent);
    }

    void utility_signalhandler_atfork_child() {
        LOG(info1) << "utility_signalhandler_atfork_child";
        AtFork::run(AtFork::child);
    }
}

AtFork::AtFork()
{
    if (-1 == ::pthread_atfork(&utility_signalhandler_atfork_pre
                               , utility_signalhandler_atfork_parent
                               , &utility_signalhandler_atfork_child))
    {
        LOG(fatal) << "Atfork registration failed: " << errno;
        _exit(EXIT_FAILURE);
    }
}

} // namespace utility
