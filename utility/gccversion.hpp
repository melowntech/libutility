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
#ifndef SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_
#define SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_

#ifdef __GNUC__
#    define GCC_VERSION \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#    define GCC_VERSION 0
#endif

#if defined(__GNUC__) && ! defined(__clang__)
#   define UTILITY_FUNCTION_ERROR(message) \
        __attribute__((error(message)))
#elif defined(__clang__)
#   define UTILITY_FUNCTION_ERROR(message)
    // TODO?
#elif defined(_WIN32)
#   define UTILITY_FUNCTION_ERROR(message)
    // TODO?
#else
#   define UTILITY_FUNCTION_ERROR(message) \
        ; static_assert(false, "error message allowed only in GCC")
#endif

#if defined(__GNUC__) && ! defined(__clang__)
#    define UTILITY_INIT_PRIORITY(PRIORITY) \
    __attribute__ ((init_priority (PRIORITY)))
#else
#    define UTILITY_INIT_PRIORITY(PRIORITY)
#endif

#if defined(__GNUC__) && ! defined(__clang__)
#    define UTILITY_POSSIBLY_UNUSED             \
    __attribute__ ((unused))
#elif __clang__
#    define UTILITY_POSSIBLY_UNUSED             \
    __attribute__ ((unused))
#else
// TODO: implement for other compilers
#    define UTILITY_POSSIBLY_UNUSED
#endif

#if defined(__GNUC__) && ! defined(__clang__)
#    define UTILITY_POSSIBLY_USED             \
    __attribute__ ((used))
#elif __clang__
#    define UTILITY_POSSIBLY_USED             \
    __attribute__ ((used))
#else
// TODO: implement for other compilers
#    define UTILITY_POSSIBLY_USED
#endif

#if defined(__GNUC__) && ! defined(__clang__)
#    define UTILITY_SECTION(SECTION)            \
    __attribute__ ((section(SECTION)))
#elif __clang__
#    define UTILITY_SECTION(SECTION)            \
    __attribute__ ((section(SECTION)))
#else
// TODO: implement for other compilers
#    define UTILITY_SECTION(section)
#endif

#if __cplusplus >= 201703L
#    define UTILITY_FALLTHROUGH      [[fallthrough]]
#elif defined(__GNUC__)
#    if defined(__clang__)
#        define UTILITY_FALLTHROUGH  [[clang::fallthrough]]
#    elif (GCC_VERSION >= 70000)
#        define UTILITY_FALLTHROUGH  __attribute__((fallthrough))
#    else
#        define UTILITY_FALLTHROUGH
#    endif
#else
#    define UTILITY_FALLTHROUGH
#endif

#if __cplusplus >= 201703L
#    define UTILITY_MAYBE_UNUSED [[maybe_unused]]
#else
#    define UTILITY_MAYBE_UNUSED
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define UTILITY_VISIBILITY(VALUE)            \
    __attribute__ ((visibility(#VALUE)))
#else
// TODO: implement for other compilers
#  define UTILITY_VISIBILITY(VALUE)
#endif

#endif // SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_
