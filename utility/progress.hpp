/** \file progress.hpp
 *
 * Simple progress information.
 */

#ifndef utility_progress_hpp_included_
#define utility_progress_hpp_included_

#include <cstddef>
#include <boost/rational.hpp>

namespace utility {

class Progress {
public:
    typedef boost::rational<std::size_t> ratio_t;

    Progress(std::size_t total) : total_(total), value_(), reportedValue_() {}

    Progress& operator++() { ++value_; return *this; }

    Progress& operator+=(std::size_t inc) { value_ += inc; return *this; }

    std::size_t total() const { return total_; }

    std::size_t value() const { return value_; }

    double percentage() const { return (100. * value_) / total_; }

    ratio_t ratio() const { return { value_, total_ }; }

    void report(const ratio_t &threshold
                , const std::string &name = std::string());

private:
    std::size_t total_;
    std::size_t value_;
    std::size_t reportedValue_;
};


} // namespace utility

#endif // utility_progress_hpp_included_
