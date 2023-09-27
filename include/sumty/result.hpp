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

#ifndef SUMTY_RESULT_HPP
#define SUMTY_RESULT_HPP

#include "sumty/detail/fwd.hpp"    // IWYU pragma: export
#include "sumty/detail/traits.hpp" // IWYU pragma: export
#include "sumty/utils.hpp"         // IWYU pragma: export
#include "sumty/variant.hpp"

// IWYU pragma: no_include "sumty/option.hpp"

#include <initializer_list>
#include <type_traits>
#include <utility>

namespace sumty {

template <typename T>
class ok_t;

template <typename E>
class error_t;

template <typename T, typename E>
class result {
  private:
    variant<T, E> res_;

    template <typename R, typename U, typename V>
    static constexpr variant<U, V> convert(R&& res);

  public:
    using value_type = typename detail::traits<T>::value_type;
    using reference = typename detail::traits<T>::reference;
    using const_reference = typename detail::traits<T>::const_reference;
    using rvalue_reference = typename detail::traits<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<T>::const_rvalue_reference;
    using pointer = typename detail::traits<T>::pointer;
    using const_pointer = typename detail::traits<T>::const_pointer;

    using error_type = typename detail::traits<E>::value_type;
    using error_reference = typename detail::traits<E>::reference;
    using error_const_reference = typename detail::traits<E>::const_reference;
    using error_rvalue_reference = typename detail::traits<E>::rvalue_reference;
    using error_const_rvalue_reference = typename detail::traits<E>::const_rvalue_reference;
    using error_pointer = typename detail::traits<E>::pointer;
    using error_const_pointer = typename detail::traits<E>::const_pointer;

    template <typename U>
    using rebind = result<U, E>;

    template <typename V>
    using rebind_error = result<T, V>;

    // For compatibility with std::expected
    using unexpected_type = error_t<E>;

    constexpr result() noexcept(std::is_nothrow_default_constructible_v<variant<T, E>>)
        requires(std::is_default_constructible_v<variant<T, E>>)
    = default;

    constexpr result(const result&) noexcept(
        std::is_nothrow_copy_constructible_v<variant<T, E>>)
        requires(std::is_copy_constructible_v<variant<T, E>>)
    = default;

    constexpr result(result&&) noexcept(std::is_nothrow_move_constructible_v<variant<T, E>>)
        requires(std::is_move_constructible_v<variant<T, E>>)
    = default;

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(std::in_place_t inplace, Args&&... args);

    template <typename U, typename... Args>
    constexpr result(std::in_place_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args);

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(std::in_place_index_t<0> inplace, Args&&... args);

    template <typename U, typename... Args>
    constexpr result(std::in_place_index_t<0> inplace,
                     std::initializer_list<U> init,
                     Args&&... args);

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(in_place_error_t inplace, Args&&... args);

    template <typename U, typename... Args>
    constexpr result(in_place_error_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args);

