
namespace bae::city::beast
{
    template<typename T>
    concept ServiceConfigConcept =
        requires(T &x) {
            { typename T::ContextExample };
            { x.configure(std::declval<typename T::ContextExample&>())};
        };

    template<typename T>
    concept RequestConcept =
        requires(T &x) {
            { typename T::ResponseExample };
            { x.BadResponse() };
            { x.NotFound() };
            { x.ServerError() };
            { x.StatusCode() };
            { x.TextResponse() };
            { x.FileResponse() };
            { x.Send(std::declval<ResponseExample&>()) };
        };
    
    template<typename T>
    concept ServerConcept =
        requires(T &x) {
            RequestConcept<typename T::RequestExample>;
            { x(std::declval<typename T::RequestExample&>()); }
        };

    template<typename T>
    concept VenueConcept =
        requires(T &x) {
            ServerConcept<typename T::ServerExample>;
        };

    struct SecureServiceConfig
    {
        struct Context; //< boost::asio::ssl::context

        using ContextExample = Context;

        template<typename _Context>
        void configure(_Context &&);
    };

    template <VenueConcept _Venue>
    struct ServiceFactory
    {
        template <typename _Body, typename _Allocator, typename _Send>
        struct Request
        {
            template<typename T = int>
            struct Response; //< boost::beast::http::response<T>

            using ResponseExample = Response<int>;

            Response<> BadRequest();
            Response<> NotFound();
            Response<> ServerError();
            Response<> StatusCode();
            Response<> TextResponse();
            Response<> FileResponse();

            template<typename _Response>
            void Send(_Response &&);

            template<ServerConcept _Server>
            void AcceptServer(_Server &&);
        };

        using RequestExample = Request<int, int, int>;

        struct Service
        {
            int operator()();
        };

        template<ServiceConfigConcept _ServiceConfig>
        Service create_service(_ServiceConfig &&);
    };

}; // namespace bae_city_beast

namespace bae::city
{
    struct MyVenue
    {
        template <RequestConcept _Request>
        struct Server
        {
            using Request = _Request;

            void operator()(Request &req);
        };

        using ServiceFactory = bae::city::beast::ServiceFactory<Venue>;
        using RequestExample = typename ServiceFactory::RequestExample;
        using ServerExample = Server<RequestExample>;
    };

}; // namespace bae::gal

int main(int argc, const char **argv)
{
    bae::city::MyVenue venue;
    bae::city::beast::SecureServiceConfig config;

    // Configure using command line arguments: argv[...] --> config....

    bae::city::beast::ServiceFactory<bae::city::Venue> service_factory{venue};

    auto service = service_factory.create_service(config);

    return service();
}