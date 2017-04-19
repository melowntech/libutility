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
 *  @file utility/has_member.hpp
 *  @author Vaclav Blazek <vaclav.blazek@citationtech.net>
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
