// MIT License
// 
// Copyright (c) 2023 Sadhbh Code
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// DERIVED WORK FROM: https://github.com/boostorg/beast
//

#ifndef INCLUDED_BAE_CITY_BEAST_SERVICE_HPP
#define INCLUDED_BAE_CITY_BEAST_SERVICE_HPP

#include "concepts.hpp"
#include "request.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>


namespace bae::city::beast {

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