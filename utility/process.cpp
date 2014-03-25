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

#include "dbglog/dbglog.hpp"

#include "process.hpp"

namespace fs = boost::filesystem;

namespace utility { namespace detail {

void SystemContext::setFdPath(int redirectIdx, const RedirectFile::DstArg &arg
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


struct ExecArgs {
    typedef std::vector<char*> Argv;
    Argv argv;

    ~ExecArgs() {
        for (auto arg : argv) { std::free(arg); }
    }

    void arg(const std::string &arg) {
        argv.push_back(strdup(arg.c_str()));
    }

    void finish() { argv.push_back(nullptr); }
};

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits> &os, const ExecArgs &a)
{
    bool first = true;
    for (const auto arg : a.argv) {
        os << (first ? "" : " ") << arg;
        first = false;
    }
    return os;
}

constexpr int EXEC_FAILED = 255;

void useFd(int dst, int src)
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
    ::close(src);
}

void processEnv(const SystemContext::Environ &environ)
{
    for (const auto &e : environ) {
        if (e.second) {
            ::setenv(e.first.c_str(), e.second->c_str(), true);
        } else {
            ::unsetenv(e.first.c_str());
        }
    }
}

pid_t execute(const ExecArgs &argv, const boost::function<void()> &afterFork)
{
    LOG(info2) << "Executing: " << argv;

    auto pid(::fork());
    if (-1 == pid) {
        // Oops, failed
        std::system_error e(errno, std::system_category());
        LOG(warn1) << "fork(2) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    } else if (0 == pid) {
        // child -> prepare and exec

        // call afterfork callback
        afterFork();

        // exec
        if (::execvpe(argv.argv.front(), &(argv.argv.front())
                      , ::environ) == -1)
        {
            std::system_error e(errno, std::system_category());
            LOG(warn1) << "execve(2) [" << argv.argv.front()
                       << "] failed: <" << e.code() << ", "
                       << e.what() << ">";
            std::exit(EXEC_FAILED);
        }

        // never reached
    }

    return pid;
}

void redirect(const SystemContext::Redirects &redirects)
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
            if (src.out) {
                fd = ::open(src.path.string().c_str()
                            , O_WRONLY | O_CREAT | O_TRUNC
                            , (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
            } else {
                fd = ::open(src.path.string().c_str(), O_RDONLY);
            }

            if (fd == -1) {
                std::system_error e(errno, std::system_category());
                LOG(err1) << "Cannot open file " << src.path << ": <"
                          << e.code() << ", " << e.what() << ">.";
                throw e;
            }
            useFd(boost::any_cast<int>(redirect.dst), fd);
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
                if (std::uncaught_exception()) {
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

int systemImpl(const std::string &program, SystemContext ctx)
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
    detail::ExecArgs argv;
    argv.arg(program);
    for (const auto arg : ctx.argv) {
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
                     processEnv(ctx.environ);
                 } catch (const std::exception &e) {
                     std::exit(EXEC_FAILED);
                 }
             }));

    LOG(info2) << "Running under pid: " << pid << ".";

    return Pump(pid, inPipes, outPipes).run();
}

int spawnImpl(const std::function<int ()> &func)
{
    auto pid(::fork());
    if (-1 == pid) {
        // Oops, failed
        std::system_error e(errno, std::system_category());
        LOG(warn1) << "fork(2) failed: <" << e.code()
                   << ", " << e.what() << ">";
        throw e;
    } else if (0 == pid) {
        std::exit(func());
    }

    return pid;
}

} } // namespace utility::detail

