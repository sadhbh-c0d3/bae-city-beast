#include "service.hpp"
#include "secure_config.hpp"

//!SSL certificate loader
#ifdef __has_include
# if __has_include("main.i")
#   include "main.i"
#define HAS_CERT
# endif
#else
#   error There is no SSL certificate file!
#endif

#include <iostream>

struct MyServer
{
    MyServer(std::string document_root)
        : m_document_root(std::move(document_root)) {}

    bool authenticate_user(const std::string &authorization)
    {
        // admin:password123
        return (authorization == "Basic YWRtaW46cGFzc3dvcmQxMjM=");
    }

    void operator()(bae::city::beast::RequestConcept auto &&request)
    {
        namespace http = boost::beast::http;

        std::cout << request->method() << " " << request->target() << std::endl;
        std::cout << "Authorization: " << request[http::field::authorization] << std::endl;
    
        if (!authenticate_user(std::string(request[http::field::authorization])))
        {
            return request.send(request.bad_request("Unauthorized"));
        }

        if (request->method() == http::verb::get)
        {
            return request.send(request.text_response("Hello!"));
        }
        else if (request->method() == http::verb::post)
        {
            std::cout << "Body size: " << request->body().size() << std::endl;

            for (auto buf : request->body().cdata())
            {
                std::cout << "Chunk size: " << buf.size() << std::endl;
            }
            
            return request.send(request.success());
        }

        if (request["Custom-Header"] == "pass")
        {
            return request.send(request.text_response("Passed"));
        }
            
        return request.send(request.bad_request("Invalid request"));
    }

private:
    std::string m_document_root;
};

struct MyLogger
{
    void info(auto &&...text)
    {
        format(std::cout, std::forward<decltype(text)>(text)...);
    }

    void error(auto &&...text)
    {
        format(std::cerr, std::forward<decltype(text)>(text)...);
    }

private:
    std::ostream &format(std::ostream &os, auto && text1)
    {
        return os << text1 << std::endl;
    }

    std::ostream &format(std::ostream &os, auto && text1, auto &&...text)
    {
        return format(os << text1, std::forward<decltype(text)>(text)...);
    }
};

int main(int argc, const char **argv)
{
    if (argc != 5)
    {
        std::cerr <<
            "Usage: run_app <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    run_app 0.0.0.0 8080 /home/volume/ 1\n";
        return EXIT_FAILURE;
    }

    auto const address = argv[1];
    auto const port = std::atoi(argv[2]);
    auto const document_root = argv[3];
    auto const thread_count = std::max<int>(1, std::atoi(argv[4]));

#ifdef HAS_CERT
    auto security = MySecurity{};
    auto config = bae::city::beast::SecureConfig<MySecurity>{security, address, port, thread_count};

    auto logger = MyLogger{};
    auto server = MyServer{document_root};
    auto service = bae::city::beast::Service<
        MyLogger, bae::city::beast::DynamicRequests, MyServer>{logger, server};
    
    return service(config);
#endif
}
