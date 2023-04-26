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

#ifndef INCLUDED_BAE_CITY_BEAST_REQUEST_HPP
#define INCLUDED_BAE_CITY_BEAST_REQUEST_HPP

#include "concepts.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/config.hpp>
#include <iostream>
#include <string>


namespace bae::city::beast {

    template <typename _RequestType>
    struct BufferTypeTrait
    {
        using BufferType = std::conditional_t<
            std::is_same_v<_RequestType, boost::beast::http::request<
                                             boost::beast::http::dynamic_body>>,
            boost::beast::multi_buffer,
            boost::beast::flat_buffer>;
    };

    template <typename _RequestType>
    struct Request
    {
        using RequestType = _RequestType;
        using BufferType = typename BufferTypeTrait<RequestType>::BufferType;

        template<typename T>
        using Response = boost::beast::http::response<T>;
        using FileBody = boost::beast::http::file_body;

        Response<boost::beast::http::string_body> bad_request(std::string why)
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

        Response<boost::beast::http::string_body> not_found(std::string target)
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

        Response<boost::beast::http::string_body> server_error(std::string what)
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
    
        Response<boost::beast::http::empty_body> success()
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

        Response<boost::beast::http::string_body> text_response(std::string text)
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

        Response<FileBody> file_response(FileBody &&body, std::string mime_type)
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
    
        Request(const Request &) = delete;
        Request &operator =(const Request &) = delete;
        
        template<bool IsRequest, typename ResponseBody, typename Fields>
        void
        send(boost::beast::http::message<IsRequest, ResponseBody, Fields>&& msg)
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


    using StringRequest = Request<
        boost::beast::http::request<
            boost::beast::http::string_body>>;

    using DynamicRequest = Request<
        boost::beast::http::request<
            boost::beast::http::dynamic_body>>;

} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_REQUEST_HPP