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
 * @file utility/enum-io.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Enum IO support. Generates operator<<(ostream, t) and operator<<(istream, T)
 * based on supplied mapping.
 *
 * Enum value <-> string mapping uses Boost.Preprocessor sequences. Mapping is
 * sequence of 1- or 2-element sequences, one for each enum value.
 *
 * To generate I/O stream operators for your enum class EnumType { a, b, c }
 * write:
 *
 * UTILITY_GENERATE_ENUM_IO(EnumType,
 *                          ((a))
 *                          ((b))
 *                          ((c))
 *                          )
 *
 * NB: notice no semicolon at the end of macro call!
 *
 * If string representation is different than C++ identifier add second element
 * to mapping tupple, e.g. for ImageOrientation::left_top represented as
 * "left-top" write: ((left_top)("left-top"))
 *
 * These support functions are also generated:
 *     std::array<Type, number-of-enumerations> enumerationValues(Type)
 *     const char* enumerationString(Type)
 */

#ifndef utility_enum_io_hpp_included_
#define utility_enum_io_hpp_included_

#include <array>
#include <iostream>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "gccversion.hpp"

#define UTILITY_DETAIL_fromEnum_element1(r,Type,value)              \
    case Type::BOOST_PP_SEQ_ELEM(0, value):                         \
    return os << BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, value));

#define UTILITY_DETAIL_fromEnum_element2(r,Type,value)   \
    case Type::BOOST_PP_SEQ_ELEM(0, value):              \
    return os << BOOST_PP_SEQ_ELEM(1, value);

#define UTILITY_DETAIL_fromEnum_element3(r,Type,value)   \
    UTILITY_DETAIL_fromEnum_element2(r,Type,value)

#define UTILITY_DETAIL_fromEnum_element4(r,Type,value)  \
    UTILITY_DETAIL_fromEnum_element2(r,Type,value)

#define UTILITY_DETAIL_fromEnum_element5(r,Type,value)  \
    UTILITY_DETAIL_fromEnum_element2(r,Type,value)

#define UTILITY_DETAIL_fromEnum_element6(r,Type,value)  \
    UTILITY_DETAIL_fromEnum_element2(r,Type,value)

#define UTILITY_DETAIL_fromEnum_element7(r,Type,value)  \
    UTILITY_DETAIL_fromEnum_element2(r,Type,value)

#define UTILITY_DETAIL_fromEnum_element(r,type,value)                   \
    BOOST_PP_CAT(UTILITY_DETAIL_fromEnum_element,BOOST_PP_SEQ_SIZE(value)) \
    (r,type,value)

