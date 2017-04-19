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

#if GCC_VERSION && (GCC_VERSION < 40700) && !defined(__clang__)
#    define UTILITY_HAS_NO_STD_FUTURE_STATUS
#endif

// pre-4.7 gcc has no override/final modifier
#if GCC_VERSION && (GCC_VERSION < 40700)
#    define UTILITY_OVERRIDE
#    define UTILITY_FINAL
#else
#    define UTILITY_OVERRIDE override
#    define UTILITY_FINAL final
#endif

#ifdef __GNUC__
#define UTILITY_FUNCTION_ERROR(message) \
    __attribute__((error(message)))
#else
#define UTILITY_FUNCTION_ERROR(message) \
    ; static_assert(false, "error message allowed only in GCC")
#endif

// pre-4.8 gcc has no thread_local storage specifier but uses __thread extension
#if GCC_VERSION && (GCC_VERSION < 40800)
#    define UTILITY_THREAD_LOCAL __thread
#else
#    define UTILITY_THREAD_LOCAL thread_local
#endif

#if GCC_VERSION
#    define UTILITY_INIT_PRIORITY(PRIORITY) \
    __attribute__ ((init_priority (PRIORITY)))
#else
#    define UTILITY_INIT_PRIORITY(PRIORITY)
#endif

#endif // SHARED_UTILITY_GCCVERSION_HPP_INCLUDED_
