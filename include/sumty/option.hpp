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

#ifndef SUMTY_OPTION_HPP
#define SUMTY_OPTION_HPP

#include "sumty/detail/fwd.hpp"
#include "sumty/detail/traits.hpp"
#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/utils.hpp"
#include "sumty/variant.hpp"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

namespace sumty {

template <typename T>
class option {
  private:
    variant<void, T> opt_{};

  public:
    using value_type = typename detail::traits<T>::value_type;
    using reference = typename detail::traits<T>::reference;
    using const_reference = typename detail::traits<T>::const_reference;
    using rvalue_reference = typename detail::traits<T>::rvalue_reference;
    using const_rvalue_reference =
        typename detail::traits<T>::const_rvalue_reference;
    using pointer = typename detail::traits<T>::pointer;
    using const_pointer = typename detail::traits<T>::const_pointer;

    constexpr option() noexcept = default;

    constexpr option(const option&) noexcept(
        detail::traits<T>::is_nothrow_copy_constructible) = default;

    constexpr option(option&&) noexcept(
        detail::traits<T>::is_nothrow_move_constructible) = default;

    constexpr option([[maybe_unused]] none_t none) noexcept;

    constexpr option([[maybe_unused]] std::nullopt_t null) noexcept;

    constexpr option([[maybe_unused]] std::nullptr_t null) noexcept
        requires(std::is_lvalue_reference_v<T>);

    template <typename U>
        requires(std::is_lvalue_reference_v<T> &&
                 std::is_convertible_v<U*, pointer>)
    explicit(!std::is_convertible_v<U*, pointer>) constexpr option(
        U* ptr) noexcept;

    template <typename U>
        requires(std::is_constructible_v<
                 variant<void, T>,
                 std::in_place_index_t<1>,
                 typename detail::traits<U>::const_reference>)
    explicit(!detail::traits<T>::template is_convertible_from<
             U>) constexpr option(const option<U>& other);

    template <typename U>
        requires(std::is_constructible_v<
                 variant<void, T>,
                 std::in_place_index_t<1>,
                 typename detail::traits<U>::rvalue_reference>)
    explicit(!detail::traits<T>::template is_convertible_from<
             U>) constexpr option(option<U>&& other);

    template <typename... Args>
    explicit(sizeof...(Args) ==
             0) constexpr option([[maybe_unused]] std::in_place_t in_place,
                                 Args&&... args);

    template <typename U, typename... Args>
    constexpr option([[maybe_unused]] std::in_place_t in_place,
                     std::initializer_list<U> init,
                     Args&&... args);

    template <typename U>
        requires(std::is_constructible_v<variant<void, T>,
                                         std::in_place_index_t<1>,
                                         U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 (!std::is_same_v<std::remove_const_t<T>, bool> ||
                  !detail::is_option_v<U>))
    explicit(!detail::traits<T>::template is_convertible_from<
             U>) constexpr option(U&& value);

    constexpr ~option() noexcept(detail::traits<T>::is_nothrow_destructible) =
        default;

    constexpr option& operator=(const option&) noexcept(
        detail::traits<T>::is_nothrow_copy_assignable&&
            detail::traits<T>::is_nothrow_copy_constructible&&
                detail::traits<T>::is_nothrow_destructible) = default;

    constexpr option& operator=(option&&) noexcept(
        detail::traits<T>::is_nothrow_move_assignable&&
            detail::traits<T>::is_nothrow_move_constructible&&
                detail::traits<T>::is_nothrow_destructible) = default;

    constexpr option& operator=([[maybe_unused]] none_t none) noexcept(
        detail::traits<T>::is_nothrow_destructible);

    constexpr option& operator=([[maybe_unused]] std::nullopt_t null) noexcept(
        detail::traits<T>::is_nothrow_destructible);

    constexpr option& operator=([[maybe_unused]] std::nullptr_t null) noexcept
        requires(std::is_lvalue_reference_v<T>);

    template <typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, option<T>> &&
                 std::is_constructible_v<variant<void, T>,
                                         std::in_place_index_t<1>,
                                         U &&> &&
                 detail::traits<T>::template is_assignable<U &&> &&
                 (!std::is_scalar_v<value_type> ||
                  !std::is_same_v<T, std::decay_t<U>>))
    constexpr option& operator=(U&& value);

    template <typename U>
        requires(std::is_lvalue_reference_v<value_type> &&
                 std::is_convertible_v<U*, pointer>)
    constexpr option& operator=(U* ptr) noexcept;

