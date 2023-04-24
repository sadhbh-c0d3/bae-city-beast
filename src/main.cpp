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

    template <bae::city::beast::RequestConcept _Request>
    void operator()(_Request &&request)
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

int main(int argc, const char **argv)
{
    if (argc != 5)
    {
        std::cerr <<
            "Usage: run_app <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    run_app 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }

    auto const address = argv[1];
    auto const port = std::atoi(argv[2]);
    auto const document_root = argv[3];
    auto const thread_count = std::max<int>(1, std::atoi(argv[4]));

#ifdef HAS_CERT
    MySecurity security;
    bae::city::beast::SecureConfig<MySecurity> config{security, address, port, thread_count};
    MyServer server{document_root};
    bae::city::beast::Service<MyServer> service{server};
    return service(config);
#endif
}
