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

#ifndef utility_ctrlcommand_hpp_included_
#define utility_ctrlcommand_hpp_included_

#include <string>
#include <vector>
#include <iterator>
#include <stdexcept>

namespace utility {

/** Control interface command + arguments.
 */
struct CtrlCommand {
    typedef std::vector<std::string> Args;

    std::string cmd;
    Args args;

    CtrlCommand() = default;
    CtrlCommand(const std::string &cmd) : cmd(cmd) {}

    template <typename Iterator>
    CtrlCommand(const std::string &cmd, Iterator begin, Iterator end)
        : cmd(cmd), args(begin, end)
    {}

    CtrlCommand shift() const {
        if (args.empty()) { return {}; }
        return { args.front(), std::next(args.begin()), args.end() };
    }
};

struct CtrlCommandError : std::runtime_error {
    CtrlCommandError(const std::string &msg) : std::runtime_error(msg) {}
};

} // namespace utility

#endif // utility_ctrlcommand_hpp_included_
