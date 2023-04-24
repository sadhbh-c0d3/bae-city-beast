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

#ifndef INCLUDED_BAE_CITY_BEAST_SERVICE_CONFIG_HPP
#define INCLUDED_BAE_CITY_BEAST_SERVICE_CONFIG_HPP

#include <boost/asio/ip/address.hpp>
#include <string>


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

} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_SERVICE_CONFIG_HPP