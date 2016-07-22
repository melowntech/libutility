/**
 * @file utility/typeinfo.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_typeinfo_hpp_included_
#define utility_typeinfo_hpp_included_

#include <string>
#include <typeinfo>

namespace utility {

template <typename T> std::string typeName();

template <typename T> std::string typeName(const T &value);

std::string typeName(const std::type_info &info);

std::string demangleTypeName(const char *name);

// inlines

inline std::string typeName(const std::type_info &info) {
    return demangleTypeName(info.name());
}

template <typename T> std::string typeName() {
    return typeName(typeid(T));
}

template <typename T> inline std::string typeName(const T &value) {
    return typeName(typeid(value));
}

} // namespace utility

#endif // utility_typeinfo_hpp_included_
