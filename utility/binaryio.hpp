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
#ifndef utility_binaryio_hpp_included_
#define utility_binaryio_hpp_included_

#include <iostream>
#include <array>
#include <vector>

namespace utility { namespace binaryio {

inline void write(std::ostream &os, const char *v, size_t count) {
    os.write(v, count);
}

template <typename T>
void write(std::ostream &os, const T *v, size_t count)
{
    os.write(reinterpret_cast<const char*>(v), count * sizeof(T));
}

template <typename T, int size>
void write(std::ostream &os, const T(&v)[size]) {
    os.write(reinterpret_cast<const char*>(v), size * sizeof(T));
}

template <typename T, int size>
void write(std::ostream &os, const std::array<T, size> &v) {
    os.write(reinterpret_cast<const char*>(v.data()), size * sizeof(T));
}

template <typename T>
void write(std::ostream &os, const T &v)
{
    os.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

template <typename T, typename Allocator>
void write(std::ostream &os, const std::vector<T, Allocator> &v) {
    write(os, v.data(), v.size());
}

inline void read(std::istream &is, char *v, size_t count) {
    is.read(v, count);
}

template <typename T>
void read(std::istream &is, T *v, size_t count)
{
    is.read(reinterpret_cast<char*>(v), count * sizeof(T));
}

template<typename T, int size>
void read(std::istream &is, T(&v)[size]) {
    is.read(reinterpret_cast<char*>(v), size * sizeof(T));
}

template<typename T, int size>
void read(std::istream &is, std::array<T, size> &v) {
    is.read(reinterpret_cast<char*>(v.data()), size * sizeof(T));
}

template <typename T>
void read(std::istream &is, T &v)
{
    is.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T, typename Allocator>
void read(std::istream &is, std::vector<T, Allocator> &v) {
    read(is, v.data(), v.size());
}

template <typename T>
T read(std::istream &is)
{
    T v;
    is.read(reinterpret_cast<char*>(&v), sizeof(T));
    return v;
}

} } // namespace utility::binaryio

#endif // utility_binaryio_hpp_included_
