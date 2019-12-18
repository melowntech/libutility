/**
 * Copyright (c) 2019 Melown Technologies SE
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
 * @file assert.hpp
 * @author Pavel Sevecek <pavel.sevecek@melown.com>
 *
 * Custom assertions, intended for checking internal consistency of the code.
 */

#ifndef utility_assert_hpp_included_
#define utility_assert_hpp_included_

namespace utility {
namespace detail {
    void doAssert(const char* message, const char* file, const char* func, const int line, const char* param);
}

} // namespace utility

#ifdef NDEBUG
#define MLWN_DISABLE_ASSERTS
#endif

#ifdef MLWN_DISABLE_ASSERTS
// sizeof prevents the "unused variable" warning, but does not evaluate the expression
#define MLWN_ASSERT(x) (void)(sizeof(x))
#define MLWN_ASSERT1(x, p)                                                                                   \
    (void)(sizeof(x));                                                                                       \
    (void)(sizeof(p))
#else
/** Evaluates the boolean expression and reports an assertion message to standard output in case it
 * returns false.
 */
#define MLWN_ASSERT(x)                                                                                       \
    if (!bool(x)) {                                                                                          \
        ::utility::detail::doAssert(#x, __FILE__, __FUNCTION__, __LINE__, nullptr);                          \
    }
/** Similar to MLWN_ASSERT, but allows to print a user-specified parameter in the message.
 */
#define MLWN_ASSERT1(x, p)                                                                                   \
    if (!bool(x)) {                                                                                          \
        ::utility::detail::doAssert(#x, __FILE__, __FUNCTION__, __LINE__, std::to_string(p).c_str());        \
    }
#endif

#endif // utility_assert_hpp_included_
