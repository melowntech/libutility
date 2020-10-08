/**
 * Copyright (c) 2019-2020 Melown Technologies SE
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

#include <new>
#include <iosfwd>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

namespace utility { namespace po {

template <typename T, typename Key>
boost::optional<T> get(const boost::program_options::variables_map &vars
                       , const Key &key, const std::nothrow_t&);

template <typename T, typename Key>
T get(const boost::program_options::variables_map &vars, const Key &key);

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

/** Simple class for holding and parsing program options.
 */
class ProgramOptions {
public:
    ProgramOptions(const std::string &help);

    std::ostream& dump(std::ostream &os, const std::string& = "") const;

protected:
    /** Parses arguments. Handles help request.
     *
     * Returns false if help was handled and no further processing was
     * performed.
     *
     * Returns true if args were fully processed.
     *
     * \params args arguments
     * \params os   help output stream
     * \return true if args are usable.
     */
    bool parse(const std::vector<std::string> &args
               , std::ostream &os);

    /** Same as above but prints help to stdout.
     */
    bool parse(const std::vector<std::string> &args);

    boost::program_options::options_description od_;
    boost::program_options::positional_options_description pd_;
    boost::program_options::variables_map vars_;
};

// inlines

template <typename T, typename Key>
boost::optional<T> get(const boost::program_options::variables_map &vars
                       , const Key &key, const std::nothrow_t&)
{
    auto fvars(vars.find(key));
    if (fvars == vars.end()) { return boost::none; }
    return fvars->second.template as<T>();
}

template <typename T, typename Key>
T get(const boost::program_options::variables_map &vars
      , const Key &key)
{
    return vars.at(key).template as<T>();
}

} } // namespace utility::po

#endif // utility_po_hpp_included_
