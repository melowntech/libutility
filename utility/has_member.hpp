/**
 *  @file utility/has_member.hpp
 *  @author Vaclav Blazek <vaclav.blazek@ext.citationtech.net>
 *
 *  Adds template class has_XXX that can be used to distinguish types that have
 *  data member names XXX.
 */

#ifndef shared_has_memeber_map_hpp_included_
#define shared_has_memeber_map_hpp_included_

#define UTILITY_HAS_MEMBER(MEMBER)                                              \
template<typename T> class has_##MEMBER {                               \
    struct Fallback { int MEMBER; };                                    \
    struct Derived : T, Fallback {};                                    \
    template<typename U, U> struct Check;                               \
    typedef char ArrayOfOne[1];                                         \
    typedef char ArrayOfTwo[2];                                         \
    template<typename U>                                                \
    static ArrayOfOne& func(Check<int Fallback::*, &U::MEMBER> *);      \
    template<typename U> static ArrayOfTwo& func(...);                  \
public:                                                                 \
    typedef has_##MEMBER type;                                          \
    static const bool value = (sizeof(func<Derived>(0)) == 2);          \
}

#endif // shared_has_memeber_map_hpp_included_
