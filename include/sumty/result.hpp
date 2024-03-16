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
#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/option.hpp" // IWYU pragma: keep
#include "sumty/utils.hpp"  // IWYU pragma: export
#include "sumty/variant.hpp"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace sumty {

template <typename E>
class error_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<E> err_;

  public:
#ifndef DOXYGEN
    using value_type = typename detail::traits<E>::value_type;
    using reference = typename detail::traits<E>::reference;
    using const_reference = typename detail::traits<E>::const_reference;
    using rvalue_reference = typename detail::traits<E>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<E>::const_rvalue_reference;
    using pointer = typename detail::traits<E>::pointer;
    using const_pointer = typename detail::traits<E>::const_pointer;
#else
    using value_type = ...;
    using reference = ...;
    using const_reference = ...;
    using rvalue_reference = ...;
    using const_rvalue_reference = ...;
    using pointer = ...;
    using const_pointer = ...;
#endif

    constexpr error_t()
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    constexpr error_t(const error_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    constexpr error_t(error_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_move_constructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : err_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr error_t([[maybe_unused]] std::in_place_t inplace,
                      std::initializer_list<U> init,
                      Args&&... args)
        : err_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t([[maybe_unused]] in_place_error_t inplace, Args&&... args)
        : err_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr error_t([[maybe_unused]] in_place_error_t inplace,
                      std::initializer_list<U> init,
                      Args&&... args)
        : err_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename V>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<E>, std::in_place_index_t<0>, V &&> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_index_t<1>> &&
                 (!std::is_same_v<std::remove_cvref_t<E>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<V>>))
    explicit(detail::traits<E>::template is_convertible_from<V&&>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t(V&& err)
        : err_(std::in_place_index<0>, std::forward<V>(err)) {
    }

    constexpr ~error_t()
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr error_t& operator=(const error_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    constexpr error_t& operator=(error_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_move_assignable) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename V>
#ifndef DOXYGEN
        requires(!std::is_same_v<error_t, std::remove_cvref_t<V>> &&
                 detail::traits<E>::template is_constructible<V> &&
                 detail::traits<E>::template is_assignable<V>)
#endif
    constexpr error_t& operator=(V&& error) {
        err_[index<0>] = std::forward<V>(error);
        return *this;
    }

    [[nodiscard]] constexpr reference operator*() & noexcept { return err_[index<0>]; }

    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return err_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(err_)[index<0>];
    }

    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(err_)[index<0>];
    }

    constexpr pointer operator->() noexcept { return &err_[index<0>]; }

    constexpr const_pointer operator->() const noexcept { return &err_[index<0>]; }

    [[nodiscard]] constexpr reference error() & noexcept { return err_[index<0>]; }

    [[nodiscard]] constexpr const_reference error() const& noexcept {
        return err_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference error() && {
        return std::move(err_)[index<0>];
    }

    [[nodiscard]] constexpr const_rvalue_reference error() const&& {
        return std::move(err_)[index<0>];
    }

    constexpr void swap(error_t& other)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_swappable_v<variant<E>>)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        err_.swap(other.err_);
    }
};

/// @relates error_t
template <typename E, typename V>
#ifndef DOXYGEN
    requires(std::is_void_v<E> == std::is_void_v<V>)
#endif
constexpr bool operator==(const error_t<E>& lhs, const error_t<V>& rhs) {
    if constexpr (std::is_void_v<E>) {
        return true;
    } else {
        return *lhs == *rhs;
    }
}

/// @relates error_t
template <typename E, typename V>
#ifndef DOXYGEN
    requires(!std::is_void_v<E>)
#endif
constexpr bool operator==(const error_t<E>& lhs, const V& rhs) {
    return *lhs == rhs;
}

/// @relates error_t
template <typename E, typename V>
#ifndef DOXYGEN
    requires(!std::is_void_v<V>)
#endif
constexpr bool operator==(const E& lhs, const error_t<V>& rhs) {
    return lhs == *rhs;
}

