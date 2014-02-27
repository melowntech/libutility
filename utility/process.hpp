#ifndef utility_process_hpp_included_
#define utility_process_hpp_included_

#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <stdexcept>
#include <system_error>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

namespace utility {

struct RedirectFile {
    int dst;
    int src;
    boost::filesystem::path path;
    enum class Type { fd, path };
    Type type;
    bool out;

    RedirectFile() : dst(-1), src(-1), out(false) {}
    RedirectFile(int dst, int src, bool out)
        : dst(dst), src(src), type(Type::fd), out(out) {}
    RedirectFile(int dst, const boost::filesystem::path &path
                 , bool out)
        : dst(dst), src(-1), path(path), type(Type::path), out(out) {}
};

struct Stdin : RedirectFile {
    Stdin(int fd) : RedirectFile(STDIN_FILENO, fd, false) {}
    Stdin(const boost::filesystem::path &path)
        : RedirectFile(STDIN_FILENO, path, false) {}
};

struct Stdout : RedirectFile {
    Stdout(int fd) : RedirectFile(STDOUT_FILENO, fd, true) {}
    Stdout(const boost::filesystem::path &path)
        : RedirectFile(STDOUT_FILENO, path, true) {}
};

struct Stderr : RedirectFile {
    Stderr(int fd) : RedirectFile(STDERR_FILENO, fd, true) {}
    Stderr(const boost::filesystem::path &path)
        : RedirectFile(STDERR_FILENO, path, true) {}
};

struct SetEnv {
    std::string name;
    std::string value;
    SetEnv(const std::string &name, const std::string value = "")
        : name(name), value(value)
    {}
};

struct UnsetEnv {
    std::string name;
    UnsetEnv(const std::string &name) : name(name) {}
};

/** Execute child process and wait for its completion.
 *
 *  Argument can be one of:
 *     InFile/OutFile/ErrFile: file to redirect stdin/stdout/stderr to/from
 *     SetEnv: sets environmental variable
 *     UnsetEnv: unsets environmental variable
 *     other: positional argument to exec (converted to string
 *            via boost::lexical_cast)
 *
 *  First positional argument is interpreted as path to binary to execute.
 */
template <typename ...Args>
int system(const std::string &program, Args &&...args);

int spawn(const std::function<int ()> &func);

} // namespace utility

#include "./detail/process.hpp"

// implementation

namespace utility {

template <typename ...Args>
inline int system(const std::string &program, Args &&...args)
{
    detail::Context ctx;
    detail::systemBuildArgs(ctx, std::forward<Args>(args)...);
    return detail::systemImpl(program, ctx);
}

/** Call function in new process.
 *  Convenient wrapper around fork(2)
 */
inline int spawn(const std::function<int ()> &func)
{
    return detail::spawnImpl(func);
}

} // namespace utility

#endif // utility_process_hpp_included_
