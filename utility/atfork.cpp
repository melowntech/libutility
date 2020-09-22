/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <vector>
#include <mutex>
#include <algorithm>

#include <pthread.h>

#include "dbglog/dbglog.hpp"

#include "atfork.hpp"
#include "enum-io.hpp"

namespace utility {

UTILITY_GENERATE_ENUM_IO(AtFork::Event,
                         ((prepare))
                         ((parent))
                         ((child))
                         )

struct AtFork_Detail {
    struct Entry {
        const void *id;
        AtFork::Callback cb;

        Entry(const void *id, AtFork::Callback cb) : id(id), cb(cb) {}
        Entry() : id(), cb() {}

        typedef std::vector<Entry> list;
    };

    AtFork_Detail();
    ~AtFork_Detail() = default;

    void add(const void *id, const AtFork::Callback &cb) {
        std::lock_guard<std::mutex> guard(mutex);
        entries.emplace_back(id, cb);
    }

    void remove(const void *id) {
        std::lock_guard<std::mutex> guard(mutex);

        // remove handler
        auto nend(std::remove_if(entries.begin(), entries.end()
                                 , [id](const Entry &e) {
                                       return e.id == id;
                                   }));
        // trim
        entries.resize(nend - entries.begin());
    }

    void run(AtFork::Event event) {
        // first, lock/unlock
        switch (event) {
        case AtFork::Event::prepare: mutex.lock(); break;
        case AtFork::Event::parent: mutex.unlock(); break;
        case AtFork::Event::child: new (&mutex) std::mutex(); break;
        }

        for (auto &entry : entries) {
            try {
                entry.cb(event);
            } catch (const std::exception &e) {
                LOG(err3) << "Failed to run at-fork event <"
                          << event << "> for id="
                          << entry.id << ": " << e.what();
            } catch (...) {
                LOG(err3) << "Failed to run at-fork event <"
                          << event << "> for id="
                          << entry.id << ": unknown error.";
            }
        }
    }

    std::mutex mutex;
    Entry::list entries;
};

AtFork_Detail *detail = new AtFork_Detail();

void AtFork::add(const void *id, const AtFork::Callback &cb)
{
    detail->add(id, cb);
}

void AtFork::remove(const void *id)
{
    detail->remove(id);
}

extern "C" {
    void utility_signalhandler_atfork_pre() {
        LOG(info1) << "utility_signalhandler_atfork_pre";
        detail->run(AtFork::Event::prepare);
    }

    void utility_signalhandler_atfork_parent() {
        LOG(info1) << "utility_signalhandler_atfork_parent";
        detail->run(AtFork::Event::parent);
    }

    void utility_signalhandler_atfork_child() {
        LOG(info1) << "utility_signalhandler_atfork_child";
        detail->run(AtFork::Event::child);
    }
}

AtFork_Detail::AtFork_Detail()
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
