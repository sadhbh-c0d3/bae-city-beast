
namespace bae::city::beast
{
    struct ServiceConfig
    {
        struct Context; //< boost::asio::ssl::context

        template<typename _Context>
        void configure(_Context &&);
    };

    template <template <typename> typename _Handler>
    struct ServiceFactory
    {
        template <typename _Body, typename _Allocator, typename _Send>
        struct Request
        {
            template<typename T = int>
            struct Response; //< boost::beast::http::response<T>

            Response<> BadRequest();
            Response<> NotFound();
            Response<> ServerError();

            template<typename _Response>
            void Send(_Response &&);

            template<typename _RequestHandler>
            void AcceptHandler(_RequestHandler &&);
        };

        struct Service
        {
            int operator()();
        };

        template<typename _ServiceConfig>
        Service create_service(_ServiceConfig &&);
    };

}; // namespace bae_city_beast

namespace bae::city::gal
{
    template <typename _Request>
    struct RequestHandler
    {
        using Request = _Request;
        
        template<typename T>
        using Response = typename Request::Response<T>;

        void operator()(Request &req);
    };

}; // namespace bae::gal

int main(int argc, const char **argv)
{
    bae::city::beast::ServiceConfig config;

    // Configure using command line arguments: argv[...] --> config....

    bae::city::beast::ServiceFactory<bae::city::gal::RequestHandler> service_factory;

    auto service = service_factory.create_service(config);

    return service();
}