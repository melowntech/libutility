/** \file progress.hpp
 *
 * Simple progress information.
 */

#include "dbglog/dbglog.hpp"

#include "./progress.hpp"

namespace utility {

void Progress::report(const ratio_t &threshold, const std::string &name)
{
    if (ratio_t(value_ - reportedValue_, total_) < threshold) {
        return;
    }

    auto r(ratio());
    auto integral(boost::rational_cast<int>(r * 100));
    auto decimals(boost::rational_cast<int>((r * 10000) - (integral * 100)));

    LOG(info3)
        << name << "Progress: " << std::setw(3) << std::setfill(' ')
        << integral << "." << std::setw(2) << std::setfill('0')
        << decimals << " %.";

    reportedValue_ = value_;
}

void Progress::incrementAndReport(const ratio_t &threshold
                                  , const std::string &name)
{
    // TODO: make atomic
    ++value_;
    report(threshold, name);
}

} // namespace utility
