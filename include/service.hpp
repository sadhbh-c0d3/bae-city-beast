//
// Copyright (c) 2023 Sonia Kolasinska
//
// WORK DERIVED FROM:
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Work derived from: Example: HTTP SSL server, coroutine
// Author of changes: Sonia Kolasinska, original author: Vinnie Falco
//
//------------------------------------------------------------------------------

#ifndef INCLUDED_BAE_CITY_BEAST_SERVICE_HPP
#define INCLUDED_BAE_CITY_BEAST_SERVICE_HPP

#include "concepts.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>


namespace bae::city::beast {

    struct ServiceConfig
    {
        using Address = boost::asio::ip::address;
        using Port = unsigned short;

        ServiceConfig(const std::string &address, Port port, int thread_count)
            : m_address(boost::asio::ip::make_address(address)), m_port(port), m_thread_count(thread_count)
        {}

        ServiceConfig(Address address, Port port, int thread_count)
            : m_address(std::move(address)), m_port(port), m_thread_count(thread_count)
        {}

    const Address &address() const { return m_address; }
    const Port &port() const { return m_port; }
    const int thread_count() const { return m_thread_count; }

    private:
        const Address m_address;
        const Port m_port;
        const int m_thread_count;
    };

    template <typename _RequestType>
    struct Request
    {
        using RequestType = _RequestType;
        template<typename T>
        using Response = boost::beast::http::response<T>;
        using FileBody = boost::beast::http::file_body;

        Response<boost::beast::http::string_body> BadRequest(std::string why)
        {
            namespace http = boost::beast::http;

            std::cerr << why << std::endl;

            http::response<http::string_body> res{http::status::bad_request, m_request.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_request.keep_alive());
            res.body() = std::move(why);
            res.prepare_payload();

            return std::move(res);
        }

        Response<boost::beast::http::string_body> NotFound(std::string target)
        {
            namespace http = boost::beast::http;

            std::cerr << "Not found: " << target << std::endl;

            http::response<http::string_body> res{http::status::not_found, m_request.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_request.keep_alive());
            res.body() = "The resource '" + target + "' was not found.";
            res.prepare_payload();

            return std::move(res);
        }

        Response<boost::beast::http::string_body> ServerError(std::string what)
        {
            namespace http = boost::beast::http;

            std::cerr << "Server error: " << what << std::endl;

            http::response<http::string_body> res{http::status::internal_server_error, m_request.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_request.keep_alive());
            res.body() = "An error occurred: '" + what + "'";
            res.prepare_payload();

            return std::move(res);
        }
    
        Response<boost::beast::http::empty_body> Success()
        {
            namespace http = boost::beast::http;
            
            std::cerr << "Ok" << std::endl;
            
            http::response<http::empty_body> res{http::status::ok, m_request.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.content_length(0);
            res.keep_alive(m_request.keep_alive());
            
            return std::move(res);
        }

        Response<boost::beast::http::string_body> TextResponse(std::string text)
        {
            namespace http = boost::beast::http;

            std::cerr << "Ok" << std::endl;

            http::response<http::string_body> res{http::status::ok, m_request.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_request.keep_alive());
            res.body() = text;
            res.prepare_payload();

            return std::move(res);
        }

        Response<FileBody> FileResponse(FileBody &&body, std::string mime_type)
        {
            namespace http = boost::beast::http;
            
            std::cerr << "File <" << mime_type << ">" << std::endl;

            Response<FileBody> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, m_request.version())};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type);
            res.content_length(res.body().size());
            res.keep_alive(m_request.keep_alive());

