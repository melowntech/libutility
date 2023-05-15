/**
 * Copyright (c) 2023 Melown Technologies SE
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

#ifndef utility_streamspirit_hpp_included_
#define utility_streamspirit_hpp_included_

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/spirit/include/qi_match_attr.hpp>
#include <boost/spirit/include/qi_match_auto.hpp>
#include <boost/spirit/include/qi_alternative.hpp>
#include <boost/spirit/include/qi_stream.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

namespace utility {

/** Parses single token from input stream (using istream >> std::string) and
 *  then parses given token as a single phrase without skipping whitespaces via
 *  lexeme[grammar].
 */
template<typename CharT, typename Traits
         , typename Grammar, typename ...Attrs>
auto& parseToken(std::basic_istream<CharT, Traits> &is
                 , Grammar &&grammar
                 , Attrs&&... attrs)
{
    using boost::spirit::qi::phrase_match;
    using boost::spirit::ascii::blank;
    using boost::spirit::qi::skip_flag;
    using boost::spirit::lexeme;

    // prapare token extraction
    typename std::basic_istream<CharT, Traits>::sentry sentry(is);
    if (!sentry) { return is; }

    // extract token
    std::string token;
    if (!(is >> token)) { return is; }

    // parse
    auto b(token.begin()), e(token.end());
    auto res(phrase_parse(b, e, lexeme[std::move(grammar)]
                          , blank, skip_flag::dont_postskip
                          , std::forward<Attrs>(attrs)...));

    // error checking
    if (!res || (b != e)) {
        is.setstate(std::ios::failbit);
    }

    // OK
    return is;
}

} // namespace utility

#endif // utility_streamspirit_hpp_included_
