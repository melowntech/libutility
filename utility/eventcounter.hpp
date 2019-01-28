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

#ifndef utility_eventcounter_hpp_included_
#define utility_eventcounter_hpp_included_

#include <ctime>
#include <vector>
#include <mutex>

namespace utility {

/** Event counter. A cyclic buffer of slots storing events that occured in
 *  particular second. Calculates average in given number of last slots.
 */
class EventCounter {
public:
    /** Create event counter with given number of slots.
     */
    EventCounter(int size);

    /** Record event in the current slot. Can be used to report multiple events
     *  at once.
     */
    void event(std::size_t count = 1);

    void eventMax(std::size_t count = 1);

    /** Returns event average per second in over given second window. Current
     *  slot is ignored.
     *
     *  If there is not enough slots the count is reduced.
     */
    double average(std::size_t count) const;

    /** Returns event maximum in given second window. Current slot is ignored.
     *
     *  If there is not enough slots the count is reduced.
     */
    std::size_t max(std::size_t count) const;

    /** Returns event total (i.e. sum) in given second window. Current slot is
     * ignored.
     *
     *  If there is not enough slots the count is reduced.
     */
    std::size_t total(std::size_t count) const;

    /** Returns event average and maximum in given second window. Current slot
     * is ignored.
     *
     *  If there is not enough slots the count is reduced.
     */
    std::tuple<double, std::size_t> averageAndMax(std::size_t count) const;

    typedef std::vector<std::size_t> Counts;

    /** Reports averages to output stream.
     */
    void average(std::ostream &os, const std::string &name
                 , const Counts &counts = standardTimes) const;

    /** Reports total values to output stream.
     */
    void total(std::ostream &os, const std::string &name
               , const Counts &counts = standardTimes) const;

    /** Reports maximums to output stream.
     */
    void max(std::ostream &os, const std::string &name
             , const Counts &counts = standardTimes) const;

    /** Reports averages and maximums to output stream.
     */
    void averageAndMax(std::ostream &os, const std::string &name
             , const Counts &counts = standardTimes) const;

private:
    /** Internal function.
     */
    template <typename F>
    std::size_t processBlock(std::size_t count, const F &f) const;

    /** Event slot.
     */
    struct Slot {
        std::size_t count;
        std::time_t when;

        Slot() : count(), when() {}

        typedef std::vector<Slot> list;
    };

    mutable std::mutex mutex_;
    std::size_t size_;
    Slot::list slots_;

    /** Counts for standard times (5, 60 and 300 seconds).
     */
    static Counts standardTimes;
};

} // namespace utility

#endif // utility_eventcounter_hpp_included_
