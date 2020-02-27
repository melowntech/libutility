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

#ifndef utility_po_hpp_included_
#define utility_po_hpp_included_

#include <boost/program_options.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

namespace utility { namespace po {

/** Positive-only configuration option value.
 */
template <typename T>
struct Positive {
    T value;
    Positive(T value = {}) : value(value) {}
    operator T() const { return value; }
};

using pos_short_t = Positive<short>;
using pos_int_t = Positive<int>;
using pos_long_t = Positive<long>;
using pos_long_long_t = Positive<long long>;

template <typename T>
void validate(boost::any &v
              , const std::vector<std::string> &xs
              , Positive<T>*, int)
{
    namespace bpo = boost::program_options;
    bpo::validators::check_first_occurrence(v);
    std::string s(bpo::validators::get_single_string(xs));
    try {
        Positive<T> value{boost::lexical_cast<T>(s)};
        if (value.value <= 0) {
            boost::throw_exception(bpo::invalid_option_value(s));
        }
        v = boost::any(value);
    } catch(const boost::bad_lexical_cast&) {
        boost::throw_exception(bpo::invalid_option_value(s));
    }
}

} } // namespace utility::po

#endif // utility_po_hpp_included_
