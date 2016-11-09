/** \file progress.hpp
 *
 * Simple progress information.
 */

#include "dbglog/dbglog.hpp"

#include "./progress.hpp"

namespace utility {

void Progress::report(const ratio_t &threshold, const std::string &name)
{
    if (full() || (ratio_t(value_ - reportedValue_, total_) < threshold)) {
        return;
    }

    auto r(ratio());
    auto integral(boost::rational_cast<std::size_t>(r * 100));
    auto decimals(boost::rational_cast<std::size_t>
                  ((r * 10000) - (integral * 100)));

    LOG(info3)
        << name << "progress: " << std::setw(3) << std::setfill(' ')
        << integral << "." << std::setw(2) << std::setfill('0')
        << decimals << " %.";

    reportedValue_ = value_;
}

void Progress::reportRational(const std::string &name)
{
    if (full() || value_ == reportedValue_) {
        return;
    }

    int numLen(log(total_) / log(10.0));
    
    LOG(info3)
        << name << "progress: " << std::setw(numLen + 1) << std::setfill(' ')
        << value_ << " / " << total_ << ".";

    reportedValue_ = value_;
}

void Progress::incrementAndReport(const ratio_t &threshold
                                  , const std::string &name)
{
    ++value_;
    report(threshold, name);
}

namespace ts {

void Progress::report(std::size_t rv)
{
    auto nextReportValue(nextReportValue_.load());
    if (rv >= nextReportValue) {
        // this should be reported, maybe :)
        std::lock_guard<std::mutex> lock(mutex_);

        // read again and check whether there was someone faster
        nextReportValue = nextReportValue_.load();
        if (rv < nextReportValue) { return; }

        // calculate new report value and store it so others see it now
        nextReportValue = calculateNextReportValue(rv);
        nextReportValue_.store(nextReportValue);

        // free to report
        Ratio r(rv, total_);
        auto integral(boost::rational_cast<std::size_t>(r * 100));
        auto decimals(boost::rational_cast<std::size_t>
                      ((r * 10000) - (integral * 100)));

        LOG(info3)
            << name_ << " progress: " << std::setw(3) << std::setfill(' ')
            << integral << "." << std::setw(2) << std::setfill('0')
            << decimals << " %.";
    }
}

} // namespace ts

} // namespace utility