    template <typename U>
        requires(
            !detail::traits<T>::template is_constructible<option<U>&> &&
            !detail::traits<T>::template is_constructible<const option<U>&> &&
            !detail::traits<T>::template is_constructible<option<U> &&> &&
            !detail::traits<T>::template is_constructible<const option<U> &&> &&
            !detail::traits<T>::template is_convertible_from<option<U>&> &&
            !detail::traits<T>::template is_convertible_from<
                const option<U>&> &&
            !detail::traits<T>::template is_convertible_from<option<U> &&> &&
            !detail::traits<T>::template is_convertible_from<
                const option<U> &&> &&
            !detail::traits<T>::template is_assignable<option<U>&> &&
            !detail::traits<T>::template is_assignable<const option<U>&> &&
            !detail::traits<T>::template is_assignable<option<U> &&> &&
            !detail::traits<T>::template is_assignable<const option<U> &&> &&
            (std::is_lvalue_reference_v<T> ||
             detail::traits<T>::template is_constructible<
                 typename detail::traits<U>::const_reference>) &&
            detail::traits<T>::template is_assignable<
                typename detail::traits<U>::const_reference>)
    constexpr option& operator=(const option<U>& value);

    template <typename U>
        requires(
            !detail::traits<T>::template is_constructible<option<U>&> &&
            !detail::traits<T>::template is_constructible<const option<U>&> &&
            !detail::traits<T>::template is_constructible<option<U> &&> &&
            !detail::traits<T>::template is_constructible<const option<U> &&> &&
            !detail::traits<T>::template is_convertible_from<option<U>&> &&
            !detail::traits<T>::template is_convertible_from<
                const option<U>&> &&
            !detail::traits<T>::template is_convertible_from<option<U> &&> &&
            !detail::traits<T>::template is_convertible_from<
                const option<U> &&> &&
            !detail::traits<T>::template is_assignable<option<U>&> &&
            !detail::traits<T>::template is_assignable<const option<U>&> &&
            !detail::traits<T>::template is_assignable<option<U> &&> &&
            !detail::traits<T>::template is_assignable<const option<U> &&> &&
            (std::is_lvalue_reference_v<T> ||
             detail::traits<T>::template is_constructible<
                 typename detail::traits<U>::rvalue_reference>) &&
            detail::traits<T>::template is_assignable<
                typename detail::traits<U>::rvalue_reference>)
    constexpr option& operator=(option<U>&& value);

    constexpr operator bool() const noexcept;

    template <typename U>
        requires(std::is_lvalue_reference_v<T> &&
                 std::is_assignable_v<pointer&, U*>)
    explicit(!std::is_convertible_v<pointer, U*>) constexpr operator U*()
        const noexcept;

    constexpr bool has_value() const noexcept;

    constexpr reference operator*() & noexcept;

    constexpr const_reference operator*() const& noexcept;

    constexpr rvalue_reference operator*() &&;

    constexpr const_rvalue_reference operator*() const&&;

    constexpr pointer operator->() noexcept;

    constexpr const_pointer operator->() const noexcept;

    constexpr reference value() &;

    constexpr const_reference value() const&;

    constexpr rvalue_reference value() &&;

    constexpr rvalue_reference value() const&&;

    template <typename U>
    constexpr value_type value_or(U&& default_value) const&;

    template <typename U>
    constexpr value_type value_or(U&& default_value) &&;

    constexpr value_type value_or() const&;

    constexpr value_type value_or() &&;

    template <typename F>
    constexpr auto and_then(F&& f) &;

    template <typename F>
    constexpr auto and_then(F&& f) const&;

    template <typename F>
    constexpr auto and_then(F&& f) &&;

    template <typename F>
    constexpr auto and_then(F&& f) const&&;

    template <typename F>
    constexpr decltype(auto) transform(F&& f) &;

    template <typename F>
    constexpr decltype(auto) transform(F&& f) const&;

    template <typename F>
    constexpr decltype(auto) transform(F&& f) &&;

    template <typename F>
    constexpr decltype(auto) transform(F&& f) const&&;

    template <typename F>
    constexpr option or_else(F&& f) const&;

    template <typename F>
    constexpr option or_else(F&& f) &&;

    constexpr void swap(option& other) noexcept(
        noexcept(opt_.swap(other.opt_)));

    constexpr void reset() noexcept;

    template <typename... Args>
    constexpr reference emplace(Args&&... args);
};

