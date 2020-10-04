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
 * @file detail/streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_detail_streams_hpp_included_
#define utility_detail_streams_hpp_included_

#include <cctype>
#include <cstddef>
#include <array>
#include <istream>
#include <ostream>
#include <fstream>
#include <algorithm>

#include <boost/optional.hpp>

namespace utility { namespace detail {

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
resetFailOnEof(std::basic_istream<CharT, Traits> &is)
{
    if (is.eof()) {
        is.clear(is.rdstate() & ~std::ios::failbit);
    }
    return is;
}

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT> struct Expect { CharT c; };

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, const Expect<CharT> &ce)
{
    bool skipws(is.flags() & std::ios::skipws);
    for (;;) {
        auto c(is.get());
        if (c == ce.c) {
            // matched
            return is;
        }

        if (skipws && std::isspace(c)) {
            // skip ws
            continue;
        }
        // failed
        is.setstate(std::ios::failbit);

        break;
    }
    return is;
}

template<typename CharT> struct Match {
    CharT c;
    bool matched;

    Match(CharT c) : c(c), matched(false) {}
};

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Match<CharT> &ce)
{
    ce.matched = false;

    // check for stream validity and eat whitespaces
    std::istream::sentry s(is);
    if (!s) {
        return resetFailOnEof(is);
    };

    auto c(is.get());
    if (is.eof()) {
        return resetFailOnEof(is);
    };

    if (c == ce.c) {
        // matched
        ce.matched = true;
        return is;
    }

    // not found; return back what was read
    is.unget();
    return is;
}

template<typename T, std::size_t size>
struct ArrayPrinter {
    const T *data;
    ArrayPrinter(const T *data, const std::string &separator)
        : data(data), separator(separator)
    {}

    friend std::ostream&
    operator<<(std::ostream &os, const ArrayPrinter<T, size> &a)
    {
        bool first(true);
        std::for_each(a.data, a.data + size
                      , [&](const T &d) -> void {
                          if (first) { first = false; }
                          else { os << a.separator; }
                          os << d;
                      });

        return os;
    }

    const std::string separator;
};

template<typename Container>
struct ContainerPrinter {
    const Container &container;
    ContainerPrinter(const Container &container
                     , const std::string separator = "")
        : container(container), separator(separator)
    {}

    friend std::ostream&
    operator<<(std::ostream &os, const ContainerPrinter<Container> &c)
    {
        bool first(true);
        std::for_each(c.container.begin(), c.container.end()
                      , [&](const typename Container::value_type &d) -> void {
                          if (first) { first = false; }
                          else { os << c.separator; }
                          os << d;
                      });

        return os;
    }

    const std::string separator;
};

/** Dumpable is a class having this member function:
 *  template <typename E, typename T>
 *  std::basic_ostream<E, T>&
 *  dump( std::basic_ostream<E, T> &os, const std::string & prefix) const;
 */
template <typename Dumpable>
struct Dumper {
    Dumper(const Dumpable &dumpable, const std::string &prefix)
        : dumpable(dumpable), prefix(prefix)
    {}

    const Dumpable &dumpable;
    const std::string &prefix;

private:
    Dumpable& operator=(Dumpable&); // assignment operator deleted
};

template <typename E, typename T, typename Dumpable>
std::basic_ostream<E, T>&
dump(std::basic_ostream<E,T> &os, const Dumpable &dumpable
     , const std::string &prefix = std::string())
{
    return dumpable.dump(os, prefix);
}

template <typename E, typename T, typename Dumpable>
std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E,T> &os, const Dumper<Dumpable> &dumper)
{
    // return dumper.dumpable.dump(os, dumper.prefix);
    return dump(os, dumper.dumpable, dumper.prefix);
}

template <typename Container>
struct Join {
    const Container &c;
    const std::string &sep;
    const std::string &dflt;
};

template <typename E, typename T, typename Container>
inline std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E, T> &os, const Join<Container> &j)
{
    using std::cbegin;
    using std::cend;
    if (cbegin(j.c) == cend(j.c)) {
        // empty -> use default
        if (!j.dflt.empty()) { return os << j.dflt; }
        return os;
    }
    bool first = true;
    for (const auto &e : j.c) {
        if (!first) {
            os << j.sep;
        }
        os << e;
        first = false;
    }
    return os;
}

namespace first_valid {

template <typename ...Args>
struct Writer {
    Writer(Args &&...args)
        : args(std::forward<Args>(args)...)
    {}

    const std::tuple<Args...> args;

private:
    Writer& operator=(Writer&) = delete;
};

template<std::size_t> struct int_{};

template <typename E, typename T, typename V>
bool tryPrint(std::basic_ostream<E, T> &os, const V &t)
{
    os << t;
    return true;
}

template <typename E, typename T, typename V>
bool tryPrint(std::basic_ostream<E, T> &os, const boost::optional<V> &t)
{
    if (t) {
        os << *t;
        return true;
    }
    return false;
}

template <typename E, typename T>
bool tryPrint(std::basic_ostream<E, T> &os, const char *str)
{
    if (str) {
        os << str;
        return true;
    }
    return false;
}

template <typename E, typename T, typename V>
bool tryPrint(std::basic_ostream<E, T> &os, const V *t)
{
    if (t) {
        os << *t;
        return true;
    }
    return false;
}

template <typename E, typename T, typename Tuple, std::size_t I>
bool printFirst(std::basic_ostream<E, T> &os, const Tuple &t, int_<I>)
{
    if (printFirst(os, t, int_<I - 1>())) {
        return true;
    }
    return tryPrint(os, std::get<I>(t));
}

template <typename E, typename T, typename Tuple>
bool printFirst(std::basic_ostream<E, T> &os, const Tuple &t, int_<0>)
{
    return tryPrint(os, std::get<0>(t));
}

template <typename E, typename T, typename ...Args>
std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E, T> &os
           , const Writer<Args...> &fvw)
{
    printFirst(os, fvw.args, int_<sizeof...(Args) - 1>());
    return os;
}

} // namespace first_valid


// terminator
inline void concat(std::ostream&) {}

template <typename T, typename ...Args>
void concat(std::ostream &os, T &&head, Args &&...args)
{
    os << head;
    concat(os, std::forward<Args>(args)...);
}

// terminators
inline void concatWithSeparator(std::ostream&, const std::string&) {}
inline void concatWithSeparatorOther(std::ostream&, const std::string&) {}

template <typename T, typename ...Args>
void concatWithSeparatorOther(std::ostream &os, const std::string &sep
                              , T &&head, Args &&...args)
{
    os << sep << head;
    concatWithSeparatorOther(os, sep, std::forward<Args>(args)...);
}

template <typename T, typename ...Args>
void concatWithSeparator(std::ostream &os, const std::string &sep
                         , T &&head, Args &&...args)
{
    os << head;
    concatWithSeparatorOther(os, sep, std::forward<Args>(args)...);
}


} } // namespace utility::detail

#endif // utility_detail_streams_hpp_included_
