#ifndef utility_process_hpp_included_
#define utility_process_hpp_included_

#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <iosfwd>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include "./detail/redirectfile.hpp"

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
    explicit Stdout(const boost::filesystem::path &path)
        : RedirectFile(STDOUT_FILENO, path, true) {}
    explicit Stdout(std::ostream &os) : RedirectFile(STDOUT_FILENO, os) {}

    friend class ProcessExecContext;
};

struct Stderr : detail::RedirectFile {
    explicit Stderr(int fd) : RedirectFile(STDERR_FILENO, fd) {}
    explicit Stderr(const boost::filesystem::path &path)
        : RedirectFile(STDERR_FILENO, path, true) {}
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

} // namespace utility

#include "./detail/process.hpp"

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

} // namespace utility

#endif // utility_process_hpp_included_