/// @relates error_t
template <typename E>
constexpr void swap(error_t<E>& a, error_t<E>& b)
#ifndef DOXYGEN
    noexcept(std::is_nothrow_swappable_v<variant<E>>)
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

template <typename T>
class ok_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<T> ok_;

  public:
#ifndef DOXYGEN
    using value_type = typename detail::traits<T>::value_type;
    using reference = typename detail::traits<T>::reference;
    using const_reference = typename detail::traits<T>::const_reference;
    using rvalue_reference = typename detail::traits<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<T>::const_rvalue_reference;
    using pointer = typename detail::traits<T>::pointer;
    using const_pointer = typename detail::traits<T>::const_pointer;
#else
    using value_type = ...;
    using reference = ...;
    using const_reference = ...;
    using rvalue_reference = ...;
    using const_rvalue_reference = ...;
    using pointer = ...;
    using const_pointer = ...;
#endif

    constexpr ok_t()
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    constexpr ok_t(const ok_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    constexpr ok_t(ok_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_constructible) = default;
#else
        ;
#endif

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr ok_t([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : ok_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr ok_t([[maybe_unused]] std::in_place_t inplace,
                   std::initializer_list<U> init,
                   Args&&... args)
        : ok_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<T>, std::in_place_index_t<0>, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(detail::traits<T>::template is_convertible_from<U&&>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr ok_t(U&& value)
        : ok_(std::in_place_index<0>, std::forward<U>(value)) {
    }

    constexpr ~ok_t()
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr ok_t& operator=(const ok_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    constexpr ok_t& operator=(ok_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_assignable) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename U>
#ifndef DOXYGEN
        requires(!std::is_same_v<ok_t, std::remove_cvref_t<U>> &&
                 detail::traits<T>::template is_constructible<U> &&
                 detail::traits<T>::template is_assignable<U>)
#endif
    constexpr ok_t& operator=(U&& value) {
        ok_[index<0>] = std::forward<U>(value);
        return *this;
    }

    [[nodiscard]] constexpr reference operator*() & noexcept { return ok_[index<0>]; }

    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return ok_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(ok_)[index<0>];
    }

    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(ok_)[index<0>];
    }

    constexpr pointer operator->() noexcept { return &ok_[index<0>]; }

    constexpr const_pointer operator->() const noexcept { return &ok_[index<0>]; }

    [[nodiscard]] constexpr reference value() & noexcept { return ok_[index<0>]; }

    [[nodiscard]] constexpr const_reference value() const& noexcept {
        return ok_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference value() && { return std::move(ok_)[index<0>]; }

    [[nodiscard]] constexpr const_rvalue_reference value() const&& {
        return std::move(ok_)[index<0>];
    }

    constexpr void swap(ok_t& other)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_swappable_v<variant<T>>)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        ok_.swap(other.ok_);
    }
};

/// @relates ok_t
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_void_v<T> == std::is_void_v<U>)
#endif
constexpr bool operator==(const ok_t<T>& lhs, const ok_t<U>& rhs) {
    if constexpr (std::is_void_v<T>) {
        return true;
    } else {
        return *lhs == *rhs;
    }
}

/// @relates ok_t
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<T>)
#endif
constexpr bool operator==(const ok_t<T>& lhs, const U& rhs) {
    return *lhs == rhs;
}

/// @relates ok_t
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<U>)
#endif
constexpr bool operator==(const T& lhs, const ok_t<U>& rhs) {
    return lhs == *rhs;
}

/// @relates ok_t
template <typename T>
constexpr void swap(ok_t<T>& a, ok_t<T>& b)
#ifndef DOXYGEN
    noexcept(std::is_nothrow_swappable_v<variant<T>>)
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

template <typename T, typename E>
class result {
  private:
    variant<T, E> res_;

    template <typename R, typename U, typename V>
    static constexpr variant<U, V> convert(R&& res) {
        if (res.has_value()) {
            if constexpr (std::is_void_v<decltype(*std::forward<R>(res))>) {
                return variant<U, V>{std::in_place_index<0>};
            } else {
                return variant<U, V>{std::in_place_index<0>, *std::forward<R>(res)};
            }
        } else {
            if constexpr (std::is_void_v<decltype(std::forward<R>(res).error())>) {
                return result<U, V>{std::in_place_index<1>};
            } else {
                return result<U, V>{std::in_place_index<1>, std::forward<R>(res).error()};
            }
        }
    }

