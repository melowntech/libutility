/** \file progress.hpp
 *
 * Simple progress information.
 */

#ifndef utility_progress_hpp_included_
#define utility_progress_hpp_included_

#include <atomic>
#include <thread>
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

    void incrementAndReport(const ratio_t &threshold
                            , const std::string &name = std::string());

private:
    std::size_t total_;
    std::size_t value_;
    std::size_t reportedValue_;
};

/** Thread-safe variant
 */
namespace ts {

class Progress {
public:
    typedef boost::rational<std::size_t> Ratio;

    Progress(const std::string &name, std::size_t total
             , const Ratio &reportTreshold = Ratio(1, 100))
        : name_(name), total_(total), reportTreshold_(reportTreshold)
        , value_(0), nextReportValue_(calculateNextReportValue(0))
    {}

    Progress& operator++() { report(++value_); return *this; }

    Progress& operator+=(std::size_t inc) {
        report(value_ += inc); return *this;
    }

    std::size_t total() const { return total_; }

    std::size_t value() const { return value_; }

    double percentage() const { return (100. * value_) / total_; }

    Ratio ratio() const { return { value_, total_ }; }

private:
    std::size_t calculateNextReportValue(std::size_t v) {
        auto next(boost::rational_cast<std::size_t>
                  (total_ * (Ratio(v, total_) + reportTreshold_)));
        if (next == v) {
            // too small treshold, add just one
            return next + 1;
        }
        return next;
    }

    void report(std::size_t rv);

    const std::string name_;
    const std::size_t total_;
    const Ratio reportTreshold_;

    std::atomic<std::size_t> value_;
    std::atomic<std::size_t> nextReportValue_;
    std::mutex mutex_;
};

} // namespace ts

} // namespace utility

#endif // utility_progress_hpp_included_
