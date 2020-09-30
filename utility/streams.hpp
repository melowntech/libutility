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
/**
 * @file streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_streams_hpp_included_
#define utility_streams_hpp_included_

#include <string>
#include <sstream>

#include <boost/filesystem.hpp>

#include "detail/streams.hpp"

/** This module adds support for C++ iostream.
 */
namespace utility {

using detail::resetFailOnEof;

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT>
inline detail::Expect<CharT> expect(CharT c) {
    return {c};
}

/** Similar to expect(c) but returns char back on mismatch.
 */
template<typename CharT>
inline detail::Match<CharT> match(CharT c) {
    return {c};
}

template<typename T, std::size_t size>
detail::ArrayPrinter<T, size>
arrayPrinter(const T(&data)[size], const std::string &separator = ", ")
{
    return {data, separator};
}

template<typename T, std::size_t size>
detail::ArrayPrinter<T, size>
arrayPrinter(const std::array<T, size> &array
             , const std::string &separator = ", ")
{
    return {&array[0], separator};
}

template <typename Dumpable>
detail::Dumper<Dumpable> dump(const Dumpable &dumpable
                              , const std::string &prefix = "")
{
    return {dumpable, prefix};
}

struct LManip {
    typedef std::function<void (std::ostream&)> type;
    const type &l;
    LManip(const type &l) : l(l) {}
};

inline std::ostream& operator<<(std::ostream &os, const LManip &l)
{
    l.l(os);
    return os;
}

/** Join container (range) content using provided separator. Container is any
 * range (i.e. whatever object that provides std::begin() and std::end(). Value
 * must have stream output operator.
 *
 * NB: since all values are held by reference output object is usable only for
 * immediate write to output stream.
 *
 * Usage example:
 *     std::vector<int> numbers{ 1, 2, 3 };
 *     LOG(info4) << utility::join(numbers, " -> ");
 * Outpur:
 *     1 -> 2 -> 3
 *
 * String representation can be obtained using boost::lexical_cast:
 *     auto string(boost::lexical_cast<std:string>(numbers));
` *
 * \param c container
 * \param sep separator
 * \param dflt default value written when range (container) is empty
 */
template <typename Container>
detail::Join<Container> join(const Container &c, const std::string &sep
                             , const std::string &dflt = std::string())
{
    return { c, sep, dflt };
}

/** Join given values and return result as a std::string.
 */
template <typename ...Args>
inline std::string concat(Args &&...args)
{
    std::ostringstream os;
    detail::concat(os, std::forward<Args>(args)...);
    return os.str();
}

/** Join given values with separatator and return result as a std::string.
 */
template <typename ...Args>
inline std::string concatWithSeparator(const std::string &sep, Args &&...args)
{
    std::ostringstream os;
    detail::concatWithSeparator(os, sep, std::forward<Args>(args)...);
    return os.str();
}

template<typename T, int size>
inline void write(std::ostream &os, const T(&v)[size]) {
    os.write(reinterpret_cast<const char*>(v), size * sizeof(T));
}

template<typename T>
inline void write(std::ostream &os, const T *v, std::size_t size) {
    os.write(reinterpret_cast<const char*>(v), size * sizeof(T));
}

template<typename T>
inline void write(const boost::filesystem::path &file
                  , const T *v, std::size_t size)
{
    std::ofstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(file.string(), std::ios_base::out | std::ios_base::trunc);
    write(f, v, size);
    f.close();
}

template<typename T, std::size_t size>
inline void write(const boost::filesystem::path &file, const T(&v)[size])
{
    write(file, v, size);
}

inline std::string read(const boost::filesystem::path &file)
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(file.string(), std::ios_base::in);
    f.seekg(0, std::ifstream::end);
    auto size(f.tellg());
    f.seekg(0);
    std::vector<char> tmp(size);
    f.read(&tmp[0], tmp.size());
    f.close();
    return { tmp.begin(), tmp.end() };
}

template<class CharT, class Traits>
class ScopedStreamExceptions {
public:
    ScopedStreamExceptions(std::basic_ios<CharT, Traits> &ios)
        : ios_(&ios), state_(ios.exceptions())
    {}

    ScopedStreamExceptions(ScopedStreamExceptions &&other)
        : ios_(other.ios_), state_(other.state_)
    {
        other.ios_ = nullptr;
    }

