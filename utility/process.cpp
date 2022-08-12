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
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <system_error>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "dbglog/dbglog.hpp"

#include "process.hpp"
#include "uncaught-exception.hpp"

namespace fs = boost::filesystem;

namespace utility {

std::ostream& operator<<(std::ostream &os, const ExecArgs &a)
{
    bool first = true;
    for (const auto *arg : a.args()) {
        if (!arg) { break; } // final nullptr
        os << (first ? "" : " ") << arg;
        first = false;
    }
    return os;
}

void ProcessExecContext::setFdPath(int redirectIdx
                                   , const detail::RedirectFile::DstArg &arg
                                   , int fd)
{
    const std::string devPath("/dev/fd/%d");

    auto fplaceHolders(placeHolders.find(redirectIdx));
    if (fplaceHolders == placeHolders.end()) {
        LOGTHROW(err1, std::runtime_error)
            << "system: invalid redirect index (" << redirectIdx << ").";
    }

    std::string format;
    // if there is no format -> set to dev path
    // else if format contains %d -> replace with fd
    // else if format contains %s -> replace with dev path
    // otherwise just append dev path

    if (arg.format.find("%d") != std::string::npos) {
        format = arg.format;
        // OK
    } else if (arg.format.find("%s") != std::string::npos) {
        format = str(boost::format(arg.format) % devPath);
    } else {
        format = arg.format + devPath;
    }

    argv[fplaceHolders->second] = str(boost::format(format) % fd);
}

namespace detail {

constexpr int EXEC_FAILED = 255;

void useFd(int dst, int src, bool closeSrc = false)
{
    // ignore invalid descriptors
    if ((src < 0) || (dst < 0)) { return; }
    if (-1 == ::dup2(src, dst)) {
        std::system_error e(errno, std::system_category());
        LOG(warn1)
            << "dup2(" << src << ", " << dst << ") failed: <" << e.code()
            << ", " << e.what() << ">";
        throw e;
    }

    if (closeSrc) { ::close(src); }
}

pid_t execute(const ExecArgs &argv, const boost::function<void()> &afterFork)
{
    LOG(info2) << "Executing: " << argv;

    auto pid(::fork());
    if (-1 == pid) {
        // Oops, failed
        std::system_error e(errno, std::system_category());
        LOG(err2) << "fork(2) failed: <" << e.code()
                  << ", " << e.what() << ">";
        throw e;
    } else if (0 == pid) {
        // child -> prepare and exec

        // call afterfork callback
        afterFork();

        // exec
#ifdef __APPLE__
        if (::execvp(argv.filename(), argv.argv()) == -1)
#else
        if (::execvpe(argv.filename(), argv.argv(), ::environ) == -1)
#endif
        {
            std::system_error e(errno, std::system_category());
            LOG(err2) << "execve(2) [" << argv.filename()
                      << "] failed: <" << e.code() << ", "
                      << e.what() << ">";
            std::exit(EXEC_FAILED);
        }

        // never reached
    }

    return pid;
}

void execute(const ExecArgs &argv)
{
    LOG(info2) << "Executing: " << argv;

    // exec
#ifdef __APPLE__
    if (::execvp(argv.filename(), argv.argv()) == -1)
#else
    if (::execvpe(argv.filename(), argv.argv(), ::environ) == -1)
#endif
    {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "execve(2) [" << argv.filename()
                  << "] failed: <" << e.code() << ", "
                  << e.what() << ">";
        throw(e);
    }
    // never reached
}

void redirect(const ProcessExecContext::Redirects &redirects)
{
    for (const auto &redirect : redirects) {
        switch (redirect.srcType) {
        case RedirectFile::SrcType::path: {
            if (redirect.dstType != RedirectFile::DstType::fd) {
                LOGTHROW(err1, std::runtime_error)
                    << "redirect: path source can be used only with "
                    "file descriptor destination!";
            }

            auto src(boost::any_cast<RedirectFile::SrcPath>(redirect.src));
            int fd(-1);
            switch (src.dir) {
            case RedirectFile::Direction::in:
                fd = ::open(src.path.string().c_str(), O_RDONLY);
                break;
            case RedirectFile::Direction::outTruncate:
                fd = ::open(src.path.string().c_str()
                            , O_WRONLY | O_CREAT | O_TRUNC
                            , (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
                break;
            case RedirectFile::Direction::out:
                fd = ::open(src.path.string().c_str()
                            , O_WRONLY | O_CREAT | O_APPEND
                            , (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
                break;
            }

            if (fd == -1) {
                std::system_error e(errno, std::system_category());
                LOG(err1) << "Cannot open file " << src.path << ": <"
                          << e.code() << ", " << e.what() << ">.";
                throw e;
            }
            useFd(boost::any_cast<int>(redirect.dst), fd, true);
            break;
        }

        case RedirectFile::SrcType::fd:
            if (redirect.dstType != RedirectFile::DstType::fd) {
                LOGTHROW(err1, std::runtime_error)
                    << "redirect: file descriptor source can be used only "
                    "with file descriptor destination!";
            }

            useFd(boost::any_cast<int>(redirect.dst)
                  , boost::any_cast<int>(redirect.src));
            break;

        case RedirectFile::SrcType::istream:
        case RedirectFile::SrcType::ostream:
            LOGTHROW(err1, std::runtime_error)
                << "redirect; streams must be converted to file "
                "descriptors first!";
            break;
        }
    }
}

class Pipe : boost::noncopyable {
public:
    Pipe()
        : pipe_{-1, -1}
    {
        LOG(info1) << "creating pipe";
        if (-1 == ::pipe(pipe_)) {
            std::system_error e(errno, std::system_category());
            LOG(warn1)
                << "Cannot create pipe: "
                << "<" << e.code() << ", " << e.what() << ">.";
            throw e;
        }
    }

    Pipe(Pipe &&other)
        : pipe_{ -1, -1}
    {
        std::swap(pipe_[0], other.pipe_[0]);
        std::swap(pipe_[1], other.pipe_[1]);
    }

    Pipe& operator=(Pipe &&other) {
        std::swap(pipe_[0], other.pipe_[0]);
        std::swap(pipe_[1], other.pipe_[1]);
        return *this;
    }

    ~Pipe() {
        closeIn();
        closeOut();
    }

    int in() const { return pipe_[0]; }
    int out() const { return pipe_[1]; }

    void releaseIn() { pipe_[0] = -1; }
    void releaseOut() { pipe_[1] = -1; }

    void closeIn() {
        if (in() > -1) {
            ::close(in());
            pipe_[0] = -1;
        }
    }

    void closeOut() {
        if (out() > -1) {
            ::close(out());
            pipe_[1] = -1;
        }
    }

private:
    int pipe_[2];
};

class OutPipe : public Pipe {
public:
    OutPipe(std::ostream &stream)
        : stream_(&stream)
    {}

    std::ostream &stream() { return *stream_; }

private:
    std::ostream *stream_;
};

class InPipe : public Pipe {
public:
    InPipe(std::istream &stream)
        : stream_(&stream)
    {}

    std::istream &stream() { return *stream_; }

private:
    std::istream *stream_;
};

namespace asio = boost::asio;
namespace lib = std;
namespace placeholders = std::placeholders;

struct InSocket {
    InSocket(asio::io_service &ios, InPipe &pipe)
        : socket(ios, pipe.out())
        , stream(&pipe.stream())
    {
        pipe.releaseOut();
        pipe.closeIn();
        buffer.resize(4096);
        socket.non_blocking(true);

        // we want to fail on bad io
        stream->exceptions(std::ios::badbit);
    }

    asio::posix::stream_descriptor socket;
    std::istream *stream;
    std::vector<char> buffer;
};

struct OutSocket {
    OutSocket(asio::io_service &ios, OutPipe &pipe)
        : socket(ios, pipe.in())
        , stream(&pipe.stream())
    {
        pipe.releaseIn();
        pipe.closeOut();
        buffer.resize(4096);
        socket.non_blocking(true);

        // we want to fail on bad io
        stream->exceptions(std::ios::badbit);
    }

    asio::posix::stream_descriptor socket;
    std::ostream *stream;
    std::vector<char> buffer;
};

struct Pump
{
    Pump(pid_t pid, std::vector<InPipe> &inPipes
         , std::vector<OutPipe> &outPipes)
        : pid(pid), inPipes(inPipes), outPipes(outPipes)
        , signals(ios, SIGCHLD)
    {
        // initialize sockets
        for (auto &pipe : inPipes) { inSockets.emplace_back(ios, pipe); }
        for (auto &pipe : outPipes) { outSockets.emplace_back(ios, pipe); }
    }

    void startSignals() {
        signals.async_wait(lib::bind(&Pump::signal, this
                                     , placeholders::_1
                                     , placeholders::_2));
    }

    void signal(const boost::system::error_code &e, int signo) {
        if (e) {
            if (boost::asio::error::operation_aborted == e) {
                return;
            }
            startSignals();
        }
        // ignore signal '0'
        if (!signo) {
            startSignals();
            return;
        }

        // child terminated
        LOG(info1) << "stopped";
        ios.stop();
    }

    void write(InSocket &s) {
        // readsome returns data from buffer => we need to force underflow to be
        // called
        s.stream->peek();
        // read data from input
        auto size(s.stream->readsome(s.buffer.data(), s.buffer.size()));
        if (!size) {
            if (s.stream->eof()) {
                // eof
                LOG(info1) << "input stream closed";
                s.socket.close();
            }
            // should never happen
            return;
        }
        asio::async_write(s.socket, asio::buffer(s.buffer.data(), size)
                          , lib::bind(&Pump::allWritten, this
                                     , placeholders::_1
                                      , placeholders::_2
                                      , std::ref(s)));
    }

    void allWritten(const boost::system::error_code &e
                    , std::size_t bytes, InSocket &s)
    {
        if (!e) {
            LOG(info1) << "Written: " << bytes << " bytes to child.";
            write(s);
        } else if (e.value() != asio::error::operation_aborted) {
            LOGTHROW(err1, std::runtime_error)
                << "Write to child process " << pid << " failed: <" << e
                << ">.";
        }
    }

    void read(OutSocket &s) {
        s.socket.async_read_some(asio::buffer(s.buffer.data(), s.buffer.size())
                          , lib::bind(&Pump::somethingRead, this
                                      , placeholders::_1
                                      , placeholders::_2
                                      , std::ref(s)));
    }

    void somethingRead(const boost::system::error_code &e
                       , std::size_t bytes, OutSocket &s)
    {
        if (!e) {
            LOG(info1) << "read: " << bytes << " bytes.";
            s.stream->write(s.buffer.data(), bytes);
            read(s);
        } else {
            if ((e.value() == asio::error::operation_aborted)
                || e.value() == asio::error::eof)
            {
                return;
            }
            LOGTHROW(err1, std::runtime_error)
                << "Read from child process " << pid << " failed: <" << e
                << ">.";
        }
    }

    int run();

    pid_t pid;
    std::vector<InPipe> &inPipes;
    std::vector<OutPipe> &outPipes;

    asio::io_service ios;
    asio::signal_set signals;

    std::vector<InSocket> inSockets;
    std::vector<OutSocket> outSockets;
};

int Pump::run()
{
    startSignals();
    for (auto &socket : inSockets) { write(socket); }
    for (auto &socket : outSockets) { read(socket); }

    {
        // this wrapper kills process on exception leaking from ios.run()
        // without catching it
        struct Kill {
            pid_t pid;
            Kill(pid_t pid) : pid(pid) {}
            ~Kill() {
                if (utility::uncaught_exception()) {
                    ::kill(pid, SIGKILL);
                }
            }
        } kill(pid);
        ios.run();
    }

    LOG(info1) << "waiting for child";
    // wait for child
    int status;
    for (;;) {
        auto res(::waitpid(pid, &status, 0x0));
        if (res < 0) {
            if (EINTR == errno) { continue; }

            std::system_error e(errno, std::system_category());
            LOG(warn1) << "waitpid(2) failed: <" << e.code()
                       << ", " << e.what() << ">";
            throw e;
        }
        break;
    }

    LOG(info1) << "waiting for child: status=" << status;

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

void childClose(std::vector<InPipe> &pipes)
{
    for (auto &p : pipes) {
        p.closeOut();
    }
}

void childClose(std::vector<OutPipe> &pipes)
{
    for (auto &p : pipes) {
        p.closeIn();
    }
}

int systemImpl(const std::string &program, ProcessExecContext ctx)
{
    std::vector<InPipe> inPipes;
    std::vector<OutPipe> outPipes;

    // prepare redirects
    int idx(0);
    for (auto &r : ctx.redirects) {
        switch (r.srcType) {
        case RedirectFile::SrcType::istream: {
            // add input pipe
            inPipes.emplace_back(*boost::any_cast<std::istream*>(r.src));

            // add pipe's write end to input
            int fd(inPipes.back().in());
            switch (r.dstType) {
            case RedirectFile::DstType::fd:
                r = { boost::any_cast<int>(r.dst), fd };
                break;

            case RedirectFile::DstType::arg:
                ctx.setFdPath(idx, boost::any_cast<RedirectFile::DstArg>
                              (r.dst), fd);
                r = { -1, -1 };
                break;

            case RedirectFile::DstType::none:
                // forget
                r = { -1, -1 };
                break;
            }
            break;
        }

        case RedirectFile::SrcType::ostream: {
            // add input pipe
            outPipes.emplace_back(*boost::any_cast<std::ostream*>(r.src));

            // add pipe's write end to input
            int fd(outPipes.back().out());
            switch (r.dstType) {
            case RedirectFile::DstType::fd:
                r = { boost::any_cast<int>(r.dst), fd };
                break;

            case RedirectFile::DstType::arg:
                ctx.setFdPath(idx, boost::any_cast<RedirectFile::DstArg>
                              (r.dst), fd);
                r = { -1, -1 };
                break;

            case RedirectFile::DstType::none:
                // forget
                r = { -1, -1 };
                break;
            }
            break;
        }

        default:
            // keep
            break;
        }
        ++idx;
    }

    // build arguments
    ExecArgs argv;
    argv.arg(program);
    for (const auto &arg : ctx.argv) {
        if (arg) {
            argv.arg(*arg);
        }
    }
    argv.finish();

    auto pid(detail::execute
             (argv,
              [&ctx, &inPipes, &outPipes]() {
                 try {
                     childClose(inPipes);
                     childClose(outPipes);
                     redirect(ctx.redirects);
                     apply(ctx.environ);
                     // chdir to wd if set
                     if (ctx.cwd) { current_path(*ctx.cwd); }
                 } catch (const std::exception &e) {
                     std::exit(EXEC_FAILED);
                 }
             }));

    LOG(info2) << "Running under pid: " << pid << ".";

    return Pump(pid, inPipes, outPipes).run();
}

void execImpl(const std::string &program, ProcessExecContext ctx)
{
    // build arguments
    ExecArgs argv;
    argv.arg(program);
    for (const auto &arg : ctx.argv) {
        if (arg) {
            argv.arg(*arg);
        }
    }
    argv.finish();

    redirect(ctx.redirects);
    apply(ctx.environ);
    // chdir to wd if set
    if (ctx.cwd) { current_path(*ctx.cwd); }

    detail::execute(argv);
}

int spawnImpl(const std::function<int ()> &func, int flags)
{
    auto pid(::fork());
    if (-1 == pid) {
        // Oops, failed
        std::system_error e(errno, std::system_category());
        LOG(err2) << "fork(2) failed: <" << e.code()
                  << ", " << e.what() << ">";
        throw e;
    } else if (0 == pid) {
        // run function
        auto res(func());

        if (flags & SpawnFlag::quickExit) {
            // quick exit
            ::_Exit(res);
        }
        std::exit(res);
    }

    return pid;
}

}

void apply(const Environment &environ)
{
    for (const auto &e : environ) {
        if (e.second) {
            ::setenv(e.first.c_str(), e.second->c_str(), true);
        } else {
            ::unsetenv(e.first.c_str());
        }
    }
}


bool checkTermination(::pid_t expectedPid)
{
    if (!expectedPid) {
        return true;
    }

    int status;
    pid_t pid(::waitpid(expectedPid, &status, WNOHANG));
    if (!pid) {
        return false;
    }
    if (pid == -1) {
        if (errno != ECHILD) {
            std::system_error e(errno, std::system_category());
            LOG(warn3) << "waitpid(2) failed: <" << e.code()
                       << ", " << e.what() << ">";
            return false;
        }
        // no such child -> bail out
        return true;
    }

    return pid == expectedPid;
}

Process::~Process()
{
    if (joinable()) {
        LOG(fatal) << "Destroying utility::Process with assigned pid.";
        std::terminate();
    }
}

Process::ExitCode Process::join(bool justTry)
{

    if (!joinable()) {
        std::system_error e(EINVAL, std::system_category());
        LOG(err3) << "Cannot join non-joinable process.";
        throw e;
    }

    if (id_ == ::getpid()) {
        std::system_error e(EDEADLK, std::system_category());
        LOG(err3) << "Cannot join a process from within.";
        throw e;
    }

    int status;
    int options(0);
    if (justTry) { options |= WNOHANG; }
    for (;;) {
        auto res(::waitpid(id_, &status, options));
        if (res < 0) {
            if (EINTR == errno) { continue; }

            std::system_error e(errno, std::system_category());
            LOG(warn1) << "waitpid(" << id_ << ") failed: <" << e.code()
                       << ", " << e.what() << ">";
            throw e;
        }

        if (!res) {
            // process still running -> Alive
            throw Alive{};
        }
        break;
    }

    LOG(info1) << "Joined process " << id_ << ", status: " << status << ".";

    // reset ID
    id_ = 0;

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    // TODO: handle signals
    return EXIT_FAILURE;
}

void Process::terminate(Id id)
{
    auto res(::kill(id, SIGTERM));

    if (res < 0) {
        std::system_error e(errno, std::system_category());
        LOG(warn1) << "kill(" << id << "SIGTERM) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    }
}

void Process::kill(Id id)
{
    auto res(::kill(id, SIGKILL));

    if (res < 0) {
        std::system_error e(errno, std::system_category());
        LOG(warn1) << "kill(" << id << ", SIGKILL) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    }
}

void Process::signal(Id id, int signo)
{
    auto res(::kill(id, signo));

    if (res < 0) {
        std::system_error e(errno, std::system_category());
        LOG(warn1) << "kill(" << id << ", " << signo
                   << ") failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    }
}

void Process::terminate()
{
    if (!joinable()) {
        std::system_error e(EINVAL, std::system_category());
        LOG(err3) << "Cannot join non-joinable process.";
        throw e;
    }

    terminate(id_);
    killed_ = true;
}

void Process::kill()
{
    if (!joinable()) {
        std::system_error e(EINVAL, std::system_category());
        LOG(err3) << "Cannot join non-joinable process.";
        throw e;
    }

    kill(id_);
    killed_ = true;
}


void Process::detach()
{
    if (!joinable()) {
        std::system_error e(EINVAL, std::system_category());
        LOG(err3) << "Cannot join non-joinable process.";
        throw e;
    }
    id_ = 0;
}

Process::Id Process::run(const std::function<int()> &func
                         , const Flags &flags)
{
    int pflags(SpawnFlag::none);
    if (flags.quickExit()) {
        pflags |= SpawnFlag::quickExit;
    }

    return spawn(func, pflags);
}

Process::Id ThisProcess::id()
{
    return ::getpid();
}

Process::Id ThisProcess::parentId()
{
    return ::getppid();
}

} // namespace utility
