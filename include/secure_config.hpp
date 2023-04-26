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

#ifndef INCLUDED_BAE_CITY_BEAST_SECURE_CONFIG_HPP
#define INCLUDED_BAE_CITY_BEAST_SECURE_CONFIG_HPP

#include "service_config.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>


namespace bae::city::beast {

    template<SslCertificateLoaderConcept _SslCertificateLoader>
    struct SecureConfig : ServiceConfig
    {
        using Context = boost::asio::ssl::context;
        using SslCertificateLoader = _SslCertificateLoader;

        template<typename... A>
        SecureConfig(SslCertificateLoader &loader, A&&... a)
            : ServiceConfig(std::forward<A>(a)...)
            , m_loader(loader) {}

        SecureConfig(const SecureConfig &) = delete;
        SecureConfig &operator =(const SecureConfig &) = delete;

        void configure(Context &context)
        {
            m_loader.load();

            auto &cert = m_loader.cert();
            auto &key = m_loader.key();
            auto &dh = m_loader.dh();

            context.set_password_callback(
                [this](size_t, boost::asio::ssl::context_base::password_purpose)
                { return std::move(m_loader.get_password()); });

            context.set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::single_dh_use);

            context.use_certificate_chain(
                boost::asio::buffer(cert.data(), cert.size()));

            context.use_private_key(
                boost::asio::buffer(key.data(), key.size()),
                boost::asio::ssl::context::file_format::pem);

            context.use_tmp_dh(
                boost::asio::buffer(dh.data(), dh.size()));
        }

    private:
        SslCertificateLoader &m_loader;
    };

} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_SECURE_CONFIG_HPP