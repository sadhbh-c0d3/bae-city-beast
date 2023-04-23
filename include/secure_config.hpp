#ifndef INCLUDED_BAE_CITY_BEAST_SECURE_CONFIG_HPP
#define INCLUDED_BAE_CITY_BEAST_SECURE_CONFIG_HPP

#include "service.hpp"

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