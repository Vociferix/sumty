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
    constexpr typename detail::traits<detail::select_t<I, T...>>::reference emplace(
        Args&&... args);

    template <size_t I, typename U, typename... Args>
    constexpr typename detail::traits<detail::select_t<I, T...>>::reference emplace(
        std::initializer_list<U> ilist,
        Args&&... args);

    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr typename detail::traits<U>::reference emplace(Args&&... args);

    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr typename detail::traits<U>::reference emplace(std::initializer_list<V> ilist,
                                                            Args&&... args);

    template <size_t I>
    [[nodiscard]] constexpr typename detail::traits<detail::select_t<I, T...>>::reference
    operator[](index_t<I> index) & noexcept;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_reference
        operator[](index_t<I> index) const& noexcept;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
        operator[](index_t<I> index) &&;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
        operator[](index_t<I> index) const&&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::reference operator[](
        [[maybe_unused]] type_t<U> type) & noexcept {
        return this->operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_reference operator[](
        [[maybe_unused]] type_t<U> type) const& noexcept {
        return this->operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::rvalue_reference operator[](
        [[maybe_unused]] type_t<U> type) && {
        return std::move(*this).operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_rvalue_reference operator[](
        [[maybe_unused]] type_t<U> type) const&& {
        return std::move(*this).operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    template <size_t I>
    [[nodiscard]] constexpr typename detail::traits<detail::select_t<I, T...>>::reference
    get() &;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_reference
        get() const&;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
        get() &&;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
        get() const&&;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::reference get() & {
        return this->template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_reference get() const& {
        return this->template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::rvalue_reference get() && {
        return std::move(*this).template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_rvalue_reference get()
        const&& {
        return std::move(*this).template get<detail::index_of_v<U, T...>>();
    }

    template <size_t I>
    [[nodiscard]] constexpr typename detail::traits<detail::select_t<I, T...>>::pointer
    get_if() noexcept;

    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_pointer
        get_if() const noexcept;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::pointer get_if() noexcept;

    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_pointer get_if()
        const noexcept;

    template <typename U>
    [[nodiscard]] constexpr bool holds_alternative() const noexcept;

    template <typename V>
    constexpr detail::
        invoke_result_t<V&&, typename detail::traits<detail::select_t<0, T...>>::reference>
        visit(V&& visitor) &;

    template <typename V>
    constexpr detail::invoke_result_t<
        V&&,
        typename detail::traits<detail::select_t<0, T...>>::const_reference>
    visit(V&& visitor) const&;

    template <typename V>
    constexpr detail::invoke_result_t<
        V&&,
        typename detail::traits<detail::select_t<0, T...>>::rvalue_reference>
    visit(V&& visitor) &&;

    template <typename V>
    constexpr detail::invoke_result_t<
        V&&,
        typename detail::traits<detail::select_t<0, T...>>::const_rvalue_reference>
    visit(V&& visitor) const&&;

    constexpr void swap(variant& other) noexcept(noexcept(data_.swap(other.data_)));
};

template <typename T, typename... U>
constexpr bool holds_alternative(const variant<U...>& v) noexcept;

template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::reference get(
    variant<T...>& v);

template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::const_reference get(
    const variant<T...>& v);

template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::rvalue_reference get(
    variant<T...>&& v);

template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference get(
    const variant<T...>&& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::reference get(variant<U...>& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::const_reference get(const variant<U...>& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::rvalue_reference get(variant<U...>&& v);

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::const_rvalue_reference get(const variant<U...>&& v);

template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::pointer get_if(
    variant<T...>& v) noexcept;

template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::const_pointer get_if(
    const variant<T...>& v) noexcept;

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::pointer get_if(variant<U...>& v) noexcept;

template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::const_pointer get_if(const variant<U...>& v) noexcept;

template <typename V>
constexpr std::invoke_result_t<V&&> visit(V&& visitor);

template <typename V, typename T0, typename... TN>
constexpr detail::invoke_result_t<V&&,
                                  decltype(get<0>(std::declval<T0&&>())),
                                  decltype(get<0>(std::declval<TN&&>()))...>
visit(V&& visitor, T0&& var0, TN&&... varn);

template <typename... T>
constexpr void swap(variant<T...>& a, variant<T...>& b);

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
