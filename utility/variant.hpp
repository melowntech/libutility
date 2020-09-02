/**
 * Copyright (c) 2020 Melown Technologies SE
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

#ifndef utility_variant_hpp_included_
#define utility_variant_hpp_included_

/** Simple lambda-composed static visitor for boost::variant.
 *  Inspired by https://stackoverflow.com/a/7870614/1623502
 */

#include <utility>

#include <boost/variant.hpp>

namespace utility {

namespace detail {

/** boost::variant visitor where all operator() functions are implemented by
 *  provided list of lambdas.
 */
template <typename ReturnType, typename... Lambdas>
struct lambda_visitor;

/** Recursive case.
 */
template <typename ReturnType, typename Lambda, typename... Lambdas>
struct lambda_visitor<ReturnType, Lambda, Lambdas...>
    : std::remove_reference<Lambda>::type
    , lambda_visitor<ReturnType, Lambdas...>
{
    using LambdaType = typename std::remove_reference<Lambda>::type;

    /** Pull in call operator from fist lambda
     */
    using LambdaType::operator();

    /** Pull in the rest of call operators.
     */
    using lambda_visitor<ReturnType, Lambdas...>::operator();

    lambda_visitor(Lambda lambda, Lambdas ...lambdas)
        : LambdaType(lambda)
        , lambda_visitor<ReturnType, Lambdas...>
        (std::forward<Lambdas>(lambdas)...)
    {}
};

/** Terminator case.
 */
template <typename ReturnType, typename Lambda>
struct lambda_visitor<ReturnType, Lambda>
    : boost::static_visitor<ReturnType>
    , std::remove_reference<Lambda>::type
{
    using LambdaType = typename std::remove_reference<Lambda>::type;

    /** Pull in call operator from fist lambda
     */
    using LambdaType::operator();

    lambda_visitor(Lambda lambda)
        : LambdaType(std::forward<Lambda>(lambda)) {}
};

} // namespace detail

/** Creates static visitor suitable for boost::variant from list of lambdas.
 */
template <typename ReturnType, typename... Lambdas>
detail::lambda_visitor<ReturnType, Lambdas...>
make_lambda_visitor(Lambdas &&...lambdas)
{
    return { std::forward<Lambdas>(lambdas)... };
}

template <typename T, typename... Ts>
T& make_variant(boost::variant<Ts...> &variant)
{
    return boost::get<T>(variant = T());
}

} // namespace utility

#endif // utility_variantri_hpp_included_
