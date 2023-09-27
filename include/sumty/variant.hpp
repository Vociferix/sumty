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

#include "sumty/detail/fwd.hpp"    // IWYU pragma: export
#include "sumty/detail/traits.hpp" // IWYU pragma: export
#include "sumty/detail/utils.hpp"
#include "sumty/detail/variant_impl.hpp"
#include "sumty/utils.hpp"

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace sumty {

template <typename... T>
class variant {
  private:
    SUMTY_NO_UNIQ_ADDR detail::variant_impl<void, T...> data_;

    template <size_t IDX, typename V, typename U>
    static constexpr decltype(auto) jump_table_entry(V&& visitor, U&& var);

    template <typename V, typename U, size_t... IDX>
    static consteval auto jump_table(std::index_sequence<IDX...> seq);

    template <typename V, typename U>
    static consteval auto jump_table() noexcept;

    template <typename V, typename U>
    static constexpr decltype(auto) visit_impl(V&& visitor, U&& var);

    template <size_t IDX, typename U>
    [[nodiscard]] constexpr bool holds_alt_impl() const noexcept;

  public:
    constexpr variant() noexcept(
        detail::traits<detail::first_t<T...>>::is_nothrow_default_constructible)
        requires(detail::traits<detail::first_t<T...>>::is_default_constructible)
    = default;

    constexpr variant(const variant&)
        requires(true && ... && detail::traits<T>::is_copy_constructible)
    = default;

    constexpr variant(variant&&) noexcept(
        (true && ... && detail::traits<T>::is_nothrow_move_constructible))
        requires(true && ... && detail::traits<T>::is_move_constructible)
    = default;

    template <size_t IDX, typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr variant(std::in_place_index_t<IDX> inplace, Args&&... args);

    template <size_t IDX, typename U, typename... Args>
    constexpr variant(std::in_place_index_t<IDX> inplace,
                      std::initializer_list<U> init,
                      Args&&... args);

    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr variant(std::in_place_type_t<U> inplace, Args&&... args);

    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr variant(std::in_place_type_t<U> inplace,
                      std::initializer_list<V> init,
                      Args&&... args);

    constexpr ~variant() noexcept((true && ... &&
                                   detail::traits<T>::is_nothrow_destructible)) = default;

    constexpr variant& operator=(const variant& rhs)
        requires(true && ... && detail::traits<T>::is_copy_assignable)
    = default;

    constexpr variant& operator=(variant&& rhs) noexcept(
        (true && ... &&
         (detail::traits<T>::is_nothrow_move_assignable &&
          detail::traits<T>::is_nothrow_destructible &&
          detail::traits<T>::is_nothrow_move_constructible)))
        requires(true && ... && detail::traits<T>::is_move_assignable)
    = default;

    [[nodiscard]] constexpr size_t index() const noexcept;

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
    [[nodiscard]] constexpr decltype(auto) operator[](index_t<I> index) & noexcept;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) operator[](index_t<I> index) const& noexcept;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) operator[](index_t<I> index) &&;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) operator[](index_t<I> index) const&&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) operator[](type_t<U> type) & noexcept;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) operator[](type_t<U> type) const& noexcept;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) operator[](type_t<U> type) &&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) operator[](type_t<U> type) const&&;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) get() &;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) get() const&;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) get() &&;

    template <size_t I>
    [[nodiscard]] constexpr decltype(auto) get() const&&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) get() &;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) get() const&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) get() &&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr decltype(auto) get() const&&;

    template <size_t I>
    [[nodiscard]] constexpr auto get_if() noexcept;

    template <size_t I>
    [[nodiscard]] constexpr auto get_if() const noexcept;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr auto get_if() noexcept;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr auto get_if() const noexcept;

    template <typename U>
    [[nodiscard]] constexpr bool holds_alternative() const noexcept;

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

template <size_t I, typename... T>
constexpr decltype(auto) get(variant<T...>& v);

template <size_t I, typename... T>
constexpr decltype(auto) get(const variant<T...>& v);

template <size_t I, typename... T>
constexpr decltype(auto) get(variant<T...>&& v);

template <size_t I, typename... T>
constexpr decltype(auto) get(const variant<T...>&& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr decltype(auto) get(variant<U...>& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr decltype(auto) get(const variant<U...>& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr decltype(auto) get(variant<U...>&& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr decltype(auto) get(const variant<U...>&& v);

template <size_t I, typename... T>
constexpr auto get_if(variant<T...>& v) noexcept;

template <size_t I, typename... T>
constexpr auto get_if(const variant<T...>& v) noexcept;

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr auto get_if(variant<U...>& v) noexcept;

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr auto get_if(const variant<U...>& v) noexcept;

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
struct variant_alternative<I, const variant<T...>> : variant_alternative<I, variant<T...>> {
};

template <size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

} // namespace sumty

#include "sumty/impl/variant.hpp" // IWYU pragma: export

#endif
