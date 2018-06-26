/**
 * Copyright (c) 2018 Melown Technologies SE
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

#ifndef utility_httpquery_hpp_included_
#define utility_httpquery_hpp_included_

#include <utility>
#include <string>
#include <vector>

#include <boost/range/iterator_range.hpp>

/** Generic HTTP Query parsing. No URL-decoding is performed.
 *
 * All returned types use iterator ranges poiting in the original string.
 *
 * May be rewritten to use (boost|std)::string_view in the future, though.
 */
namespace utility { namespace query {

typedef boost::iterator_range<std::string::const_iterator> Argument;
typedef std::vector<Argument> Arguments;
typedef std::pair<Argument, Argument> KeyValue;

/** Split query by '&' into list of arguments.
 */
Arguments splitQuery(const std::string &query);

/** Split query argument into key/value pair by '='.
 */
KeyValue splitArgument(const Argument &arg);

/** Tries to find argument's value with given key.
 */
Argument find(const Arguments &args, const std::string &key);

inline bool empty(const Argument &arg) {
    return std::begin(arg) == std::end(arg);
}

inline std::string asString(const Argument &arg) {
    return std::string(std::begin(arg), std::end(arg));
}

} } // namespace utility::query

#endif // utility_httpquery_hpp_included_
