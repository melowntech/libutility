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
#ifndef utility_detail_process_hpp_included_
#define utility_detail_process_hpp_included_

#include <vector>
#include <map>
#include <utility>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>

#include "../process.hpp"
#include "../environment.hpp"

namespace utility {

class ProcessExecContext {
public:
    typedef std::vector<boost::optional<std::string> > Argv;
    typedef std::vector<detail::RedirectFile> Redirects;
    typedef std::map<int, int> PlaceHolders;

    Argv argv;
    Redirects redirects;
    PlaceHolders placeHolders;

    Environment environ;

    boost::optional<boost::filesystem::path> cwd;

    void add(const detail::RedirectFile &arg) {
        // TODO: check for duplicity

        placeHolders[redirects.size()] = argv.size();

        redirects.push_back(arg);

        // placeholder for redirect argument (if any)
        argv.push_back(boost::none);
    }

    void apply(const Stdin &arg) { add(arg); }
    void apply(const Stderr &arg) { add(arg); }
    void apply(const Stdout &arg) { add(arg); }
    void apply(const Stream &arg) { add(arg); }
    void apply(const SetEnv &arg) { environ[arg.name] = arg.value; }
    void apply(const UnsetEnv &arg) { environ[arg.name] = boost::none; }
    void apply(const ChangeCwd &arg) { cwd = arg.wd; }
    void apply(const std::string &arg) { argv.push_back(arg); }
    void apply(const boost::filesystem::path &arg) {
        argv.push_back(arg.string());
    }
    void apply(const ProcessExecContext &arg) { *this = arg; }

    template <typename T>
    void apply(const T &arg) {
        argv.push_back(boost::lexical_cast<std::string>(arg));
    }

    template <typename T>
    void apply(const std::vector<T> &args)
    {
        for (const auto &arg : args) {
            apply(arg);
        }
    }

    void setFdPath(int redirectIdx, const detail::RedirectFile::DstArg &arg
                   , int fd);

    /** Abuse dump.
     */
    template <typename E, typename T>
    std::basic_ostream<E, T>& dump(std::basic_ostream<E, T> &os
                                   , const std::string &exePath) const;
};

namespace detail {

inline void systemBuildArgs(ProcessExecContext&) {}

template <typename T, typename ...Args>
inline void systemBuildArgs(ProcessExecContext &ctx, T &&arg, Args &&...rest)
{
    // ctx.argv.push_back(boost::lexical_cast<std::string>(arg));
    ctx.apply(std::move(arg));
    return detail::systemBuildArgs(ctx, std::forward<Args>(rest)...);
}

/** Equivalent of system(3); does fork/exec and waits for result.
 */
int systemImpl(const std::string &program, ProcessExecContext ctx);

/** Equivalent of execvpe(3); replaces current process
 */
void execImpl(const std::string &program, ProcessExecContext ctx);

int spawnImpl(const std::function<int ()> &func, int flags);

} // namespace detail

template <typename E, typename T>
inline std::basic_ostream<E, T>&
ProcessExecContext::dump(std::basic_ostream<E, T> &os
                         , const std::string &exePath) const
{

    std::string separator("");

    // print environment
    for (const auto &e : environ) {
        if (e.second) {
            os << separator << e.first << '=' << *e.second;
        } else {
            os << separator << "unset(" << e.first << ")";
        }
    }

    os << separator << exePath;

    // print parameters
    for (const auto arg : argv) {
        if (arg) {
            os << " " << *arg;
        }
    }

    return os;
}

} // namespace utility

#endif // utility_detail_process_hpp_included_
