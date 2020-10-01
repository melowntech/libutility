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

#include <iostream>

#include "po.hpp"

namespace utility { namespace po {

ProgramOptions::ProgramOptions(const std::string &help)
    : od_(help)
{
    od_.add_options()
        ("help", "Show help.")
        ;
}

std::ostream& ProgramOptions::dump(std::ostream &os, const std::string&) const
{
    return os << od_;
}

bool ProgramOptions::parse(const std::vector<std::string> &args)
{
    return parse(args, std::cout);
}

bool ProgramOptions::parse(const std::vector<std::string> &args
                           , std::ostream &os)
{
    auto style
        ((boost::program_options::command_line_style::default_style
          & ~boost::program_options::command_line_style::allow_guessing));
    auto parser(boost::program_options::command_line_parser(args)
                .style(style)
                .options(od_)
                .positional(pd_));

    auto parsed(parser.run());
    boost::program_options::store(parsed, vars_);

    if (vars_.count("help")) {
        os << od_;
        return false;
    }

    boost::program_options::notify(vars_);
    return true;
}

} } // namespace utility::po
