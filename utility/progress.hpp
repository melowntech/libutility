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
    Progress(std::size_t total) : total_(total), current_() {}

    Progress& operator++() { ++current_; return *this; }

    Progress& operator+=(std::size_t inc) { current_ += inc; return *this; }

    double percentage() const { return (100. * current_) / total_; }

    boost::rational<std::size_t> value() const { return { current_, total_ }; }

private:
    std::size_t total_;
    std::size_t current_;
};


} // namespace utility

#endif // utility_progress_hpp_included_
