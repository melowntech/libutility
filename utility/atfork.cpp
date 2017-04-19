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
