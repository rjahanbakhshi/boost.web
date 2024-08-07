//
// Copyright (c) 2022-2024 Reza Jahanbakhshi (reza dot jahanbakhshi at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/rjahanbakhshi/boost-taar
//

#ifndef BOOST_TAAR_CORE_IS_HTTP_RESPONSE_HPP
#define BOOST_TAAR_CORE_IS_HTTP_RESPONSE_HPP

#include <boost/beast/http/message.hpp>

namespace boost::taar {
namespace detail {

template<class T>
class is_http_response_impl
{
    template<class F>
    static std::true_type check(boost::beast::http::response<F> const*);
    static std::false_type check(...);

public:
    using type = decltype(check((T*)0));
};

} // namespace detail

template<class T>
concept is_http_response = requires
{
    requires detail::is_http_response_impl<T>::type::value;
};

} // namespace boost::taar

#endif // BOOST_TAAR_CORE_IS_HTTP_RESPONSE_HPP
