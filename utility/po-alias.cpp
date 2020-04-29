/**
 * Copyright (c) 2020 Melown Technologies SE
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

#include <boost/algorithm/string.hpp>

#include "po-alias.hpp"

namespace ba = boost::algorithm;

namespace utility { namespace po {

namespace {

std::pair<std::string, std::string>
renameOptions(const std::string &token, const OptionAliases &aliases)
{
    auto rtoken(boost::make_iterator_range(token));

    // consume "--" prefix
    if (!ba::starts_with(rtoken, "--")) { return { "", "" }; }
    rtoken.advance_begin(2);

    // find equal sign (returns iterator range)
    const auto eq(ba::find_first(rtoken, "="));

    // extract option (between "--prefix" and "="/end() and map it to output
    const auto faliases(aliases.find(std::string(rtoken.begin(), eq.begin())));
    if (faliases == aliases.end()) { return { "", "" }; }

    // return remapped option and (optionally) value after "="
    return std::make_pair(faliases->second
                          , std::string(eq.end(), rtoken.end()));
}

} // namespace

boost::program_options::ext_parser optionAlias(OptionAliases &&aliases)
{
    return [aliases{std::move(aliases)}](const std::string &token)
    {
        return renameOptions(token, aliases);
    };
}

} } // namespace utility::po
