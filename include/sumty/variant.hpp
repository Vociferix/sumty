/* Copyright 2023 Jack A Bernard Jr.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SUMTY_VARIANT_HPP
#define SUMTY_VARIANT_HPP

#include "sumty/detail/fwd.hpp"
#include "sumty/detail/traits.hpp"
#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/utils.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <variant> // for std::variant_size and std::variant_alternative

namespace sumty {

namespace detail {

template <typename Enable, typename... T>
class variant_impl;

template <typename... T>
using variant_impl_t = variant_impl<void, T...>;

}

template <typename... T>
class variant {
  private:
    SUMTY_NO_UNIQ_ADDR detail::variant_impl_t<T...> data_;

    template <size_t IDX, typename V, typename U>
    static constexpr decltype(auto) visit_impl(V&& visitor, U&& var);

    template <size_t IDX, typename U>
    constexpr bool holds_alt_impl() const noexcept;

  public:
    constexpr variant() noexcept(detail::traits<detail::first_t<T...>>::is_nothrow_default_constructible) requires(detail::traits<detail::first_t<T...>>::is_default_constructible) = default;

    constexpr variant(const variant&) requires(true && ... && detail::traits<T>::is_copy_constructible) = default;

    constexpr variant(variant&&) noexcept((true && ... && detail::traits<T>::is_nothrow_move_constructible)) requires(true && ... && detail::traits<T>::is_move_constructible) = default;

    template <size_t IDX, typename... Args>
    constexpr variant(std::in_place_index_t<IDX> in_place, Args&&... args);

    template <size_t IDX, typename U, typename... Args>
    constexpr variant(std::in_place_index_t<IDX> in_place, std::initializer_list<U> init, Args&&... args);

    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr variant([[maybe_unused]] std::in_place_type_t<U> in_place, Args&&... args);

    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr variant([[maybe_unused]] std::in_place_type_t<U> in_place, std::initializer_list<V> init, Args&&... args);

    constexpr ~variant() noexcept((true && ... && detail::traits<T>::is_nothrow_destructible)) = default;

    constexpr variant& operator=(const variant& rhs)
        requires(true && ... && detail::traits<T>::is_copy_assignable)
         = default;

    constexpr variant& operator=(variant&& rhs)
        noexcept((true && ... && (detail::traits<T>::is_nothrow_move_assignable && detail::traits<T>::is_nothrow_destructible && detail::traits<T>::is_nothrow_move_constructible)))
        requires(true && ... && detail::traits<T>::is_move_assignable)
        = default;

    constexpr size_t index() const noexcept;

    template <size_t I, typename... Args>
    constexpr decltype(auto) emplace(Args&&... args);

    template <size_t I, typename U, typename... Args>
    constexpr decltype(auto) emplace(std::initializer_list<U> ilist, Args&&... args);

    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) emplace(Args&&... args);

    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) emplace(std::initializer_list<V> ilist, Args&&... args);

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) & noexcept;

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) const& noexcept;

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) &&;

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) const&&;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) & noexcept;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) const& noexcept;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) &&;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) const&&;

    template <size_t I>
    constexpr decltype(auto) get() &;

    template <size_t I>
    constexpr decltype(auto) get() const&;

    template <size_t I>
    constexpr decltype(auto) get() &&;

    template <size_t I>
    constexpr decltype(auto) get() const&&;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() &;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() const&;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() &&;

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() const&&;

    template <typename U>
    constexpr bool holds_alternative() const noexcept;

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) &;

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) const&;

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) &&;

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) const&&;

    constexpr void swap(variant& other) noexcept(noexcept(data_.swap(other.data_)));
};

template <typename T, typename... U>
constexpr bool holds_alternative(const variant<U...>& v) noexcept;

template <typename T, typename... U>
constexpr decltype(auto) get(variant<U...>& v);

template <typename T, typename... U>
constexpr decltype(auto) get(const variant<U...>& v);

template <typename T, typename... U>
constexpr decltype(auto) get(variant<U...>&& v);

template <typename T, typename... U>
constexpr decltype(auto) get(const variant<U...>&& v);

template <typename V>
constexpr decltype(auto) visit(V&& visitor);

template <typename V, typename T0, typename... TN>
constexpr decltype(auto) visit(V&& visitor, T0&& var0, TN&&... varn);

template <typename T>
struct variant_size;

template <typename... T>
struct variant_size<variant<T...>> : std::integral_constant<size_t, sizeof...(T)> {};

template <typename... T>
struct variant_size<const variant<T...>> : variant_size<variant<T...>> {};

template <typename T>
static inline constexpr size_t variant_size_v = variant_size<T>::value;

template <size_t I, typename T>
struct variant_alternative;

template <size_t I, typename... T>
struct variant_alternative<I, variant<T...>> {
    using type = detail::select_t<I, T...>;
};

template <size_t I, typename... T>
struct variant_alternative<I, const variant<T...>> : variant_alternative<I, variant<T...>> {};

template <size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

}

template <typename... T>
struct std::variant_size<::sumty::variant<T...>> : ::sumty::variant_size<::sumty::variant<T...>> {};

template <size_t I, typename... T>
struct std::variant_alternative<I, ::sumty::variant<T...>> : ::sumty::variant_alternative<I, ::sumty::variant<T...>> {};

#include "sumty/impl/variant.hpp"

#endif
