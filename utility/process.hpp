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
#ifndef utility_process_hpp_included_
#define utility_process_hpp_included_

#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#include "detail/redirectfile.hpp"

namespace utility {

/** Execution context
 */
class ProcessExecContext;

struct Stdin : detail::RedirectFile {
    explicit Stdin(int fd) : RedirectFile(STDIN_FILENO, fd) {}
    explicit Stdin(const boost::filesystem::path &path)
        : RedirectFile(STDIN_FILENO, path, false) {}
    explicit Stdin(std::istream &is) : RedirectFile(STDIN_FILENO, is) {}
};

struct Stdout : detail::RedirectFile {
    explicit Stdout(int fd) : RedirectFile(STDOUT_FILENO, fd) {}
    explicit Stdout(const boost::filesystem::path &path
                    , bool truncate = true)
        : RedirectFile(STDOUT_FILENO, path
                       , (truncate
                          ? detail::RedirectFile::Direction::outTruncate
                          : detail::RedirectFile::Direction::out))
    {}
    explicit Stdout(std::ostream &os) : RedirectFile(STDOUT_FILENO, os) {}

    friend class ProcessExecContext;
};

struct Stderr : detail::RedirectFile {
    explicit Stderr(int fd) : RedirectFile(STDERR_FILENO, fd) {}
    explicit Stderr(const boost::filesystem::path &path
                    , bool truncate = true)
        : RedirectFile(STDERR_FILENO, path
                       , (truncate
                          ? detail::RedirectFile::Direction::outTruncate
                          : detail::RedirectFile::Direction::out))
    {}
    explicit Stderr(std::ostream &os) : RedirectFile(STDERR_FILENO, os) {}

    friend class ProcessExecContext;
};

struct Stream : detail::RedirectFile {
    explicit Stream(std::istream &is) : RedirectFile(is) {}
    explicit Stream(const std::string &format, std::istream &is)
        : RedirectFile(format, is) {}

    explicit Stream(std::ostream &os) : RedirectFile(os) {}
    explicit Stream(const std::string &format, std::ostream &os)
        : RedirectFile(format, os) {}

    friend class ProcessExecContext;
};

struct SetEnv {
    std::string name;
    std::string value;
    explicit SetEnv(const std::string &name, const std::string value = "")
        : name(name), value(value)
    {}

    friend class ProcessExecContext;
};

struct UnsetEnv {
    std::string name;
    explicit UnsetEnv(const std::string &name) : name(name) {}
};

/** Changes current working directory of a new process after fork.
 */
struct ChangeCwd {
    boost::filesystem::path wd;
    explicit ChangeCwd(const boost::filesystem::path &wd) : wd(wd) {}
};

/** Execute child process and wait for its completion.
 *
 *  Argument can be one of:
 *     Stdin/Stdout/Stderr: fd/path/stream to redirect stdin/stdout/stderr
 *                          to/from
 *     Stream/Stream: stream input/output
 *     SetEnv: sets environmental variable
 *     UnsetEnv: unsets environmental variable
 *     ChangeCwd: changes current working directory after fork
 *     other: positional argument to exec (converted to string
 *            via boost::lexical_cast)
 *     std::vector<T>: each element is interpretted based on type
 *
 *  First positional argument is interpreted as path to binary to execute.
 */
template <typename ...Args>
int system(const std::string &program, Args &&...args);

/** Execute another binary in current process
 *
 *  Argument can be one of:
 *     SetEnv: sets environmental variable
 *     UnsetEnv: unsets environmental variable
 *     ChangeCwd: changes current working directory after fork
 *     other: positional argument to exec (converted to string
 *            via boost::lexical_cast)
 *     std::vector<T>: each element is interpretted based on type
 *
 *  First positional argument is interpreted as path to binary to execute.
 */
template <typename ...Args>
void exec(const std::string &program, Args &&...args);

namespace SpawnFlag {
    typedef int value_type;
    enum {
        none = 0x00
        , quickExit = 0x01
    };
} // namespace SpawnFlag

int spawn(const std::function<int ()> &func
          , SpawnFlag::value_type flags = SpawnFlag::none);

/** Simple termination checker.
 */
bool checkTermination(::pid_t expectedPid);

/** Convenience type of generating exec arguments
 */
class ExecArgs {
public:
    typedef std::vector<char*> Argv;

    ExecArgs()
        : argv_(new Argv(), [](Argv *argv)
                            {
                                for (auto arg : *argv) { std::free(arg); }
                                delete argv;
                            })
    {}

    template <typename T> void arg(const T &a);

    void arg(const char *arg) { argv_->push_back(::strdup(arg)); }
    void arg(const std::string &a) { arg(a.c_str()); }
    void arg(const boost::filesystem::path &a) { arg(a.string()); }

    template <typename T>
    void arg(const boost::optional<T> &a) { if (a) { arg(*a); } }

    void finish() { argv_->push_back(nullptr); }

