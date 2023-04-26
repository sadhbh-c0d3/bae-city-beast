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
            // configure (SSL) context
            { x.configure(std::declval<typename std::remove_cvref_t<T>::Context&>())};
        };

    template<typename T>
    concept LoggerConcept =
        requires(T x) {
            { x.info(std::declval<std::string>()) };
            { x.error(std::declval<std::string>()) };
        };

    template<typename T>
    concept RequestConcept = 
        requires(T x) {
            // should define type aliases
            typename std::remove_cvref_t<T>::RequestType;
            typename std::remove_cvref_t<T>::BufferType;
            // should be able to send(*) response produced by *
            { x.send(x.bad_request(std::declval<std::string>())) };
            { x.send(x.not_found(std::declval<std::string>())) };
            { x.send(x.server_error(std::declval<std::string>())) };
            { x.send(x.success()) };
            { x.send(x.text_response(std::declval<std::string>())) };
            { x.send(x.file_response(
                std::declval<typename std::remove_cvref_t<T>::FileBody>(),
                std::declval<std::string>())) };
            // should provide access to underlying boost::beast::http::request<>
            { x.request() } -> std::convertible_to<typename std::remove_cvref_t<T>::RequestType>;
            // ...we also require that underlying request type has method() and target()
            { x->method() } -> auto;
            { (*x).target() } -> auto;
            // and shorthand for underlying operator []
            { x[std::declval<std::string>()] } -> auto;
        };

    template<typename T, typename L>
    concept RequestFactoryConcept =
        requires(T x) {
            LoggerConcept<L>;
            // should produce type implementing RequestConcept
            RequestConcept<typename T::Request<L>>;
        };
    
    template<typename T, typename R>
    concept ServerConcept =
        requires(T x) {
            // should define type alias
            RequestConcept<R>;
            // and should provide operator (R) for request type R
            { x(std::declval<R>()) };
        };


} //namespace bae::city::beast
#endif//INCLUDED_BAE_CITY_BEAST_CONCEPTS_HPP