    ScopedStreamExceptions& operator=(ScopedStreamExceptions &&other) = delete;

    ScopedStreamExceptions(const ScopedStreamExceptions &other) = delete;
    ScopedStreamExceptions&
    operator=(const ScopedStreamExceptions &other) = delete;

    ~ScopedStreamExceptions() {
        if (ios_) { ios_->exceptions(state_); }
    }

    std::ios_base::iostate state() const { return state_; }

private:
    std::basic_ios<CharT, Traits> *ios_;
    std::ios_base::iostate state_;
};

template<class CharT, class Traits>
ScopedStreamExceptions<CharT, Traits>
scopedStreamExceptions(std::basic_ios<CharT, Traits> &ios)
{
    return { ios };
}

struct StreamState {
    template<class CharT, class Traits>
    StreamState(std::basic_ios<CharT, Traits> &ios)
        : state(ios.rdstate()) {}

    std::ios_base::iostate state;
};

template <typename E, typename T>
std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E, T> &os, const StreamState &ss)
{
    return
        os << ((ss.state & std::ios_base::goodbit) ? 'G' : 'g')
           << ((ss.state & std::ios_base::badbit) ? 'B' : 'b')
           << ((ss.state & std::ios_base::failbit) ? 'F' : 'f')
           << ((ss.state & std::ios_base::eofbit) ? 'E' : 'e')
        ;
}

// helper classes for stream I/O with increased buffer size

namespace detail {
    /* This class holds the buffer used iin ofstreambuf/istreambuf. Buffer
     * cannot be destroyed before std::ofstream/std::istream dtor (since
     * stream's dtor flushes unwritten data) therefore it cannot be direct
     * member of ofstreambuf/ifstreambuf class. Plugging this small helper class
     * before actual stream keeps buffer alive during its dtor.
     */
    struct BufHolder {
        BufHolder(std::streamsize size) : size(size), buffer(new char[size]) {}
        ~BufHolder() { delete [] buffer; }
        std::streamsize size;
        char* buffer;
    };
} // namespace detail

class ofstreambuf
    : private detail::BufHolder
    , public std::ofstream
{
public:
    static constexpr std::streamsize DefaultBufSize = 1024*1024;

    ofstreambuf(std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ofstream()
    {
        init();
    }

    explicit
    ofstreambuf(const char* filename,
                std::ios_base::openmode mode =
                    std::ios_base::out | std::ios_base::trunc
                    | std::ios_base::binary,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ofstream()
    {
        init();
        open(filename, mode);
    }

    explicit
    ofstreambuf(const std::string& filename,
                std::ios_base::openmode mode =
                    std::ios_base::out | std::ios_base::trunc
                    | std::ios_base::binary,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ofstream()
    {
        init();
        open(filename, mode);
    }

private:
    void init() {
        exceptions(std::ios::badbit | std::ios::failbit);
        rdbuf()->pubsetbuf(detail::BufHolder::buffer
                           , detail::BufHolder::size);
    }
};

class ifstreambuf
    : private detail::BufHolder
    , public std::ifstream
{
public:
    static constexpr std::streamsize DefaultBufSize = 1024*1024;

    ifstreambuf(std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ifstream()
    {
        init();
    }

    explicit
    ifstreambuf(const char* filename,
                std::ios_base::openmode mode =
                    std::ios_base::in
                    | std::ios_base::binary,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ifstream()
    {
        init();
        open(filename, mode);
    }

    explicit
    ifstreambuf(const std::string& filename,
                std::ios_base::openmode mode =
                    std::ios_base::in
                    | std::ios_base::binary,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ifstream()
    {
        init();
        open(filename, mode);
    }

private:
    void init() {
        exceptions(std::ios::badbit | std::ios::failbit);
        rdbuf()->pubsetbuf(detail::BufHolder::buffer
                           , detail::BufHolder::size);
    }
};

template<typename Container>
detail::ContainerPrinter<Container>
print(const Container &container
      , const std::string &separator = " ")
{
    return {container, separator};
}

template <typename ...Args>
detail::first_valid::Writer<Args...> printFirst(Args &&...args)
{
    return detail::first_valid::Writer<Args...>
        (std::forward<Args>(args)...);
}

} // namespace utility

#endif // utility_streams_hpp_included_
