#ifndef INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP
#define INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP

#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>


namespace bae::city::beast {

    template<typename T>
    concept SslCertificateLoaderConcept =
        requires(T &x, const T &c) {
            { x.load() }; //< should load cert, key, dh into itself
            // observers
            { c.cert() } -> std::convertible_to<const std::string &>;
            { c.key() } -> std::convertible_to<const std::string &>;
            { c.dh() } -> std::convertible_to<const std::string &>;
            // produce password
            { c.get_password() } -> std::convertible_to<std::string>;
        };

    template<typename T>
    concept ServiceConfigConcept =
        requires(T x) {
            typename std::remove_cvref_t<T>::Context;
            { x.configure(std::declval<typename std::remove_cvref_t<T>::Context&>())};
        };

    template<typename T>
    concept RequestConcept = 
        requires(T x) {
            { x.Send(x.BadRequest(std::declval<std::string>())) };
            { x.Send(x.NotFound(std::declval<std::string>())) };
            { x.Send(x.ServerError(std::declval<std::string>())) };
            { x.Send(x.Success()) };
            { x.Send(x.TextResponse(std::declval<std::string>())) };
            { x.Send(x.FileResponse(
                std::declval<typename std::remove_cvref_t<T>::FileBody>(),
                std::declval<std::string>())) };
            { x.request() } -> std::convertible_to<typename std::remove_cvref_t<T>::RequestType>;
        };
    
    template<typename T, typename R>
    concept ServerConcept =
        requires(T x) {
            { x(std::declval<R>()) };
        };


} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP