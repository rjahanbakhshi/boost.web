//
// Copyright (c) 2022-2024 Reza Jahanbakhshi (reza dot jahanbakhshi at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/rjahanbakhshi/boost-taar
//

#include <boost/taar/core/is_http_response.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace {

struct not_a_response
{
    using header_type = int;
    using body_type = float;
};

namespace http = boost::beast::http;
using empty_response = http::response<http::empty_body>;
using string_response = http::response<http::string_body>;
using file_response = http::response<http::file_body>;

BOOST_AUTO_TEST_CASE(test_is_response)
{
    using namespace boost::taar;
    static_assert(is_http_response<empty_response>, "Failed!");
    static_assert(is_http_response<string_response>, "Failed!");
    static_assert(is_http_response<file_response>, "Failed!");

    static_assert(!is_http_response<http::request<http::empty_body>>, "Failed!");
    static_assert(!is_http_response<not_a_response>, "Failed!");
    static_assert(!is_http_response<std::string>, "Failed!");
    static_assert(!is_http_response<float>, "Failed!");
    static_assert(!is_http_response<int>, "Failed!");
}

} // namespace
