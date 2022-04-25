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

/** Only ifg OpenMP is enabled then this macro expands argument to
 *  praga. Otherwise, nothing is expanded at all. There must be at least one
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
#define UTILITY_OMP_(...) _Pragma(#__VA_ARGS__)

/** Define extra functions if compiling with OpenMP
 */
#ifndef _OPENMP
inline int omp_get_max_threads() { return 1; }
inline int omp_get_num_threads() { return 1; }
inline int omp_get_thread_num() { return 0; }
inline int omp_get_nested() { return 0; }
inline void omp_set_nested(int) {}
#endif

namespace utility {

inline int capThreadCount(int limit) {
    return std::min(omp_get_max_threads(), limit);
}

class ScopedNestedParalellism {
public:
    ScopedNestedParalellism(bool newValue)
        : oldValue_(omp_get_nested())
    {
        omp_set_nested(newValue);
    }

    ~ScopedNestedParalellism() { omp_set_nested(oldValue_); }

private:
    bool oldValue_;
};

} // namespace utility

#endif // utility_openmp_hpp_included_
