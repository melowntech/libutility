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

/** Onky ifg OpenMP is enabled then this macro expands argument to
 *  praga. Otherwise, nothing is expanded at all. Argument must be a signle
 *  string literal that contains what would normally be written after #pragma
 *  keyword.
 *
 *  Example:
 *          UTILITY_OMP("omp critical")
 /      expadns to
 *          _Pragma("omp critical")
 *      which preprocessor replaces with
 *          #pragma omp critical
 *
 *  NB: name ends with _S to indicate string literal version
 *
 *  TODO: use preprocessor magic to write non-string arugment (we have to handle
 *        variadic macro properly since comma separates arguments) under name
 *        UTILITY_OMP
 */
#ifdef _OPENMP
#define UTILITY_OMP_S(STRING_CLAUSE) _Pragma(STRING_CLAUSE)
#else
#define UTILITY_OMP_S(STRING_CLAUSE)
#endif

#endif // utility_openmp_hpp_included_
