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
 * @file openmp.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Support for openmp.
 */

#ifndef utility_openmp_hpp_included_
#define utility_openmp_hpp_included_

#include <algorithm>

/** Include OpenMP runtime header if enabled.
 */
#ifdef _OPENMP
# include <omp.h>
#endif

/** Only if OpenMP is enabled then this macro expands argument to
 *  pragma. Otherwise, nothing is expanded at all. There must be at least one
 *  argument. Macro arguments must contains what would normally be written after
 *  "#pragma omp" declaration.
 *
 *  Example:
 *          UTILITY_OMP(critical)
 /      expands to
 *          _Pragma("omp critical")
 *      which preprocessor replaces with
 *          #pragma omp critical
 *      that is seen by compiler
 */
#ifdef _OPENMP
#  define UTILITY_OMP(...) UTILITY_OMP_(omp __VA_ARGS__)
#else
#  define UTILITY_OMP(...)
#endif
#ifdef _MSC_VER
#define UTILITY_OMP_(...) __pragma(__VA_ARGS__)
#else
#define UTILITY_OMP_(...) _Pragma(#__VA_ARGS__)
#endif

/** Values of the _OPENMP macro for different OpenMP versions
 *  (adapted from https://stackoverflow.com/a/38309448/6847659)
 */
#define OMP_VERSION_2_0 200203
#define OMP_VERSION_2_5 200505
#define OMP_VERSION_3_0 200805
#define OMP_VERSION_3_5 201107
#define OMP_VERSION_4_0 201307
#define OMP_VERSION_4_5 201511

/** Define extra functions if compiling with OpenMP
 */
#ifndef _OPENMP
inline int omp_get_max_threads() { return 1; }
inline int omp_get_num_threads() { return 1; }
inline int omp_get_thread_num() { return 0; }
inline int omp_get_nested() { return 0; }
inline void omp_set_nested(int) {}

using omp_lock_t = int;
inline void omp_set_lock(omp_lock_t *) {}
inline void omp_unset_lock(omp_lock_t *) {}
inline void omp_init_lock(omp_lock_t *) {}
inline void omp_destroy_lock(omp_lock_t *) {}
#endif

namespace utility {

inline int capThreadCount(int limit) {
    return std::min(omp_get_max_threads(), limit);
}

class ScopedNestedParallelism {
public:
    ScopedNestedParallelism(bool newValue)
        : oldValue_(omp_get_nested())
    {
        omp_set_nested(newValue);
    }

    ~ScopedNestedParallelism() { omp_set_nested(oldValue_); }

private:
    bool oldValue_;
};

} // namespace utility

#endif // utility_openmp_hpp_included_