    template <typename U>
        requires(std::is_constructible_v<variant<T, E>, std::in_place_index_t<0>, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<0>> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<1>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(!detail::traits<T>::template is_convertible_from<U&&>)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(U&& value);

    template <typename U>
    explicit(!detail::traits<T>::template is_convertible_from<U&&>)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(ok_t<U> ok);

    template <typename U>
    explicit(!detail::traits<E>::template is_convertible_from<U&&>)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(error_t<U> err);

    template <typename U, typename V>
        requires(((std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<0>,
                                          typename detail::traits<U>::const_reference>) &&
                 ((std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<1>,
                                          typename detail::traits<E>::const_reference>))
    explicit((!std::is_void_v<U> && !detail::traits<T>::template is_convertible_from<U>) ||
             (!std::is_void_v<V> && !detail::traits<E>::template is_convertible_from<V>))
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(const result<U, V>& other);

    template <typename U, typename V>
        requires(((std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<0>,
                                          typename detail::traits<U>::rvalue_reference>) &&
                 ((std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<1>,
                                          typename detail::traits<E>::rvalue_reference>))
    explicit((!std::is_void_v<U> && !detail::traits<T>::template is_convertible_from<U>) ||
             (!std::is_void_v<V> && !detail::traits<E>::template is_convertible_from<V>))
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(result<U, V>&& other);

    constexpr ~result() noexcept(std::is_nothrow_destructible_v<variant<T, E>>) = default;

    constexpr result& operator=(const result&) noexcept(
        std::is_nothrow_copy_assignable_v<variant<T, E>>)
        requires(std::is_copy_assignable_v<variant<T, E>>)
    = default;

    constexpr result& operator=(result&&) noexcept(
        std::is_nothrow_move_assignable_v<variant<T, E>>)
        requires(std::is_move_assignable_v<variant<T, E>>)
    = default;

  private:
    template <typename U>
    static inline constexpr bool assign_value_req =
        !std::is_same_v<result, std::remove_cvref_t<U>> &&
        !detail::is_error_v<std::remove_cvref_t<U>> &&
        !detail::is_ok_v<std::remove_cvref_t<U>> &&
        detail::traits<T>::template is_constructible<U> &&
        detail::traits<T>::template is_assignable<U>;

    template <typename U>
    constexpr void assign_value(U&& value);

  public:
    template <typename U>
        requires assign_value_req<U>
    constexpr result<T, E>& operator=(U&& value) {
        assign_value(std::forward<U>(value));
        return *this;
    }

    template <typename U>
        requires((detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::const_reference> &&
                  detail::traits<T>::template is_assignable<
                      typename detail::traits<U>::const_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
    constexpr result& operator=(const ok_t<U>& value);

    template <typename U>
        requires((detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::rvalue_reference> &&
                  detail::traits<T>::template is_assignable<
                      typename detail::traits<U>::rvalue_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
    constexpr result& operator=(ok_t<U>&& value);

    template <typename V>
        requires((detail::traits<E>::template is_constructible<
                      typename detail::traits<V>::const_reference> &&
                  detail::traits<E>::template is_assignable<
                      typename detail::traits<V>::const_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
    constexpr result& operator=(const error_t<V>& error);

    template <typename V>
        requires((detail::traits<E>::template is_constructible<
                      typename detail::traits<V>::rvalue_reference> &&
                  detail::traits<E>::template is_assignable<
                      typename detail::traits<V>::rvalue_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
    constexpr result& operator=(error_t<V>&& error);

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator bool() const noexcept;

    [[nodiscard]] constexpr bool has_value() const noexcept;

    [[nodiscard]] constexpr reference operator*() & noexcept;

    [[nodiscard]] constexpr const_reference operator*() const& noexcept;

    [[nodiscard]] constexpr rvalue_reference operator*() &&;

    [[nodiscard]] constexpr const_rvalue_reference operator*() const&&;

    // cppcheck-suppress functionConst
    [[nodiscard]] constexpr pointer operator->() noexcept;

    [[nodiscard]] constexpr const_pointer operator->() const noexcept;

    [[nodiscard]] constexpr reference value() &;

    [[nodiscard]] constexpr const_reference value() const&;

    [[nodiscard]] constexpr rvalue_reference value() &&;

    [[nodiscard]] constexpr rvalue_reference value() const&&;

    // cppcheck-suppress functionConst
    [[nodiscard]] constexpr error_reference error() & noexcept;

    [[nodiscard]] constexpr error_const_reference error() const& noexcept;

    [[nodiscard]] constexpr error_rvalue_reference error() &&;

    [[nodiscard]] constexpr error_const_rvalue_reference error() const&&;

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) const&;

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) &&;

    [[nodiscard]] constexpr value_type value_or() const&;

    [[nodiscard]] constexpr value_type value_or() &&;

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
    constexpr auto or_else(F&& f) const&;

    template <typename F>
    constexpr auto or_else(F&& f) &&;

    template <typename F>
    constexpr decltype(auto) transform_error(F&& f) &;

    template <typename F>
    constexpr decltype(auto) transform_error(F&& f) const&;

    template <typename F>
    constexpr decltype(auto) transform_error(F&& f) &&;

    template <typename F>
    constexpr decltype(auto) transform_error(F&& f) const&&;

    [[nodiscard]] constexpr result<reference, error_reference> ref() noexcept;

    [[nodiscard]] constexpr result<const_reference, error_const_reference> ref()
        const noexcept;

    [[nodiscard]] constexpr result<const_reference, error_const_reference> cref()
        const noexcept;

    [[nodiscard]] constexpr option<T> or_none() const& noexcept;

    [[nodiscard]] constexpr option<T> or_none() &&;

    [[nodiscard]] constexpr option<E> error_or_none() const& noexcept;

    [[nodiscard]] constexpr option<E> error_or_none() &&;

    template <typename... Args>
    constexpr reference emplace(Args&&... args);

    template <typename U, typename... Args>
    constexpr reference emplace(std::initializer_list<U> ilist, Args&&... args);

    constexpr void swap(result& other) noexcept(std::is_nothrow_swappable_v<variant<T, E>>);
};

template <typename T, typename E, typename U, typename V>
    requires(std::is_void_v<T> == std::is_void_v<U> &&
             std::is_void_v<E> == std::is_void_v<V>)
constexpr bool operator==(const result<T, E>& lhs, const result<U, V>& rhs);

template <typename T, typename E, typename U>
constexpr bool operator==(const result<T, E>& lhs, const U& rhs);

template <typename T, typename E, typename V>
constexpr bool operator==(const result<T, E>& lhs, const error_t<V>& rhs);

template <typename T, typename E>
constexpr void swap(result<T, E>& a,
                    result<T, E>& b) noexcept(std::is_nothrow_swappable_v<variant<T, E>>);

template <typename E, typename... Args>
constexpr error_t<E> error(Args&&... args);

template <typename E, typename U, typename... Args>
constexpr error_t<E> error(std::initializer_list<U> ilist, Args&&... args);

template <typename T, typename... Args>
constexpr ok_t<T> ok(Args&&... args);

template <typename T, typename U, typename... Args>
constexpr ok_t<T> ok(std::initializer_list<U> ilist, Args&&... args);

template <typename E>
class error_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<E> err_;

  public:
    constexpr error_t() = default;

    constexpr error_t(const error_t&) = default;

    constexpr error_t(error_t&&) noexcept(
        detail::traits<E>::is_nothrow_move_constructible) = default;

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t(std::in_place_t inplace, Args&&... args);

    template <typename U, typename... Args>
    constexpr error_t(std::in_place_t inplace,
                      std::initializer_list<U> init,
                      Args&&... args);

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t(in_place_error_t inplace, Args&&... args);

    template <typename U, typename... Args>
    constexpr error_t(in_place_error_t inplace,
                      std::initializer_list<U> init,
                      Args&&... args);

    template <typename V>
        requires(std::is_constructible_v<variant<E>, std::in_place_index_t<0>, V &&> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_index_t<1>> &&
                 (!std::is_same_v<std::remove_cvref_t<E>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<V>>))
    explicit(detail::traits<E>::template convertible_from<V&&>)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t(V&& err);

    constexpr ~error_t() noexcept(detail::traits<E>::is_nothrow_destructible) = default;

    constexpr error_t& operator=(const error_t&) = default;

    constexpr error_t& operator=(error_t&&) noexcept(
        detail::traits<E>::is_nothrow_move_assignable) = default;

  private:
    template <typename V>
    static inline constexpr bool assign_value_req =
        !std::is_same_v<error_t, std::remove_cvref_t<V>> &&
        detail::traits<E>::template is_constructible<V> &&
        detail::traits<E>::template is_assignable<V>;

    template <typename V>
    constexpr void assign_value(V&& error);

  public:
    template <typename V>
        requires assign_value_req<V>
    constexpr error_t& operator=(V&& error) {
        assign_value(std::forward<V>(error));
    }

    constexpr typename detail::traits<E>::reference operator*() & noexcept;

    constexpr typename detail::traits<E>::const_reference operator*() const& noexcept;

    constexpr typename detail::traits<E>::rvalue_reference operator*() &&;

    constexpr typename detail::traits<E>::const_rvalue_reference operator*() const&&;

    // cppcheck-suppress functionConst
    constexpr typename detail::traits<E>::pointer operator->() noexcept;

    constexpr typename detail::traits<E>::const_pointer operator->() const noexcept;

    // cppcheck-suppress functionConst
    constexpr typename detail::traits<E>::reference error() & noexcept;

    constexpr typename detail::traits<E>::const_reference error() const& noexcept;

    constexpr typename detail::traits<E>::rvalue_reference error() &&;

    constexpr typename detail::traits<E>::const_rvalue_reference error() const&&;

    constexpr void swap(error_t& other) noexcept(std::is_nothrow_swappable_v<variant<E>>);
};

template <typename E, typename V>
    requires(std::is_void_v<E> == std::is_void_v<V>)
constexpr bool operator==(const error_t<E>& lhs, const error_t<V>& rhs);

template <typename E, typename V>
    requires(!std::is_void_v<E>)
constexpr bool operator==(const error_t<E>& lhs, const V& rhs);

template <typename E, typename V>
    requires(!std::is_void_v<V>)
constexpr bool operator==(const E& lhs, const error_t<V>& rhs);

template <typename E>
constexpr void swap(error_t<E>& a,
                    error_t<E>& b) noexcept(std::is_nothrow_swappable_v<variant<E>>);

template <typename T>
class ok_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<T> ok_;

  public:
    constexpr ok_t() = default;

    constexpr ok_t(const ok_t&) = default;

    constexpr ok_t(ok_t&&) noexcept(detail::traits<T>::is_nothrow_move_constructible) =
        default;

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr ok_t(std::in_place_t inplace, Args&&... args);

    template <typename U, typename... Args>
    constexpr ok_t(std::in_place_t inplace, std::initializer_list<U> init, Args&&... args);

    template <typename U>
        requires(std::is_constructible_v<variant<T>, std::in_place_index_t<0>, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(detail::traits<T>::template convertible_from<U&&>)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr ok_t(U&& value);

    constexpr ~ok_t() noexcept(detail::traits<T>::is_nothrow_destructible) = default;

    constexpr ok_t& operator=(const ok_t&) = default;

    constexpr ok_t& operator=(ok_t&&) noexcept(
        detail::traits<T>::is_nothrow_move_assignable) = default;

  private:
    template <typename U>
    static inline constexpr bool assign_value_req =
        !std::is_same_v<ok_t, std::remove_cvref_t<U>> &&
        detail::traits<T>::template is_constructible<U> &&
        detail::traits<T>::template is_assignable<U>;

    template <typename U>
    constexpr void assign_value(U&& value);

  public:
    template <typename U>
        requires assign_value_req<U>
    constexpr ok_t& operator=(U&& value) {
        assign_value(std::forward<U>(value));
        return *this;
    }

    constexpr typename detail::traits<T>::reference operator*() & noexcept;

    constexpr typename detail::traits<T>::const_reference operator*() const& noexcept;

    constexpr typename detail::traits<T>::rvalue_reference operator*() &&;

    constexpr typename detail::traits<T>::const_rvalue_reference operator*() const&&;

    // cppcheck-suppress functionConst
    constexpr typename detail::traits<T>::pointer operator->() noexcept;

    constexpr typename detail::traits<T>::const_pointer operator->() const noexcept;

    // cppcheck-suppress functionConst
    constexpr typename detail::traits<T>::reference value() & noexcept;

    constexpr typename detail::traits<T>::const_reference value() const& noexcept;

    constexpr typename detail::traits<T>::rvalue_reference value() &&;

    constexpr typename detail::traits<T>::const_rvalue_reference value() const&&;

    constexpr void swap(ok_t& other) noexcept(std::is_nothrow_swappable_v<variant<T>>);
};

template <typename T, typename U>
    requires(std::is_void_v<T> == std::is_void_v<U>)
constexpr bool operator==(const ok_t<T>& lhs, const ok_t<U>& rhs);

template <typename T, typename U>
    requires(!std::is_void_v<T>)
constexpr bool operator==(const ok_t<T>& lhs, const U& rhs);

template <typename T, typename U>
    requires(!std::is_void_v<U>)
constexpr bool operator==(const T& lhs, const ok_t<U>& rhs);

template <typename T>
constexpr void swap(ok_t<T>& a,
                    ok_t<T>& b) noexcept(std::is_nothrow_swappable_v<variant<T>>);

} // namespace sumty

#include "sumty/impl/result.hpp" // IWYU pragma: export

#endif
