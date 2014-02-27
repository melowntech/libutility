#ifndef shared_utility_map_hpp_included_
#define shared_utility_map_hpp_included_

#include <future>
#include <cstdlib>
#include <utility>

#include <boost/thread.hpp>
#include <boost/range.hpp>

#include "dbglog/dbglog.hpp"

#include "./detail/map.hpp"

namespace utility {

/** Returns [ callable(value, args...) for value in values ], i.e. vector of
 * values mapped by `callable'.
 *
 * Operations are performed in parallel if there are more than one execution
 * unit (CPU, CPU core) available and there are more than one elements in
 * `value'.
 *
 * Result vector value_type behaves almost like boost::optional<X> where X is
 * result type of function call; you can use get(), operator* and operator->.
 *
 * If function call terminates with an exception this exception is saved and
 * thrown on get/operator* /operator-> calls.
 *
 * \param name name of operation (used in log messages)
 * \param values sequence of values to process
 * \param callable callable(const Sequence::value_type&, args...)
 * \param ...args arguments to be passed to each call of callable
 * \return list of results
 */
template <typename Sequence, typename Callable, typename... Args>
typename detail::map_helper<Sequence, Callable, Args...>::ResultList
map(const std::string &name, const Sequence &values
    , const Callable &callable, Args&& ...args);

// ************************************************************************
// Implementation

template <typename Sequence, typename Callable, typename... Args>
typename detail::map_helper<Sequence, Callable, Args...>::ResultList
map(const std::string &name, const Sequence &values
    , const Callable &callable, Args&& ...args)
{
    typedef detail::map_helper<Sequence, Callable, Args&&...> Helper;
    typedef typename Helper::Result Result;
    typedef typename Helper::ResultList ResultList;
    typedef typename Helper::SequenceSubRange SequenceSubRange;

    Helper helper;

    // NB: std::thread::hardware_concurrency() returned 0 at the time when this
    // was written, using boost::thread::hardware_concurrency() instead
    size_t threadCount
        (std::min<size_t>
         (boost::thread::hardware_concurrency(), values.size()));

    if (getenv("NO_THREADS")) {
        threadCount = 1;
    }

    if (threadCount < 2) {
        // single thread -> just run here
        ResultList result;
        result.resize(values.size());

        helper(SequenceSubRange(values), name, 0, values.size()
               , result.begin(), callable, std::forward<Args>(args)...);
        return result;
    }

    std::vector<std::future<void> > jobs;
    jobs.reserve(values.size());

    ResultList result;
    result.resize(values.size());

    size_t count(values.size());

    // compute partition size
    size_t partition(count / threadCount);
    int extra(count % threadCount);

    // start thread logging
    dbglog::log_thread();

    for (size_t i(0), start(0), length(0); i < threadCount;
         ++i, start += length, --extra)
    {
        length = (extra > 0) ? (partition + 1) : partition;

        SequenceSubRange range
            (values.begin() + start
             , ((start + length) < static_cast<size_t>(values.size()))
             ? values.begin() + start + length : values.end());

        typename ResultList::iterator iresult(result.begin() + start);

        jobs.push_back
            (std::async(std::launch::async, helper, range
                        , str(boost::format("%s:%d/%d") % name
                              % (i + 1) % threadCount)
                        , start, count, iresult, callable
                        , std::forward<Args>(args)...));
    }

    for (auto &job : jobs) {
        job.wait();
    }

    // stop thread logging
    dbglog::log_thread(false);

    return result;
}

} // namespace utility

#endif // shared_utility_map_hpp_included_
