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
 * @file utility/supplement.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 */

#ifndef utility_supplement_hpp_included_
#define utility_supplement_hpp_included_

#include <stdexcept>

#include <boost/any.hpp>

#include "dbglog/dbglog.hpp"

#include "./typeinfo.hpp"

namespace utility {

/** Simple class that adds a "supplement" to any class by inheriting from it.
 *
 * A supplement is anything you want to attach to class but you do not know what
 * is is. It is up to the user to attach and use supplement.
 *
 * Supplements are useful when you are building generic interface and you want
 * to allow to pass anything with passed data without forcing user to inherit
 * from your datatype.
 *
 * Use CRTP to allow cast from supplement to derived class:
 *
 * class Class : public Supplement<Class> { ... };
 *
 */
template <typename Owner>
class Supplement {
public:
    Supplement() = default;
    Supplement(Supplement &&other) {
        boost::swap(supplement_, other.supplement_);
    }
    Supplement(const Supplement&) = default;

    Supplement& operator=(Supplement &&other) {
        boost::swap(supplement_, other.supplement_); return *this;
    }
    Supplement& operator=(const Supplement&) = default;

    template <typename T>
    Supplement& supplement(const T &supplement) {
        supplement_ = supplement; return *this;
    }

    template <typename T>
    Supplement& supplement(T &supplement) {
        supplement_ = std::move(supplement); return *this;
    }

    /** Copies supplement from another supplement.
     */
    template <typename T>
    Supplement& supplementFrom(const Supplement<T> &other) {
        supplement_ = other.supplement_; return *this;
    }

    /** Steals supplement from other supplement.
     */
    template <typename T>
    Supplement& supplementFrom(Supplement<T> &&other) {
        supplement_ = std::move(other.supplement_); return *this;
    }

    bool hasSupplement() const { return !supplement_.empty(); }

    template <typename T> const T& supplement() const;

    Owner& owner() { return static_cast<Owner&>(*this); }
    const Owner& owner() const { return static_cast<const Owner&>(*this); }

    // TODO: add rvalue-this version when moved to gcc-4.8

private:
    boost::any supplement_;
};

// inlines

template <typename Owner>
template <typename T>
const T& Supplement<Owner>::supplement() const
{
    if (const auto *value = boost::any_cast<T>(&supplement_)) {
        return *value;
    }

    LOGTHROW(err1, std::logic_error)
        << "Cannot get value of type <" << typeName<T>()
        << "> from suplement holding instance of type <"
        << typeName(supplement_.type()) << ">.";
    throw;
}

} // namespace utility

#endif // utility_supplement_hpp_included_