            return std::move(m_request);
        }

        Request(
            RequestType &&request,
            boost::beast::ssl_stream<boost::beast::tcp_stream> &stream,
            bool &close,
            boost::beast::error_code &ec,
            boost::asio::yield_context yield)
            : m_request(std::move(request))
            , m_stream(stream)
            , m_close(close)
            , m_ec(ec)
            , m_yield(yield)
        {
        }
        
        template<bool IsRequest, typename ResponseBody, typename Fields>
        void
        Send(boost::beast::http::message<IsRequest, ResponseBody, Fields>&& msg)
        {
            m_close = msg.need_eof();

            boost::beast::http::serializer<IsRequest, ResponseBody, Fields> s{msg};
            boost::beast::http::async_write(m_stream, s, m_yield[m_ec]);
        }

        RequestType &request() { return m_request; }
        RequestType *operator->() { return &m_request; }
        RequestType &operator*() { return m_request; }

        auto operator[](auto &&arg) const { return m_request[std::forward<decltype(arg)>(arg)]; }

    private:
        RequestType m_request;
        boost::beast::ssl_stream<boost::beast::tcp_stream> &m_stream;
        bool &m_close;
        boost::beast::error_code &m_ec;
        boost::asio::yield_context m_yield;
    };

    template <
        ServerConcept<
            Request<
                boost::beast::http::request<boost::beast::http::dynamic_body>>>
            _Server>
    struct Service
    {
        using Server = _Server;

        Service(Server &server): m_server(server) {}

        template<ServiceConfigConcept _ServiceConfig>
        int operator()(_ServiceConfig &&config)
        {
            boost::asio::io_context ioc{config.thread_count()};
            boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12};

            config.configure(ctx);

            std::cerr << "Listening on " << config.address() << ":" << config.port() << std::endl;

            boost::asio::spawn(
                ioc, [this, &ioc, &ctx, &config](auto &&arg)
                { do_listen(
                      ioc,
                      ctx,
                      boost::asio::ip::tcp::endpoint{config.address(), config.port()},
                      std::forward<decltype(arg)>(arg)); });

            std::vector<std::thread> v;
            v.reserve(config.thread_count() - 1);
            for (auto i = config.thread_count() - 1; i > 0; --i)
                v.emplace_back(
                    [&ioc]
                    {
                        ioc.run();
                    });
            ioc.run();

            return EXIT_SUCCESS;
        }

    private:
        Server &m_server;
        void
        do_listen(
            boost::asio::io_context& ioc,
            boost::asio::ssl::context& ctx,
            boost::asio::ip::tcp::endpoint endpoint,
            boost::asio::yield_context yield)
        {
            boost::beast::error_code ec;

            // Open the acceptor
            boost::asio::ip::tcp::acceptor acceptor(ioc);
            acceptor.open(endpoint.protocol(), ec);
            if (ec)
                return fail(ec, "open");

            // Allow address reuse
            acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
            if (ec)
                return fail(ec, "set_option");

            // Bind to the server address
            acceptor.bind(endpoint, ec);
            if (ec)
                return fail(ec, "bind");

            // Start listening for connections
            acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
            if (ec)
                return fail(ec, "listen");

            for (;;)
            {
                boost::asio::ip::tcp::socket socket(ioc);
                acceptor.async_accept(socket, yield[ec]);
                if (ec)
                    fail(ec, "accept");
                else
                {
                    boost::asio::spawn(
                        acceptor.get_executor(),
                        std::bind(
                            &Service::do_session,
                            this,
                            boost::beast::ssl_stream<boost::beast::tcp_stream>(
                                std::move(socket), ctx),
                            std::placeholders::_1));
                }
            }
        }

        void
        do_session(
            boost::beast::ssl_stream<boost::beast::tcp_stream>& stream,
            boost::asio::yield_context yield)
        {
            bool close = false;
            boost::beast::error_code ec;
        
            // Set the timeout.
            boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        
            // Perform the SSL handshake
            stream.async_handshake(boost::asio::ssl::stream_base::server, yield[ec]);
            if(ec)
                return fail(ec, "handshake");
        
            boost::beast::multi_buffer buffer;

            typedef boost::beast::http::request<boost::beast::http::dynamic_body> RequestType;
        
            for(;;)
            {
                // Set the timeout.
                boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        
                boost::beast::http::request<boost::beast::http::dynamic_body> req;
        
                // Read a request
                boost::beast::http::async_read(stream, buffer, req, yield[ec]);
                if(ec == boost::beast::http::error::end_of_stream)
                    break;
                if(ec)
                    return fail(ec, "read");
                
                // Send the response
                Request<RequestType> request{std::move(req), stream, close, ec, yield};
                m_server(request);

                if(ec)
                    return fail(ec, "write");
                if(close)
                {
                    // This means we should close the connection, usually because
                    // the response indicated the "Connection: close" semantic.
                    break;
                }
            }
        
            // Set the timeout.
            boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        
            // Perform the SSL shutdown
            stream.async_shutdown(yield[ec]);
            if(ec)
                return fail(ec, "shutdown");
        
            // At this point the connection is closed gracefully
        }

        void
        fail(boost::beast::error_code ec, char const *what)
        {
            if (ec == boost::asio::ssl::error::stream_truncated)
                return;

            std::cerr << what << ": " << ec.message() << "\n";
        }
    };

} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_SERVICE_HPP