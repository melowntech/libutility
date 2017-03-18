#ifndef utility_binaryio_hpp_included_
#define utility_binaryio_hpp_included_

#include <iostream>
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

template<typename T, int size>
void write(std::ostream &os, const T(&v)[size]) {
    os.write(reinterpret_cast<const char*>(v), size * sizeof(T));
}

template <typename T>
void write(std::ostream &os, const T &v)
{
    os.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

template <typename T>
void write(std::ostream &os, const std::vector<T> &v) {
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

template <typename T>
void read(std::istream &is, T &v)
{
    is.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template <typename T>
void read(std::istream &is, std::vector<T> &v) {
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
