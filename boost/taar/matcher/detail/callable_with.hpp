//
// Copyright (c) 2022-2024 Reza Jahanbakhshi (reza dot jahanbakhshi at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/rjahanbakhshi/boost-taar
//

#ifndef BOOST_TAAR_MATCHER_DETAIL_CALLABLE_WITH_HPP
#define BOOST_TAAR_MATCHER_DETAIL_CALLABLE_WITH_HPP

#include <concepts>
#include <type_traits>

namespace boost::taar::matcher {
namespace detail {

// Concept to check if the callable is compatible with the signature.
template <typename CallableType, typename ResultType, typename... ArgsType>
concept callable_with =
    std::same_as<std::result_of_t<CallableType(ArgsType...)>, ResultType> ||
    std::constructible_from<std::result_of_t<CallableType(ArgsType...)>, ResultType>;

} // detail
} // namespace boost::taar::matcher

#endif // BOOST_TAAR_MATCHER_DETAIL_CALLABLE_WITH_HPP