template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
    requires(std::three_way_comparable_with<std::remove_cvref_t<U>,
                                            std::remove_cvref_t<T>>)
constexpr std::compare_three_way_result_t<std::remove_cvref_t<T>,
                                          std::remove_cvref_t<U>>
operator<=>(const option<T>& lhs, const option<U>& rhs);

template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const U& rhs);

template <typename T, typename U>
constexpr bool operator==(const U& lhs, const option<T>& rhs);

template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const U& rhs);

template <typename T, typename U>
constexpr bool operator!=(const U& lhs, const option<T>& rhs);

template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const U& rhs);

template <typename T, typename U>
constexpr bool operator<(const U& lhs, const option<T>& rhs);

template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const U& rhs);

template <typename T, typename U>
constexpr bool operator>(const U& lhs, const option<T>& rhs);

template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const U& rhs);

template <typename T, typename U>
constexpr bool operator<=(const U& lhs, const option<T>& rhs);

template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const U& rhs);

template <typename T, typename U>
constexpr bool operator>=(const U& lhs, const option<T>& rhs);

template <typename T, typename U>
    requires(std::three_way_comparable_with<std::remove_cvref_t<U>,
                                            std::remove_cvref_t<T>>)
constexpr std::compare_three_way_result_t<std::remove_cvref_t<T>,
                                          std::remove_cvref_t<U>>
operator<=>(const option<T>& lhs, const U& rhs);

template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator==([[maybe_unused]] none_t lhs, const option<T>& rhs);

template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator!=([[maybe_unused]] none_t lhs, const option<T>& rhs);

template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator<([[maybe_unused]] none_t lhs, const option<T>& rhs);

template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator>([[maybe_unused]] none_t lhs,
                         [[maybe_unused]] const option<T>& rhs);

template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator<=([[maybe_unused]] none_t lhs,
                          [[maybe_unused]] const option<T>& rhs);

template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator>=([[maybe_unused]] none_t lhs, const option<T>& rhs);

template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] none_t rhs);

template <typename T>
constexpr bool operator==(const option<T>& lhs,
                          [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
constexpr bool operator==([[maybe_unused]] std::nullopt_t lhs,
                          const option<T>& rhs);

template <typename T>
constexpr bool operator!=(const option<T>& lhs,
                          [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
constexpr bool operator!=([[maybe_unused]] std::nullopt_t lhs,
                          const option<T>& rhs);

template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
constexpr bool operator<([[maybe_unused]] std::nullopt_t lhs,
                         const option<T>& rhs);

template <typename T>
constexpr bool operator>(const option<T>& lhs,
                         [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
constexpr bool operator>([[maybe_unused]] std::nullopt_t lhs,
                         [[maybe_unused]] const option<T>& rhs);

template <typename T>
constexpr bool operator<=(const option<T>& lhs,
                          [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
constexpr bool operator<=([[maybe_unused]] std::nullopt_t lhs,
                          [[maybe_unused]] const option<T>& rhs);

template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
constexpr bool operator>=([[maybe_unused]] std::nullopt_t lhs,
                          const option<T>& rhs);

template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] std::nullopt_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator==(const option<T>& lhs,
                          [[maybe_unused]] std::nullptr_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator==([[maybe_unused]] std::nullptr_t lhs,
                          const option<T>& rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator!=(const option<T>& lhs,
                          [[maybe_unused]] std::nullptr_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator!=([[maybe_unused]] std::nullptr_t lhs,
                          const option<T>& rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] std::nullptr_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<([[maybe_unused]] std::nullptr_t lhs,
                         const option<T>& rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>(const option<T>& lhs,
                         [[maybe_unused]] std::nullptr_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>([[maybe_unused]] std::nullptr_t lhs,
                         [[maybe_unused]] const option<T>& rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<=(const option<T>& lhs,
                          [[maybe_unused]] std::nullptr_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<=([[maybe_unused]] std::nullptr_t lhs,
                          [[maybe_unused]] const option<T>& rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] std::nullptr_t rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>=([[maybe_unused]] std::nullptr_t lhs,
                          const option<T>& rhs);

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] std::nullptr_t rhs);

template <typename T, typename... Args>
constexpr option<T> some(Args&&... args);

template <typename T, typename U, typename... Args>
constexpr option<T> some(std::initializer_list<U> ilist, Args&&... args);

} // namespace sumty

#include "sumty/impl/option.hpp"

#endif
