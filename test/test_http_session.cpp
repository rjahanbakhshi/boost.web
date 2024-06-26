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
#include <boost/web/matcher/method.hpp>
#include <boost/web/matcher/target.hpp>
#include <boost/test/unit_test.hpp>

namespace {

BOOST_AUTO_TEST_CASE(test_http_session)
{
    namespace http = boost::beast::http;
    namespace web = boost::web;
    using web::matcher::method;
    using web::matcher::target;

    web::session::http http_session;

    http_session.set_hard_error_handler(
        [](std::exception_ptr eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            catch (...)
            {
                std::cerr << "Unknown error!\n";
            }
        });

    http_session.set_soft_error_handler(
        [](std::exception_ptr)
        {
            return boost::json::value{{"Error", "Oops!"}};
        }
    );

    http_session.register_request_handler(
        method == http::verb::get && target == "/special/{*}",
        [](
            const http::request<http::empty_body>& request,
            const web::matcher::context& context) -> http::message_generator
        {
            http::response<http::string_body> res {
                boost::beast::http::status::ok,
                request.version()};
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(request.keep_alive());
            res.body() = "Special path is: " + context.path_args.at("*");
            res.prepare_payload();
            return res;
        }
    );

    http_session.register_request_handler(
        method == http::verb::get && target == "/api/version",
        web::handler::rest([]{ return "1.0"; })
    );

    http_session.register_request_handler(
        method == http::verb::get && target == "/api/throw",
        web::handler::rest([]{ throw std::runtime_error{"Error"}; })
    );

    http_session.register_request_handler(
        method == http::verb::get && target == "/api/sum/{a}/{b}",
        web::handler::rest([](int a, int b)
        {
            return boost::json::value {
                {"a", a},
                {"b", b},
                {"result", a + b}
            };
        },
        web::handler::path_arg("a"),
        web::handler::path_arg("b"))
    );

    http_session.register_request_handler(
        method == http::verb::post && target == "/api/store/",
        web::handler::rest([](const std::string& value)
        {
            std::cout << "Storing value = " << value << '\n';
        },
        web::handler::query_arg("value"))
    );
}

} // namespace
