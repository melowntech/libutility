/**
 * @file utility/supplement.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_supplement_hpp_included_
#define utility_supplement_hpp_included_

#include <boost/any.hpp>

namespace utility {

/** Simple class that adds a "supplement" to any class by inheriting from it.
 *
 * A supplement is anything you want to attach to class but you do not know what
 * is is. It is up to the user to attach and use supplement.
 *
 * Supplements are useful when you are building generic interface and you want
 * to allow to pass anything with passed data without forcing user to inherit
 * from your datatype.
 */
class Supplement {
public:
    template <typename T> Supplement& supplement(const T &supplement) {
        supplement_ = supplement;
        return *this;
    }

    bool hasSupplement() const { return !supplement_.empty(); }

    template <typename T> const T& supplement() const {
        return boost::any_cast<const T&>(supplement_);
    }

private:
    boost::any supplement_;
};

} // namespace utility

#endif // utility_supplement_hpp_included_
