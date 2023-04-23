#ifndef INCLUDED_BAE_CITY_BEAST_SERVICE_HPP
#define INCLUDED_BAE_CITY_BEAST_SERVICE_HPP

#include "concepts.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/config.hpp>


namespace bae::city::beast {

    struct ServiceConfig
    {
        using Address = boost::asio::ip::address;
        using Port = unsigned short;

        ServiceConfig(const std::string &address, Port port, size_t thread_count)
            : m_address(boost::asio::ip::make_address(address)), m_port(port), m_thread_count(thread_count)
        {}

        ServiceConfig(Address address, Port port, size_t thread_count)
            : m_address(std::move(address)), m_port(port), m_thread_count(thread_count)
        {}

    const Address &address() const { return m_address; }
    const Port &port() const { return m_port; }
    const size_t thread_count() const { return m_thread_count; }

    private:
        const Address m_address;
        const Port m_port;
        const size_t m_thread_count;
    };

    template <typename _Body, typename _Allocator, typename _Send>
    struct Request
    {
        template<typename T = int>
        struct Response {}; //< boost::beast::http::response<T>

        using ResponseExample = Response<int>;

        Response<> BadRequest();
        Response<> NotFound();
        Response<> ServerError();
        Response<> StatusCode();
        Response<> TextResponse();
        Response<> FileResponse();

        template<typename _Response>
        void Send(_Response &&);
    };

    template <ServerConcept _Server>
    struct Service
    {
        using Server = _Server;

        Service(Server &server): m_server(server) {}

        template<ServiceConfigConcept _ServiceConfig>
        int operator()(_ServiceConfig &&config)
        {
            return 0;
        }

    private:
        Server &m_server;
    };

} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_SERVICE_HPP