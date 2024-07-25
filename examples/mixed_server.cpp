//
// Copyright (c) 2022-2024 Reza Jahanbakhshi (reza dot jahanbakhshi at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/rjahanbakhshi/boost-taar
//

#include <boost/taar/handler/htdocs.hpp>
#include <boost/taar/handler/rest.hpp>
#include <boost/taar/session/http.hpp>
#include <boost/taar/server/tcp.hpp>
#include <boost/taar/handler/rest_arg.hpp>
#include <boost/taar/matcher/method.hpp>
#include <boost/taar/matcher/target.hpp>
#include <boost/taar/core/result_response.hpp>
#include <boost/taar/core/cancellation_signals.hpp>
#include <boost/taar/core/ignore_and_rethrow.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/json/value.hpp>
#include <exception>
#include <iostream>
#include <cstdlib>
#include <stdexcept>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: mixed_server port /example/document/root\n";
        return EXIT_FAILURE;
    }

    namespace net = boost::asio;
    namespace http = boost::beast::http;
    namespace taar = boost::taar;
    using net::co_spawn;
    using net::bind_cancellation_slot;
    using net::detached;
    using taar::result_response;
    using taar::matcher::method;
    using taar::matcher::target;

    net::io_context io_context;

    net::signal_set os_signals(io_context, SIGINT, SIGTERM);
    os_signals.async_wait([&](auto, auto) { io_context.stop(); });

    taar::session::http http_session;

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
        [](std::exception_ptr eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                return
                    result_response(boost::json::value{{"soft_error", e.what()}})
                        .set_status(http::status::internal_server_error);
            }
            catch (...)
            {
                std::cerr << "Unknown error!\n";
                return
                    result_response(boost::json::value{{"soft_error", "Unknown error!"}})
                        .set_status(http::status::internal_server_error);
            }
        }
    );

    http_session.register_request_handler(
        method == http::verb::get && target == "/special/{*}",
        [](
            const http::request<http::empty_body>& request,
            const taar::matcher::context& context) -> http::message_generator
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
        taar::handler::rest([]{ return "1.0"; }
    ));

    http_session.register_request_handler(
        method == http::verb::get && target == "/api/throw",
        taar::handler::rest([]{ throw std::runtime_error{"Error"}; }
    ));

    http_session.register_request_handler(
        method == http::verb::get && target == "/api/throw13",
        taar::handler::rest([]{ throw 13; }
    ));

    http_session.register_request_handler(
        method == http::verb::get && target == "/api/sum/{a}/{b}",
        taar::handler::rest([](int a, int b)
        {
            return boost::json::value {
                {"a", a},
                {"b", b},
                {"result", a + b}
            };
        },
        taar::handler::path_arg("a"),
        taar::handler::path_arg("b")
    ));

    http_session.register_request_handler(
        method == http::verb::post && target == "/api/store/",
        taar::handler::rest([](const std::string& value)
        {
            std::cout << "Storing value = " << value << '\n';
        },
        taar::handler::query_arg("value")
    ));

    http_session.register_request_handler(
        method == http::verb::get && target == "/{*}",
        taar::handler::htdocs {argv[2]}
    );

    taar::cancellation_signals cancellation_signals;
    co_spawn(
        io_context,
        taar::server::tcp(
            "0.0.0.0",
            argv[1],
            http_session,
            cancellation_signals,
            [](const net::ip::tcp::endpoint& endpoint)
            {
                std::clog << "HTTP server is listening on port " << endpoint.port() << '\n';
            }),
        bind_cancellation_slot(cancellation_signals.slot(), taar::ignore_and_rethrow));

    io_context.run();

    return EXIT_SUCCESS;
}