  public:
#ifndef DOXYGEN
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
#else
    using value_type = ...;
    using reference = ...;
    using const_reference = ...;
    using rvalue_reference = ...;
    using const_rvalue_reference = ...;
    using pointer = ...;
    using const_pointer = ...;

    using error_type = ...;
    using error_reference = ...;
    using error_const_reference = ...;
    using error_rvalue_reference = ...;
    using error_const_rvalue_reference = ...;
    using error_pointer = ...;
    using error_const_pointer = ...;
#endif

    template <typename U>
    using rebind = result<U, E>;

    template <typename V>
    using rebind_error = result<T, V>;

    // For compatibility with std::expected
    using unexpected_type = error_t<E>;

    constexpr result()
#ifndef DOXYGEN
        noexcept(std::is_nothrow_default_constructible_v<variant<T, E>>)
        requires(std::is_default_constructible_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result(const result&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_copy_constructible_v<variant<T, E>>)
        requires(std::is_copy_constructible_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result(result&&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_move_constructible_v<variant<T, E>>)
        requires(std::is_move_constructible_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : res_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr result([[maybe_unused]] std::in_place_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : res_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(std::in_place_index_t<0> inplace, Args&&... args)
        : res_(inplace, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr result(std::in_place_index_t<0> inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : res_(inplace, init, std::forward<Args>(args)...) {}

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(in_place_error_t inplace, Args&&... args)
        : res_(inplace, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr result(in_place_error_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : res_(inplace, init, std::forward<Args>(args)...) {}

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<T, E>, std::in_place_index_t<0>, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<0>> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<1>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(U&& value)
        : res_(std::in_place_index<0>, std::forward<U>(value)) {
    }

    template <typename U>
#ifndef DOXYGEN
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITONALLY_NOEXCEPT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(ok_t<U> ok)
        : res_(std::in_place_index<0>, *std::move(ok)) {
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr result([[maybe_unused]] ok_t<void> ok) : res_(std::in_place_index<0>) {}

    template <typename U>
#ifndef DOXYGEN
    explicit(!detail::traits<E>::template is_convertible_from<U>)
#else
    CONDITONALLY_NOEXCEPT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(error_t<U> err)
        : res_(std::in_place_index<1>, *std::move(err)) {
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr result([[maybe_unused]] error_t<void> err) : res_(std::in_place_index<1>) {}

    template <typename U, typename V>
#ifndef DOXYGEN
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
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(const result<U, V>& other)
        : res_(convert(other)) {
    }

    template <typename U, typename V>
#ifndef DOXYGEN
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
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(result<U, V>&& other)
        : res_(convert(std::move(other))) {
    }

    constexpr ~result()
#ifndef DOXYGEN
        noexcept(std::is_nothrow_destructible_v<variant<T, E>>) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result& operator=(const result&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_copy_assignable_v<variant<T, E>>)
        requires(std::is_copy_assignable_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result& operator=(result&&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_move_assignable_v<variant<T, E>>)
        requires(std::is_move_assignable_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename U>
#ifndef DOXYGEN
        requires(!std::is_same_v<result, std::remove_cvref_t<U>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 detail::traits<T>::template is_constructible<U> &&
                 detail::traits<T>::template is_assignable<U>)
#endif
    constexpr result<T, E>& operator=(U&& value) {
        if (res_.index() == 0) {
            res_[index<0>] = std::forward<U>(value);
        } else {
            res_.template emplace<0>(std::forward<U>(value));
        }
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires((detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::const_reference> &&
                  detail::traits<T>::template is_assignable<
                      typename detail::traits<U>::const_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
#endif
    constexpr result& operator=(const ok_t<U>& value) {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<U>) {
                res_.template emplace<0>();
            } else {
                res_.template emplace<0>(*value);
            }
        } else {
            if constexpr (std::is_void_v<U>) {
                if constexpr (!std::is_void_v<T>) { res_[index<0>] = T{}; }
            } else {
                if constexpr (!std::is_void_v<T>) {
                    if constexpr (std::is_lvalue_reference_v<T>) {
                        res_.template emplace<0>(*value);
                    } else {
                        res_[index<0>] = *value;
                    }
                }
            }
        }
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires((detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::rvalue_reference> &&
                  detail::traits<T>::template is_assignable<
                      typename detail::traits<U>::rvalue_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
#endif
    constexpr result& operator=(ok_t<U>&& value) {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<U>) {
                res_.template emplace<0>();
            } else {
                res_.template emplace<0>(*std::move(value));
            }
        } else {
            if constexpr (std::is_void_v<U>) {
                if constexpr (!std::is_void_v<T>) { res_[index<0>] = T{}; }
            } else {
                if constexpr (!std::is_void_v<T>) {
                    if constexpr (std::is_lvalue_reference_v<T>) {
                        res_.template emplace<0>(*std::move(value));
                    } else {
                        res_[index<0>] = *std::move(value);
                    }
                }
            }
        }
        return *this;
    }

    template <typename V>
#ifndef DOXYGEN
        requires((detail::traits<E>::template is_constructible<
                      typename detail::traits<V>::const_reference> &&
                  detail::traits<E>::template is_assignable<
                      typename detail::traits<V>::const_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
#endif
    constexpr result& operator=(const error_t<V>& error) {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<V>) {
                res_.template emplace<1>();
            } else {
                res_.template emplace<1>(*error);
            }
        } else {
            if constexpr (std::is_void_v<V>) {
                if constexpr (!std::is_void_v<E>) { res_[index<1>] = E{}; }
            } else {
                if constexpr (!std::is_void_v<E>) {
                    if constexpr (std::is_lvalue_reference_v<E>) {
                        res_.template emplace<1>(*error);
                    } else {
                        res_[index<1>] = *error;
                    }
                }
            }
        }
        return *this;
    }

    template <typename V>
#ifndef DOXYGEN
        requires((detail::traits<E>::template is_constructible<
                      typename detail::traits<V>::rvalue_reference> &&
                  detail::traits<E>::template is_assignable<
                      typename detail::traits<V>::rvalue_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
#endif
    constexpr result& operator=(error_t<V>&& error) {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<V>) {
                res_.template emplace<1>();
            } else {
                res_.template emplace<1>(*std::move(error));
            }
        } else {
            if constexpr (std::is_void_v<V>) {
                if constexpr (!std::is_void_v<E>) { res_[index<1>] = E{}; }
            } else {
                if constexpr (!std::is_void_v<E>) {
                    if constexpr (std::is_lvalue_reference_v<E>) {
                        res_.template emplace<1>(*std::move(error));
                    } else {
                        res_[index<1>] = *std::move(error);
                    }
                }
            }
        }
        return *this;
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator bool() const noexcept { return res_.index() == 0; }

    [[nodiscard]] constexpr bool has_value() const noexcept { return res_.index() == 0; }

    [[nodiscard]] constexpr reference operator*() & noexcept { return res_[index<0>]; }

    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return res_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr pointer operator->() noexcept { return &res_[index<0>]; }

    [[nodiscard]] constexpr const_pointer operator->() const noexcept {
        return &res_[index<0>];
    }

    [[nodiscard]] constexpr reference value() & {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(res_[index<1>]);
            }
        }
        return res_[index<0>];
    }

    [[nodiscard]] constexpr const_reference value() const& {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(res_[index<1>]);
            }
        }
        return res_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference value() && {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(std::move(res_)[index<1>]);
            }
        }
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference value() const&& {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(std::move(res_)[index<1>]);
            }
        }
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr error_reference error() & noexcept { return res_[index<1>]; }

    [[nodiscard]] constexpr error_const_reference error() const& noexcept {
        return res_[index<1>];
    }

    [[nodiscard]] constexpr error_rvalue_reference error() && {
        return std::move(res_)[index<1>];
    }

    [[nodiscard]] constexpr error_const_rvalue_reference error() const&& {
        return std::move(res_)[index<1>];
    }

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) const& {
        if (res_.index() == 0) {
            return res_[index<0>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) && {
        if (res_.index() == 0) {
            return std::move(res_)[index<0>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    [[nodiscard]] constexpr value_type value_or() const& {
        if (res_.index() == 0) {
            return res_[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    [[nodiscard]] constexpr value_type value_or() && {
        if (res_.index() == 0) {
            return std::move(res_)[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr value_type value_or_else(F&& f) const& {
        if (res_.index() == 0) {
            return res_[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return std::invoke(std::forward<F>(f));
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr value_type value_or_else(F&& f) && {
        if (res_.index() == 0) {
            return std::move(res_)[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return std::invoke(std::forward<F>(f));
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) & {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error,
                                                                        res_[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), res_[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F, reference>>{
                        in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F, reference>>{
                        in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const& {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error,
                                                                        res_[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), res_[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{
                        in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{
                        in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) && {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{
                        in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const&& {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<
                        std::invoke_result_t<F, const_rvalue_reference>>{in_place_error};
                } else {
                    return std::remove_cvref_t<
                        std::invoke_result_t<F, const_rvalue_reference>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place, std::invoke(std::forward<F>(f), res_[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, const_reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place, std::invoke(std::forward<F>(f), res_[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, rvalue_reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, const_rvalue_reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto map(F&& f) & {
        return transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map(F&& f) const& {
        return transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map(F&& f) && {
        return std::move(*this).transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map(F&& f) const&& {
        return std::move(*this).transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto or_else(F&& f) const& {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::remove_cvref_t<std::invoke_result_t<F>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, res_[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f));
            }
        } else {
            using res_t =
                std::remove_cvref_t<std::invoke_result_t<F, error_const_reference>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, res_[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f), res_[index<1>]);
            }
        }
    }

    template <typename F>
    constexpr auto or_else(F&& f) && {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::remove_cvref_t<std::invoke_result_t<F>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, std::move(res_)[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f));
            }
        } else {
            using res_t =
                std::remove_cvref_t<std::invoke_result_t<F, error_rvalue_reference>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, std::move(res_)[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) & {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error, std::invoke(std::forward<F>(f), res_[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) const& {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_const_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error, std::invoke(std::forward<F>(f), res_[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) && {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_rvalue_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) const&& {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_const_rvalue_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto map_error(F&& f) & {
        return transform_error(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map_error(F&& f) const& {
        return transform_error(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map_error(F&& f) && {
        return std::move(*this).transform_error(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map_error(F&& f) const&& {
        return std::move(*this).transform_error(std::forward<F>(f));
    }

    [[nodiscard]] constexpr result<reference, error_reference> ref() noexcept {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return result<reference, error_reference>{std::in_place};
            } else {
                return result<reference, error_reference>{std::in_place, **this};
            }
        } else {
            if constexpr (std::is_void_v<E>) {
                return result<reference, error_reference>{in_place_error};
            } else {
                return result<reference, error_reference>{in_place_error, error()};
            }
        }
    }

    [[nodiscard]] constexpr result<const_reference, error_const_reference> ref()
        const noexcept {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return result<const_reference, error_const_reference>{std::in_place};
            } else {
                return result<const_reference, error_const_reference>{std::in_place,
                                                                      **this};
            }
        } else {
            if constexpr (std::is_void_v<E>) {
                return result<const_reference, error_const_reference>{in_place_error};
            } else {
                return result<const_reference, error_const_reference>{in_place_error,
                                                                      error()};
            }
        }
    }

    [[nodiscard]] constexpr result<const_reference, error_const_reference> cref()
        const noexcept {
        return ref();
    }

    [[nodiscard]] constexpr option<T> or_none() const& noexcept {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return option<T>{std::in_place};
            } else {
                return option<T>{std::in_place, res_[index<0>]};
            }
        } else {
            return option<T>{};
        }
    }

    [[nodiscard]] constexpr option<T> or_none() && {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return option<T>{std::in_place};
            } else {
                return option<T>{std::in_place, std::move(res_)[index<0>]};
            }
        } else {
            return option<T>{};
        }
    }

    [[nodiscard]] constexpr option<E> error_or_none() const& noexcept {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                return option<E>{std::in_place};
            } else {
                return option<E>{std::in_place, res_[index<1>]};
            }
        } else {
            return option<E>{};
        }
    }

    [[nodiscard]] constexpr option<E> error_or_none() && {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                return option<E>{std::in_place};
            } else {
                return option<E>{std::in_place, std::move(res_)[index<1>]};
            }
        } else {
            return option<E>{};
        }
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        return res_.template emplace<0>(std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr reference emplace(std::initializer_list<U> ilist, Args&&... args) {
        return res_.template emplace<0>(ilist, std::forward<Args>(args)...);
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) & {
        return res_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) const& {
        return res_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) && {
        return std::move(res_).visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) const&& {
        return std::move(res_).visit(std::forward<V>(visitor));
    }

    constexpr void swap(result& other)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_swappable_v<variant<T, E>>)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        res_.swap(other.res_);
    }
};

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::reference
#else
    REFERENCE
#endif
    get(result<T, E>& res) {
    return get<IDX>(res.res_);
}

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const result<T, E>& res) {
    return get<IDX>(res.res_);
}

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    get(result<T, E>&& res) {
    return get<IDX>(std::move(res.res_));
}

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const result<T, E>&& res) {
    return get<IDX>(std::move(res.res_));
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::reference
#else
    REFERENCE
#endif
    get(result<T, E>& res) {
    return get<U>(res.res_);
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const result<T, E>& res) {
    return get<U>(res.res_);
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    get(result<T, E>&& res) {
    return get<U>(std::move(res.res_));
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const result<T, E>&& res) {
    return get<U>(std::move(res.res_));
}

/// @relates result
template <typename T, typename E, typename U, typename V>
#ifndef DOXYGEN
    requires(std::is_void_v<T> == std::is_void_v<U> &&
             std::is_void_v<E> == std::is_void_v<V>)
#endif
constexpr bool operator==(const result<T, E>& lhs, const result<U, V>& rhs) {
    if (lhs.has_value()) {
        if constexpr (std::is_void_v<T>) {
            return rhs.has_value();
        } else {
            if (rhs.has_value()) {
                return *lhs == *rhs;
            } else {
                return false;
            }
        }
    } else {
        if constexpr (std::is_void_v<E>) {
            return !rhs.has_value();
        } else {
            if (rhs.has_value()) {
                return false;
            } else {
                return lhs.error() == rhs.error();
            }
        }
    }
}

/// @relates result
template <typename T, typename E, typename U>
constexpr bool operator==(const result<T, E>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

/// @relates result
template <typename T, typename E, typename V>
constexpr bool operator==(const result<T, E>& lhs, const error_t<V>& rhs) {
    return !lhs.has_value() && lhs.error() == rhs;
}

/// @relates result
template <typename T, typename E>
constexpr void swap(result<T, E>& a, result<T, E>& b)
#ifndef DOXYGEN
    noexcept(std::is_nothrow_swappable_v<variant<T, E>>)
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

/// @relates error_t
template <typename E, typename... Args>
constexpr error_t<E> error(Args&&... args) {
    return error_t<E>(std::in_place, std::forward<Args>(args)...);
}

/// @relates error_t
template <typename E, typename U, typename... Args>
constexpr error_t<E> error(std::initializer_list<U> ilist, Args&&... args) {
    return error_t<E>(std::in_place, ilist, std::forward<Args>(args)...);
}

/// @relates ok_t
template <typename T, typename... Args>
constexpr ok_t<T> ok(Args&&... args) {
    return ok_t<T>{std::in_place, std::forward<Args>(args)...};
}

/// @relates ok_t
template <typename T, typename U, typename... Args>
constexpr ok_t<T> ok(std::initializer_list<U> ilist, Args&&... args) {
    return ok_t<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

} // namespace sumty

#endif
