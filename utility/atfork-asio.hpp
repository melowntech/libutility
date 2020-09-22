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

#ifndef utility_atfork_asio_hpp_included_
#define utility_atfork_asio_hpp_included_

#include <utility>

#include <boost/asio.hpp>

#include "atfork.hpp"

namespace utility {

class AtForkAsio {
public:
    AtForkAsio(boost::asio::io_service &ios);

    AtForkAsio(const AtForkAsio&) = delete;
    AtForkAsio(AtForkAsio &&o) : ios_() { std::swap(ios_, o.ios_); }

    AtForkAsio& operator=(const AtForkAsio&) = delete;
    AtForkAsio& operator=(AtForkAsio &&o) {
        std::swap(ios_, o.ios_); return *this;
    }

    ~AtForkAsio() { AtFork::remove(ios_); }

private:
    boost::asio::io_service *ios_;
};

inline AtForkAsio::AtForkAsio(boost::asio::io_service &ios)
    : ios_(&ios)
{
    AtFork::add(ios_, [this](utility::AtFork::Event event)
    {
        switch (event) {
        case utility::AtFork::prepare:
            ios_->notify_fork(boost::asio::io_service::fork_prepare);
            break;

        case utility::AtFork::parent:
            ios_->notify_fork(boost::asio::io_service::fork_parent);
            break;

        case utility::AtFork::child:
            ios_->notify_fork(boost::asio::io_service::fork_child);
            break;
        }
    });
}

} // namespace utility

#endif // utility_atfork_asio_hpp_included_
