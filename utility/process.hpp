#ifndef utility_process_hpp_included_
#define utility_process_hpp_included_

#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <stdexcept>
#include <system_error>
#include <iosfwd>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/any.hpp>

namespace utility {

struct RedirectFile {
    RedirectFile() : dst(-1), out(false) {}
    RedirectFile(int dst, int src, bool out)
        : dst(dst), value(src), type(SrcType::fd), out(out)
    {}
    RedirectFile(int dst, const boost::filesystem::path &path, bool out)
        : dst(dst), value(path), type(SrcType::path), out(out) {}
    RedirectFile(int dst, std::istream &is)
        : dst(dst), value(&is), type(SrcType::istream), out(false) {}
    RedirectFile(int dst, std::ostream &os)
        : dst(dst), value(&os), type(SrcType::ostream), out(true) {}

    int dst;
    boost::any value;
    enum class SrcType { fd, path, istream, ostream };
    SrcType type;
    bool out;
};

struct Stdin : RedirectFile {
    Stdin(int fd) : RedirectFile(STDIN_FILENO, fd, false) {}
    Stdin(const boost::filesystem::path &path)
        : RedirectFile(STDIN_FILENO, path, false) {}
    Stdin(std::istream &is) : RedirectFile(STDIN_FILENO, is) {}
};

struct Stdout : RedirectFile {
    Stdout(int fd) : RedirectFile(STDOUT_FILENO, fd, true) {}
    Stdout(const boost::filesystem::path &path)
        : RedirectFile(STDOUT_FILENO, path, true) {}
    Stdout(std::ostream &os) : RedirectFile(STDOUT_FILENO, os) {}
};

struct Stderr : RedirectFile {
    Stderr(int fd) : RedirectFile(STDERR_FILENO, fd, true) {}
    Stderr(const boost::filesystem::path &path)
        : RedirectFile(STDERR_FILENO, path, true) {}
    Stderr(std::ostream &os) : RedirectFile(STDERR_FILENO, os) {}
};

struct InStream : RedirectFile {
    InStream(std::istream &is) : RedirectFile(-1, is) {}
};

struct OutStream : RedirectFile {
    OutStream(std::ostream &os) : RedirectFile(-1, os) {}
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
 *     Stdin/Stdout/Stderr: fd/path/stream to redirect stdin/stdout/stderr
 *                          to/from
 *     InStream/OutStream: stream input/output
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
