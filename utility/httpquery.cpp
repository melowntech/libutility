/**
 * Copyright (c) 2018 Melown Technologies SE
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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "httpquery.hpp"

namespace ba = boost::algorithm;

namespace utility { namespace query {

Arguments splitQuery(const std::string &query)
{
    Arguments args;
    ba::split(args, query, ba::is_any_of("&"), ba::token_compress_on);
    return args;
}

KeyValue splitArgument(const Argument &arg)
{
    auto b(std::begin(arg));
    auto e(std::end(arg));

    for (auto i(b); i != e; ++i) {
        if (*i == '=') {
            return KeyValue(Argument(b, i), Argument(std::next(i), e));
        }
    }

    return KeyValue(Argument(b, e), Argument());
}

Argument find(const Arguments &args, const std::string &key)
{
    for (const auto &arg : args) {
        if (!ba::starts_with(arg, key)) { continue; }

        auto i(std::begin(arg) + key.size());
        if (i == std::end(arg)) { continue; }
        if (*i != '=' ) { continue; }
        return Argument(std::next(i), std::end(arg));
    }

    return {};
}

} } // namespace utility::query
