/**
 * Copyright (c) 2017 Melown Technologies SE
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
/**
 * @file naturalsort.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * Natural sort implementation.
 */

#ifndef utility_naturalsort_hpp_included_
#define utility_naturalsort_hpp_included_

#include <cctype>
#include <string>

namespace utility {

namespace ns {

struct Key {
public:
    typedef std::vector<Key> list;

    Key(const std::string &key);

    bool operator<(const Key &o) const {
        return std::lexicographical_compare
            (items.begin(), items.end()
             , o.items.begin(), o.items.end());
    }

private:
    struct Item {
        std::string value;
        bool numeric;

        typedef std::vector<Item> list;

        Item(const std::string &value, bool numeric)
            : value(value), numeric(numeric)
        {}

        Item(const std::string::const_iterator &begin
             , const std::string::const_iterator &end
             , bool numeric)
            : value(begin, end), numeric(numeric)
        {}

        bool operator<(const Item &o) const {
            if (numeric && o.numeric) {
                // both are numeric value; compare value only on size match
                if (value.size() < o.value.size()) { return true; }
                if (o.value.size() < value.size()) { return false; }

                return value < o.value;
            }
            // numeric/non-numeric item sorts based on first character
            if (numeric && !o.numeric) { return true; }
            if (!numeric && o.numeric) { return false; }

            // plain compare
            return value < o.value;
        }
    };

    Item::list items;
};

inline Key::Key(const std::string &key)
{
    auto itemStart(key.begin());

    enum class State { begin, text, lzero, number };
    State state = State::begin;
    bool end(false);
    for (auto ikey(key.begin()), ekey(key.end()); !end; ++ikey) {
        end = (ikey == ekey);
        const auto digit(end ? false : std::isdigit(*ikey));
        const auto zero(digit && (*ikey == '0'));

        switch (state) {
        case State::begin:
            if (end) { break; }
            if (zero) {
                state = State::lzero;
            } else {
                state = digit ? State::number : State::text;
            }
            break;

        case State::text:
            if (end || digit) {
                state = (zero) ? State::lzero : State::number;
                items.emplace_back(itemStart, ikey, false);
                itemStart = ikey;
            }
            break;

        case State::lzero:
            if (zero) { break; } // keep eating zeroes
            if (digit) {
                state = State::number;
                itemStart = ikey;
            } else {
                // we need to insert just one zero
                items.emplace_back("0", true);
                state = State::text;
                itemStart = ikey;
            }
            break;

        case State::number:
            if (!digit) {
                state = State::text;
                items.emplace_back(itemStart, ikey, true);
                itemStart = ikey;
            }
            break;
        }
    }
}

struct Identity {
    const std::string& operator()(const std::string &v) const { return v; }
};

} // namespace ns

template <typename T, typename Extractor>
class NaturalLess {
public:
    NaturalLess(Extractor extractor) : extractor_(extractor) {}

    bool operator()(const T &lhs, const T &rhs) const {
        // TODO: cache
        return ns::Key(extractor_(lhs)) < ns::Key(extractor_(rhs));
    }
private:
    Extractor extractor_;
};

template <typename T>
NaturalLess<T, ns::Identity> natural_less() {
    return { ns::Identity() };
}

template <typename T, typename Extractor>
NaturalLess<T, Extractor> natural_less(Extractor extractor) {
    return { extractor };
}

} // namespace utility

#endif // utility_naturalsort_hpp_included_