    template <typename T>
    void operator()(const T &a) { arg(a); }

    template <typename T1, typename T2>
    void operator()(const T1 &a1, const T2 &a2) { arg(a1); arg(a2); }

    const char* filename() const { return argv_->front(); }
    char* const* argv() const { return argv_->data(); }

    const Argv& args() const { return *argv_; }

private:
    std::shared_ptr<Argv> argv_;
};

/** Needs to be defined here to see other arg declarations.
 */
template <typename T> void ExecArgs::arg(const T &a) {
    arg(boost::lexical_cast<std::string>(a));
}

/** Process handle modeled after thread library.
 */
class Process {
public:
    typedef int Id;
    typedef int ExitCode;

    class Flags;

    struct Alive {};

    Process() : id_(), killed_(false) {}
    Process(Process &&other);

    template<typename Function, typename ...Args>
    Process(const Flags &flags, Function &&f, Args &&...args);

    // template<typename Function, typename ...Args>
    // Process(Function &&f, Args &&...args);

    Process(const Process&) = delete;
    ~Process();

    Process& operator=(Process &&other);
    Process& operator=(Process &other) = delete;

    Id id() const { return id_; }

    inline bool joinable() const { return id_ > 0; }

    inline void swap(Process &other) {
        std::swap(id_, other.id_);
    }

    /** Joins process. Can throw system_error, see std::thread documentation.
     *
     * \param justTry throws Alive when true and process is still running.
     */
    ExitCode join(bool justTry = false);

    /** Terminate process (soft kill).
     */
    void terminate();

    /** Kill the process (hard kill).
     */
    void kill();

    bool killed() const { return killed_; }

    /** Separates the process from the process object.
     */
    void detach();

    static void terminate(Id id);

    static void kill(Id id);

    static void signal(Id id, int signo);

private:
    static Id run(const std::function<int()> &func, const Flags &flags);

    Id id_;

    bool killed_;
};

struct ThisProcess {
    static Process::Id id();
    static Process::Id parentId();
};

} // namespace utility

#include "detail/process.hpp"

// implementation

namespace utility {

template <typename ...Args>
inline int system(const std::string &program, Args &&...args)
{
    ProcessExecContext ctx;
    detail::systemBuildArgs(ctx, std::forward<Args>(args)...);
    return detail::systemImpl(program, ctx);
}

inline int system(const std::string &program, const ProcessExecContext &ctx)
{
    return detail::systemImpl(program, ctx);
}

template <typename ...Args>
inline void exec(const std::string &program, Args &&...args)
{
    ProcessExecContext ctx;
    detail::systemBuildArgs(ctx, std::forward<Args>(args)...);
    return detail::execImpl(program, ctx);
}

inline void exec(const std::string &program, const ProcessExecContext &ctx)
{
    return detail::execImpl(program, ctx);
}

/** Call function in new process.
 *  Convenient wrapper around fork(2)
 */
inline int spawn(const std::function<int ()> &func
                 , SpawnFlag::value_type flags)
{
    return detail::spawnImpl(func, flags);
}

class Process::Flags {
public:
    Flags() : quickExit_(false) {}
    Flags& quickExit(bool value) { quickExit_ = value; return *this; }
    bool quickExit() const { return quickExit_; }

private:
    bool quickExit_;
};

inline Process::Process(Process &&other)
    : id_(other.id_), killed_(other.killed_)
{
    other.id_ = 0;
    other.killed_ = false;
}

inline Process& Process::operator=(Process &&other)
{
    if (joinable()) { std::terminate(); }
    id_ = other.id_;
    killed_ = other.killed_;
    other.id_ = 0;
    other.killed_ = false;
    return *this;
}

namespace detail {

template <typename ResultOf>
struct ProcessFunction {
    template<typename Function, typename ...Args>
    static std::function<int()> make(Function &&f, Args &&...args)
    {
        auto func = std::bind<void>(std::forward<Function>(f)
                                    , std::forward<Args>(args)...);
        return [func]() { func(); return EXIT_SUCCESS; };
    }
};

template <>
struct ProcessFunction<int> {
    template<typename Function, typename ...Args>
    static std::function<int()> make(Function &&f, Args &&...args)
    {
        return std::bind<int>(std::forward<Function>(f)
                              , std::forward<Args>(args)...);
    }
};

} // namespace detail

template<class Function, typename ...Args>
inline Process::Process(const Flags &flags, Function &&f, Args &&...args)
    : killed_(false)
{
    typedef decltype(std::bind(std::forward<Function>(f)
                               , std::forward<Args>(args)...)
                     (std::forward<Args>(args)...)) FunctionResult;
    id_ = run(detail::ProcessFunction<FunctionResult>::make
              (std::forward<Function>(f), std::forward<Args>(args)...)
              , flags);
}

std::ostream& operator<<(std::ostream &os, const ExecArgs &a);

} // namespace utility

#endif // utility_process_hpp_included_
