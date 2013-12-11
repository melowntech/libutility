#ifndef utility_binaryio_hpp_included_
#define utility_binaryio_hpp_included_

#include <iostream>

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

inline void read(std::istream &os, char *v, size_t count) {
    os.read(v, count);
}

template <typename T>
void read(std::istream &os, T *v, size_t count)
{
    os.read(reinterpret_cast<char*>(v), count * sizeof(T));
}

template<typename T, int size>
void read(std::istream &os, T(&v)[size]) {
    os.read(reinterpret_cast<char*>(v), size * sizeof(T));
}

template <typename T>
void read(std::istream &os, T &v)
{
    os.read(reinterpret_cast<char*>(&v), sizeof(T));
}

} } // namespace utility::binaryio

#endif // utility_binaryio_hpp_included_
