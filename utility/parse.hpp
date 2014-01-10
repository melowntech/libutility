#ifndef utility_parse_hpp_included_
#define utility_parse_hpp_included_

#include <cstddef>
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace utility {

struct LineRange {
    LineRange(std::size_t from = 0
              , std::size_t to = std::numeric_limits<std::size_t>::max())
        : from(from), to(to)
    {}

    std::size_t from;
    std::size_t to;
};

namespace separated_values {

enum {
    FLAG_KEEP_EMPTY_TOKENS = 0x01
    , FLAG_DONT_TRIM_FIELDS = 0x02
};

template <typename RowProcessor>
std::size_t parse(std::istream &is, const std::string &separator
                  , RowProcessor processor
                  , const LineRange &range = LineRange()
                  , int flags = 0x0)
{
    typedef boost::char_separator<char> Separator;
    typedef boost::tokenizer<Separator> Tokenizer;
    Separator sep(separator.c_str(), nullptr
                  , ((flags & FLAG_KEEP_EMPTY_TOKENS)
                     ? boost::keep_empty_tokens
                     : boost::drop_empty_tokens));
    std::string line;
    std::vector<std::string> values;

    std::size_t index(0);
    while (getline(is, line) && (index <= range.to)) {
        // skip until from is reached
        if (index++ < range.from) { continue; }
        auto trimmed(boost::algorithm::trim_copy(line));
        if (trimmed.empty() || (line[0] == '#')) { continue; }

        ++index;
        Tokenizer tok(line, sep);
        values.clear();
        std::transform(tok.begin(), tok.end(), back_inserter(values)
                       , [&](const std::string &token) {
                           return ((flags & FLAG_DONT_TRIM_FIELDS)
                                   ? token
                                   : boost::algorithm::trim_copy(token));
                       });

        processor(values);
    }
    return index;
}

template <typename Row>
std::vector<Row> parse(std::istream &is, const std::string &separator
                       , const LineRange &range = LineRange()
                       , int flags = 0x0)
{
    std::vector<Row> rows;
    parse(is, separator, [&rows] (const std::vector<std::string> &values) {
            rows.emplace_back(values);
        }, range, flags);
    return rows;
}

template <typename RowProcessor>
std::size_t parse(const boost::filesystem::path &path
                  , const std::string &separator
                  , RowProcessor processor
                  , const LineRange &range = LineRange()
                  , int flags = 0x0)
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.string(), std::ios_base::in);
    f.exceptions(std::ios::badbit);

    return parse<RowProcessor>(f, separator, processor, range, flags);
}

template <typename Row>
std::vector<Row> parse(const boost::filesystem::path &path
                       , const std::string &separator
                       , const LineRange &range = LineRange()
                       , int flags = 0x0)
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.string(), std::ios_base::in);
    f.exceptions(std::ios::badbit);

    return parse<Row>(f, separator, range, flags);
}

template <typename RowProcessor>
void split(const std::string &line, const std::string &separator
           , RowProcessor processor
           , int flags = 0x0)
{
    typedef boost::char_separator<char> Separator;
    typedef boost::tokenizer<Separator> Tokenizer;
    Separator sep(separator.c_str(), nullptr
                  , ((flags & FLAG_KEEP_EMPTY_TOKENS)
                     ? boost::keep_empty_tokens
                     : boost::drop_empty_tokens));
    std::vector<std::string> values;

    Tokenizer tok(line, sep);
    std::transform(tok.begin(), tok.end(), back_inserter(values)
                   , [&](const std::string &token) {
                       return ((flags & FLAG_DONT_TRIM_FIELDS)
                               ? token
                               : boost::algorithm::trim_copy(token));
                   });

    processor(values);
}

template <typename Row>
Row split(const std::string &line, const std::string &separator
          , int flags = 0x0)
{
    boost::optional<Row> row;
    split(line, separator, [&row] (const std::vector<std::string> &v) {
            row = boost::in_place(boost::cref(v));
        }, flags);

    return *row;
}

} // namespace separated_values

} // namespace utility

#endif // utility_parse_hpp_included_
