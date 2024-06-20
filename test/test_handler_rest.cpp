//
// Copyright (c) 2022-2024 Reza Jahanbakhshi (reza dot jahanbakhshi at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/rjahanbakhshi/boost-web
//

#include <boost/web/session/http.hpp>
#include <boost/web/handler/rest.hpp>
#include "boost/web/handler/rest_arg.hpp"
#include <boost/web/matcher/method.hpp>
#include <boost/web/matcher/target.hpp>
#include <boost/web/matcher/context.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace {

// Function to convert message_generator to http::response
template<class BodyType>
boost::beast::http::response<BodyType> to_response(
    boost::beast::http::message_generator& generator)
{
    boost::beast::error_code ec;
    boost::beast::http::response_parser<BodyType> parser;

    while (!parser.is_done())
    {
        auto buffers = generator.prepare(ec);
        if(ec)
        {
            throw boost::system::system_error{ec};
        }

        auto n = parser.put(buffers, ec);
        if(ec && ec != boost::beast::http::error::need_buffer)
        {
            throw boost::system::system_error{ec};
        }

        generator.consume(n);
    }

    return parser.release();
}

struct const_obj_handler
{
    std::string operator()() const { return "blah1"; }
};

struct obj_handler
{
    std::string operator()() { return "blah2"; }
};

BOOST_AUTO_TEST_CASE(test_handler_rest)
{
    namespace http = boost::beast::http;
    namespace web = boost::web;
    using web::matcher::context;

    http::request<http::string_body> req;
    context ctx;

    web::handler::rest rh1 {const_obj_handler()};
    auto mg1 = rh1(req, ctx);
    auto resp1 = to_response<http::string_body>(mg1);
    BOOST_TEST(resp1.body() == "blah1");

    web::handler::rest rh2 {obj_handler()};
    auto mg2 = rh2(req, ctx);
    auto resp2 = to_response<http::string_body>(mg2);
    BOOST_TEST(resp2.body() == "blah2");
}

auto sum(int i, int j)
{
    return std::to_string(i + j);
}

BOOST_AUTO_TEST_CASE(test_handler_rest_sum_target)
{
    namespace http = boost::beast::http;
    namespace web = boost::web;
    using web::matcher::context;
    using web::handler::path_arg;

    http::request<http::string_body> req;
    context ctx;
    ctx.path_args = {{"a", "13"}, {"b", "42"}};

    web::handler::rest rh_sum {sum, path_arg("a"), path_arg("b")};
    auto mg_sum = rh_sum(req, ctx);
    auto resp_sum = to_response<http::string_body>(mg_sum);
    BOOST_TEST(resp_sum.body() == "55");
    BOOST_TEST(resp_sum[http::field::content_type] == "text/html");
}

BOOST_AUTO_TEST_CASE(test_handler_rest_sum_query)
{
    namespace http = boost::beast::http;
    namespace web = boost::web;
    using web::matcher::context;
    using web::handler::query_arg;

    http::request<http::string_body> req{http::verb::get, "/?a=13&b=42", 10};
    context ctx;

    web::handler::rest rh_sum {sum, query_arg("a"), query_arg("b")};
    auto mg_sum = rh_sum(req, ctx);
    auto resp_sum = to_response<http::string_body>(mg_sum);
    BOOST_TEST(resp_sum.body() == "55");
    BOOST_TEST(resp_sum[http::field::content_type] == "text/html");
}

BOOST_AUTO_TEST_CASE(test_handler_rest_sum_header)
{
    namespace http = boost::beast::http;
    namespace web = boost::web;
    using web::matcher::context;
    using web::handler::header_arg;

    http::request<http::string_body> req;
    context ctx;
    req.insert("a", "13");
    req.insert("b", "42");

    web::handler::rest rh_sum {sum, header_arg("a"), header_arg("b")};
    auto mg_sum = rh_sum(req, ctx);
    auto resp_sum = to_response<http::string_body>(mg_sum);
    BOOST_TEST(resp_sum.body() == "55");
    BOOST_TEST(resp_sum[http::field::content_type] == "text/html");
}

auto to_json(std::string value)
{
    return boost::json::value {{"value", value}};
}

BOOST_AUTO_TEST_CASE(test_handler_rest_json_response)
{
    namespace http = boost::beast::http;
    namespace web = boost::web;
    using web::matcher::context;
    using web::handler::header_arg;

    http::request<http::string_body> req;
    context ctx;
    req.insert("value", "Hello");

    web::handler::rest rh {to_json, header_arg("value")};
    auto mg = rh(req, ctx);
    auto resp = to_response<http::string_body>(mg);
    BOOST_TEST(resp.body() == R"({"value":"Hello"})");
    BOOST_TEST(resp[http::field::content_type] == "application/json");
}


} // namespace