#define UTILITY_DETAIL_toEnum_element1(r,Type,value)                \
    if (compare(s, BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, value)))) {  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element2(r,Type,value)                \
    if (compare(s, BOOST_PP_SEQ_ELEM(1, value))) {                  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element3(r,Type,value)                \
    UTILITY_DETAIL_toEnum_element2(r,Type,value)                    \
    if (compare(s, BOOST_PP_SEQ_ELEM(2, value))) {                  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element4(r,Type,value)                \
    UTILITY_DETAIL_toEnum_element3(r,Type,value)                    \
    if (compare(s, BOOST_PP_SEQ_ELEM(3, value))) {                  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element5(r,Type,value)                \
    UTILITY_DETAIL_toEnum_element4(r,Type,value)                    \
    if (compare(s, BOOST_PP_SEQ_ELEM(4, value))) {                  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element6(r,Type,value)                \
    UTILITY_DETAIL_toEnum_element5(r,Type,value)                    \
    if (compare(s, BOOST_PP_SEQ_ELEM(5, value))) {                  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element7(r,Type,value)                \
    UTILITY_DETAIL_toEnum_element6(r,Type,value)                    \
    if (compare(s, BOOST_PP_SEQ_ELEM(6, value))) {                  \
        out = Type::BOOST_PP_SEQ_ELEM(0, value);                    \
        return is;                                                  \
    }

#define UTILITY_DETAIL_toEnum_element(r,type,value)                     \
    BOOST_PP_CAT(UTILITY_DETAIL_toEnum_element,BOOST_PP_SEQ_SIZE(value)) \
    (r,type,value)

#define UTILITY_DETAIL_value(Type,value)                                \
    Type::BOOST_PP_SEQ_ELEM(0, value)

#define UTILITY_DETAIL_name1(Type,value)                                \
    BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, value))

#define UTILITY_DETAIL_name2(Type,value)                                \
    BOOST_PP_SEQ_ELEM(1, value)

#define UTILITY_DETAIL_name3(Type,value)                                \
    UTILITY_DETAIL_name2(Type,value)

#define UTILITY_DETAIL_name4(Type,value)                                \
    UTILITY_DETAIL_name2(Type,value)

#define UTILITY_DETAIL_name5(Type,value)                                \
    UTILITY_DETAIL_name2(Type,value)

#define UTILITY_DETAIL_name6(Type,value)                                \
    UTILITY_DETAIL_name2(Type,value)

#define UTILITY_DETAIL_name7(Type,value)                                \
    UTILITY_DETAIL_name2(Type,value)

#define UTILITY_DETAIL_name(Type,value)                                 \
    BOOST_PP_CAT(UTILITY_DETAIL_name,BOOST_PP_SEQ_SIZE(value))          \
    (Type,value)

#define UTILITY_DETAIL_comma_value(r,Type,value)                        \
    , UTILITY_DETAIL_value(Type,value)

#define UTILITY_DETAIL_comma_name(r,Type,value)                         \
    ", " UTILITY_DETAIL_name(Type,value)

#define UTILITY_DETAIL_untyped_value(Type,value)                        \
    BOOST_PP_SEQ_ELEM(0, value)

#define UTILITY_DETAIL_comma_untyped_value(r,Type,value)                \
    , UTILITY_DETAIL_untyped_value(Type,value)

#define UTILITY_DETAIL_toEnum_compare0                                  \
    auto compare([](const std::string &l, const std::string &r) {       \
            return l == r;                                              \
        });

#define UTILITY_DETAIL_toEnum_compare1                                  \
    auto compare([](const std::string &l, const std::string &r) {       \
            return boost::algorithm::iequals(l, r);                     \
        });

#define UTILITY_DETAIL_value_str(value)                                 \
    BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, value))

#define UTILITY_DETAIL_comma_value_str(r,ignore,value)                  \
    , UTILITY_DETAIL_value_str(value)

#define UTILITY_GENERATE_ENUM_IO_IMPL(Type, seq, ci)                    \
    template <typename E, typename T>                                   \
    std::basic_ostream<E, T>&                                           \
    operator<<(std::basic_ostream<E, T> &os, const Type &value)         \
        UTILITY_POSSIBLY_UNUSED;                                        \
    template <typename E, typename T>                                   \
    std::basic_ostream<E, T>&                                           \
    operator<<(std::basic_ostream<E, T> &os, const Type &value)         \
    {                                                                   \
        switch (value) {                                                \
        BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_fromEnum_element, Type, seq) \
        }                                                               \
        os.setstate(std::ios::failbit);                                 \
        return os;                                                      \
    }                                                                   \
                                                                        \
    template <typename E, typename T>                                   \
    std::basic_istream<E, T>&                                           \
    operator>>(std::basic_istream<E, T> &is, Type &out)                 \
        UTILITY_POSSIBLY_UNUSED;                                        \
    template <typename E, typename T>                                   \
    std::basic_istream<E, T>&                                           \
    operator>>(std::basic_istream<E, T> &is, Type &out)                 \
    {                                                                   \
        std::string s;                                                  \
        is >> s;                                                        \
        BOOST_PP_CAT(UTILITY_DETAIL_toEnum_compare,ci)                  \
        BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_toEnum_element, Type, seq) \
        out = {};                                                       \
        is.setstate(std::ios::failbit);                                 \
        return is;                                                      \
    }                                                                   \
    UTILITY_POSSIBLY_UNUSED constexpr int enumerationValuesCount(Type)  \
    {                                                                   \
        return BOOST_PP_SEQ_SIZE(seq);                                  \
    }                                                                   \
    std::array<Type, BOOST_PP_SEQ_SIZE(seq)>                            \
    enumerationValues(Type) UTILITY_POSSIBLY_UNUSED;                    \
    inline std::array<Type, BOOST_PP_SEQ_SIZE(seq)>                     \
    enumerationValues(Type)                                             \
    {                                                                   \
        return {{                                                       \
            UTILITY_DETAIL_value(Type, BOOST_PP_SEQ_HEAD(seq))          \
            BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_comma_value            \
                                  , Type, BOOST_PP_SEQ_TAIL(seq))       \
        }};                                                             \
    }                                                                   \
                                                                        \
    std::array<const char*, BOOST_PP_SEQ_SIZE(seq)>                     \
    enumerationValuesStringified(Type) UTILITY_POSSIBLY_UNUSED;         \
    inline std::array<const char*, BOOST_PP_SEQ_SIZE(seq)>              \
    enumerationValuesStringified(Type)                                  \
    {                                                                   \
        return {{                                                       \
            UTILITY_DETAIL_value_str(BOOST_PP_SEQ_HEAD(seq))            \
            BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_comma_value_str        \
                                  , _, BOOST_PP_SEQ_TAIL(seq))          \
        }};                                                             \
    }                                                                   \
                                                                        \
    const char* enumerationString(Type) UTILITY_POSSIBLY_UNUSED;        \
    inline const char* enumerationString(Type)                          \
    {                                                                   \
        return UTILITY_DETAIL_name(Type, BOOST_PP_SEQ_HEAD(seq))        \
            BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_comma_name, Type       \
                                  , BOOST_PP_SEQ_TAIL(seq));            \
    }

/** Generate I/O code for any enum (stream operator<< and operator>>).
 */
#define UTILITY_GENERATE_ENUM_IO(Type, seq)                             \
    UTILITY_GENERATE_ENUM_IO_IMPL(Type, seq, 0)

/** Case-insensitive equivalent of UTILITY_GENERATE_ENUM_IO.
 */
#define UTILITY_GENERATE_ENUM_IO_CI(Type, seq)                          \
    UTILITY_GENERATE_ENUM_IO_IMPL(Type, seq, 1)

/** Generate enum class and I/O stuff at one go
 */
#define UTILITY_GENERATE_ENUM(Type, seq)                                \
    enum class Type {                                                   \
        UTILITY_DETAIL_untyped_value(Type, BOOST_PP_SEQ_HEAD(seq))      \
        BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_comma_untyped_value        \
                              , Type, BOOST_PP_SEQ_TAIL(seq))           \
    };                                                                  \
                                                                        \
    UTILITY_GENERATE_ENUM_IO(Type, seq)

/** Generate enum class and I/O stuff at one go, case insensitive version
 */
#define UTILITY_GENERATE_ENUM_CI(Type, seq)                             \
    enum class Type {                                                   \
        UTILITY_DETAIL_untyped_value(Type, BOOST_PP_SEQ_HEAD(seq))      \
        BOOST_PP_SEQ_FOR_EACH(UTILITY_DETAIL_comma_untyped_value        \
                              , Type, BOOST_PP_SEQ_TAIL(seq))           \
    };                                                                  \
                                                                        \
    UTILITY_GENERATE_ENUM_IO_CI(Type, seq)

#endif // utility_enum_io_hpp_included_
