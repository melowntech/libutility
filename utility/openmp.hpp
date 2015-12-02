/**
 * @file openmp.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Support for openmp.
 */

#ifndef utility_openmp_hpp_included_
#define utility_openmp_hpp_included_

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
inline int omp_get_num_threads() { return 1; }
inline int omp_get_thread_num() { return 0; }
#endif

#endif // utility_openmp_hpp_included_
