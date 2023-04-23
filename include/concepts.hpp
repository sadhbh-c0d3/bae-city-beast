#ifndef INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP
#define INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP

#include <concepts>
#include <type_traits>
#include <string>


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
            typename std::remove_cvref_t<T>::ResponseExample;
            { x.BadRequest() };
            { x.NotFound() };
            { x.ServerError() };
            { x.StatusCode() };
            { x.TextResponse() };
            { x.FileResponse() };
            { x.Send(std::declval<std::remove_cvref_t<T>::ResponseExample>()) };
        };
    
    template<typename T>
    concept ServerConcept = true;
    //    requires(T x) {
    //        { x(std::declval<_Request>()) };
    //    };


} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP