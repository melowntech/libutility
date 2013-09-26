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

template <typename RowProcessor>
std::size_t parse(std::istream &is, const std::string &separator
                  , RowProcessor processor
                  , const LineRange &range = LineRange())
{
    typedef boost::char_separator<char> Separator;
    typedef boost::tokenizer<Separator> Tokenizer;
    Separator sep(separator.c_str());
    std::string line;
    std::vector<std::string> values;

    std::size_t index(0);
    while (getline(is, line) && (index <= range.to)) {
        // skip until from is reached
        if (index++ < range.from) { continue; }
        boost::algorithm::trim(line);
        if (line.empty() || (line[0] == '#')) { continue; }

        ++index;
        Tokenizer tok(line, sep);
        values.clear();
        std::transform(tok.begin(), tok.end(), back_inserter(values)
                       , [](const std::string &token) {
                           return boost::algorithm::trim_copy(token);
                       });

        processor(values);
    }
    return index;
}

template <typename Row>
std::vector<Row> parse(std::istream &is, const std::string &separator
                       , const LineRange &range = LineRange())
{
    std::vector<Row> rows;
    parse(is, separator, [&rows] (const std::vector<std::string> &values) {
            rows.emplace_back(values);
        }, range);
    return rows;
}

template <typename RowProcessor>
std::size_t parse(const boost::filesystem::path &path
                  , const std::string &separator
                  , RowProcessor processor
                  , const LineRange &range = LineRange())
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.string(), std::ios_base::in);
    f.exceptions(std::ios::badbit);

    return parse<RowProcessor>(f, separator, processor, range);
}

template <typename Row>
std::vector<Row> parse(const boost::filesystem::path &path
                       , const std::string &separator
                       , const LineRange &range = LineRange())
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(path.string(), std::ios_base::in);
    f.exceptions(std::ios::badbit);

    return parse<Row>(f, separator, range);
}

} // namespace separated_values

} // namespace utility

#endif // utility_parse_hpp_included_
