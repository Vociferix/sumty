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

#include "sumty/variant.hpp"

#include <cstddef>
#include <exception>
#include <functional>
#include <initializer_list>
#include <system_error>
#include <type_traits>
#include <utility>

namespace sumty {

template <typename T>
class option;

template <typename T, typename E = std::error_code>
class result;

template <typename E>
class bad_result_access : public std::exception {
  private:
    E err_;

  public:
    bad_result_access(E error) : err_(std::move(error)) {}

    bad_result_access(const bad_result_access&) = default;

    bad_result_access(bad_result_access&&) noexcept(std::is_nothrow_move_constructible_v<E>) = default;

    ~bad_result_access() noexcept override = default;

    bad_result_access& operator=(const bad_result_access&) = default;

    bad_result_access& operator=(bad_result_access&&) noexcept(std::is_nothrow_move_assignable_v<E>) = default;

    E& error() & noexcept {
        return err_;
    }

    const E& error() const& noexcept {
        return err_;
    }

    E&& error() && {
        return std::move(err_);
    }

    const E&& error() const&& {
        return std::move(err_);
    }

    const char* what() const noexcept override {
        return "bad result access";
    }
};

template <>
class bad_result_access<void> : public std::exception {
  public:
    bad_result_access() noexcept = default;

    bad_result_access(const bad_result_access&) noexcept = default;

    bad_result_access(bad_result_access&&) noexcept = default;

    ~bad_result_access() noexcept override = default;

    bad_result_access& operator=(const bad_result_access&) = default;

    bad_result_access& operator=(bad_result_access&&) = default;

    void error() const noexcept { }

    const char* what() const noexcept override {
        return "bad result access";
    }
};

using in_place_error_t = std::in_place_index_t<1>;

static inline constexpr in_place_error_t in_place_error = std::in_place_index<1>;

template <typename T>
class ok_t;

template <typename E>
class error_t;

namespace detail {

template <typename T>
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<result<T, E>> : std::true_type {};

template <typename T>
static inline constexpr bool is_result_v = is_result<T>::value;

template <typename T>
struct is_error : std::false_type {};

template <typename E>
struct is_error<error_t<E>> : std::true_type {};

template <typename T>
static inline constexpr bool is_error_v = is_error<T>::value;

template <typename T>
struct is_ok : std::false_type {};

template <typename T>
struct is_ok<ok_t<T>> : std::true_type {};

template <typename T>
static inline constexpr bool is_ok_v = is_ok<T>::value;

}

template <typename E>
class error_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<E> err_;

  public:
    constexpr error_t() = default;

    constexpr error_t(const error_t&) = default;

    constexpr error_t(error_t&&) noexcept(detail::traits<E>::is_nothrow_move_constructible) = default;

    template <typename... Args>
    constexpr error_t([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : err_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr error_t([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
        : err_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename... Args>
    constexpr error_t([[maybe_unused]] in_place_error_t in_place, Args&&... args)
        : err_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr error_t([[maybe_unused]] in_place_error_t in_place, std::initializer_list<U> init, Args&&... args)
        : err_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename V>
        requires(std::is_constructible_v<variant<E>, std::in_place_index_t<0>, V&&> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_index_t<1>> &&
                 (!std::is_same_v<std::remove_cvref_t<E>, bool> || !detail::is_result_v<std::remove_cvref_t<V>>))
    constexpr error_t(V&& err)
        : err_(std::in_place_index<0>, std::forward<V>(err)) {}

    constexpr ~error_t() noexcept(detail::traits<E>::is_nothrow_destructible) = default;

    constexpr error_t& operator=(const error_t&) = default;

    constexpr error_t& operator=(error_t&&) noexcept(detail::traits<E>::is_nothrow_move_assignable) = default;

    template <typename V>
        requires(!std::is_same_v<error_t, std::remove_cvref_t<V>> &&
                 detail::traits<E>::template is_constructible<V> &&
                 detail::traits<E>::template is_assignable<V>)
    constexpr error_t& operator=(V&& err) {
        err_[index<0>] = std::forward<V>(err);
        return *this;
    }

    constexpr typename detail::traits<E>::reference operator*() & noexcept {
        return err_[index<0>];
    }

    constexpr typename detail::traits<E>::const_reference operator*() const& noexcept {
        return err_[index<0>];
    }

    constexpr typename detail::traits<E>::rvalue_reference operator*() && {
        return std::move(err_)[index<0>];
    }

    constexpr typename detail::traits<E>::const_rvalue_reference operator*() const&& {
        return std::move(err_)[index<0>];
    }

    constexpr typename detail::traits<E>::pointer operator->() noexcept {
        return &err_[index<0>];
    }

    constexpr typename detail::traits<E>::const_pointer operator->() const noexcept {
        return &err_[index<0>];
    }

    constexpr typename detail::traits<E>::reference error() & noexcept {
        return err_[index<0>];
    }

    constexpr typename detail::traits<E>::const_reference error() const& noexcept {
        return err_[index<0>];
    }

    constexpr typename detail::traits<E>::rvalue_reference error() && {
        return std::move(err_)[index<0>];
    }

    constexpr typename detail::traits<E>::const_rvalue_reference error() const&& {
        return std::move(err_)[index<0>];
    }

    constexpr void swap(error_t& other) noexcept(std::is_nothrow_swappable_v<variant<E>>) {
        err_.swap(other.err_);
    }

    template <typename V>
        requires(std::is_void_v<E> == std::is_void_v<V>)
    friend constexpr bool operator==(const error_t& lhs, const error_t<V>& rhs) {
        if constexpr (std::is_void_v<E>) {
            return true;
        } else {
            return *lhs == *rhs;
        }
    }

    template <typename V>
    friend constexpr bool operator==(const error_t& lhs, const V& rhs) {
        return *lhs == rhs;
    }
};

template <typename E>
constexpr void swap(error_t<E>& a, error_t<E>& b) noexcept(std::is_nothrow_swappable_v<variant<E>>) {
    a.swap(b);
}

template <typename T>
class ok_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<T> ok_;

  public:
    constexpr ok_t() = default;

    constexpr ok_t(const ok_t&) = default;

    constexpr ok_t(ok_t&&) noexcept(detail::traits<T>::is_nothrow_move_constructible) = default;

    template <typename... Args>
    constexpr ok_t([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : ok_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr ok_t([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
        : ok_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename U>
        requires(std::is_constructible_v<variant<T>, std::in_place_index_t<0>, U&&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> || !detail::is_result_v<std::remove_cvref_t<U>>))
    constexpr ok_t(U&& value)
        : ok_(std::in_place_index<0>, std::forward<U>(value)) {}

    constexpr ~ok_t() noexcept(detail::traits<T>::is_nothrow_destructible) = default;

    constexpr ok_t& operator=(const ok_t&) = default;

    constexpr ok_t& operator=(ok_t&&) noexcept(detail::traits<T>::is_nothrow_move_assignable) = default;

    template <typename U>
        requires(!std::is_same_v<ok_t, std::remove_cvref_t<U>> &&
                 detail::traits<T>::template is_constructible<U> &&
                 detail::traits<T>::template is_assignable<U>)
    constexpr ok_t& operator=(U&& value) {
        ok_[index<0>] = std::forward<U>(value);
        return *this;
    }

    constexpr typename detail::traits<T>::reference operator*() & noexcept {
        return ok_[index<0>];
    }

    constexpr typename detail::traits<T>::const_reference operator*() const& noexcept {
        return ok_[index<0>];
    }

    constexpr typename detail::traits<T>::rvalue_reference operator*() && {
        return std::move(ok_)[index<0>];
    }

    constexpr typename detail::traits<T>::const_rvalue_reference operator*() const&& {
        return std::move(ok_)[index<0>];
    }

    constexpr typename detail::traits<T>::pointer operator->() noexcept {
        return &ok_[index<0>];
    }

    constexpr typename detail::traits<T>::const_pointer operator->() const noexcept {
        return &ok_[index<0>];
    }

    constexpr typename detail::traits<T>::reference value() & noexcept {
        return ok_[index<0>];
    }

    constexpr typename detail::traits<T>::const_reference value() const& noexcept {
        return ok_[index<0>];
    }

    constexpr typename detail::traits<T>::rvalue_reference value() && {
        return std::move(ok_)[index<0>];
    }

    constexpr typename detail::traits<T>::const_rvalue_reference value() const&& {
        return std::move(ok_)[index<0>];
    }

    constexpr void swap(ok_t& other) noexcept(std::is_nothrow_swappable_v<variant<T>>) {
        ok_.swap(other.ok_);
    }

    template <typename U>
        requires(std::is_void_v<T> == std::is_void_v<U>)
    friend constexpr bool operator==(const ok_t& lhs, const ok_t<U>& rhs) {
        if constexpr (std::is_void_v<T>) {
            return true;
        } else {
            return *lhs == *rhs;
        }
    }

    template <typename U>
    friend constexpr bool operator==(const ok_t& lhs, const U& rhs) {
        return *lhs == rhs;
    }
};

template <typename T>
constexpr void swap(ok_t<T>& a, ok_t<T>& b) noexcept(std::is_nothrow_swappable_v<variant<T>>) {
    a.swap(b);
}

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

    constexpr result()
        noexcept(std::is_nothrow_default_constructible_v<variant<T, E>>)
        requires(std::is_default_constructible_v<variant<T, E>>)
        = default;

    constexpr result(const result&)
        noexcept(std::is_nothrow_copy_constructible_v<variant<T, E>>)
        requires(std::is_copy_constructible_v<variant<T, E>>)
        = default;

    constexpr result(result&&)
        noexcept(std::is_nothrow_move_constructible_v<variant<T, E>>)
        requires(std::is_move_constructible_v<variant<T, E>>)
        = default;

    template <typename... Args>
    constexpr result([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : res_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr result([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
        : res_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename... Args>
    constexpr result(std::in_place_index_t<0> in_place, Args&&... args)
        : res_(in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr result(std::in_place_index_t<0> in_place, std::initializer_list<U> init, Args&&... args)
        : res_(in_place, init, std::forward<Args>(args)...) {}

    template <typename... Args>
    constexpr result(in_place_error_t in_place, Args&&... args)
        : res_(in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr result(in_place_error_t in_place, std::initializer_list<U> init, Args&&... args)
        : res_(in_place, init, std::forward<Args>(args)...) {}

    template <typename U>
        requires(std::is_constructible_v<variant<T, E>, std::in_place_index_t<0>, U&&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<0>> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<1>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> || !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(!detail::traits<T>::template is_convertible_from<U&&>)
    constexpr result(U&& value) : res_(std::in_place_index<0>, std::forward<U>(value)) {}

    template <typename U>
    explicit(!detail::traits<T>::template is_convertible_from<U&&>)
    constexpr result(ok_t<U> ok) : res_(std::in_place_index<0>, *std::move(ok)) {}

    template <typename U>
    explicit(!detail::traits<E>::template is_convertible_from<U&&>)
    constexpr result(error_t<U> err) : res_(std::in_place_index<1>, *std::move(err)) {}

    template <typename U, typename V>
        requires(((std::is_void_v<U> && detail::traits<T>::is_default_constructible) || std::is_constructible_v<variant<T, E>, std::in_place_index_t<0>, typename detail::traits<U>::const_reference>) &&
                 ((std::is_void_v<V> && detail::traits<E>::is_default_constructible) || std::is_constructible_v<variant<T, E>, std::in_place_index_t<1>, typename detail::traits<E>::const_reference>))
    explicit((!std::is_void_v<U> && !detail::traits<T>::template is_convertible_from<U>) || (!std::is_void_v<V> && !detail::traits<E>::template is_convertible_from<V>))
    constexpr result(const result<U, V>& other)
        : res_(convert(other)) {}

    template <typename U, typename V>
        requires(((std::is_void_v<U> && detail::traits<T>::is_default_constructible) || std::is_constructible_v<variant<T, E>, std::in_place_index_t<0>, typename detail::traits<U>::rvalue_reference>) &&
                 ((std::is_void_v<V> && detail::traits<E>::is_default_constructible) || std::is_constructible_v<variant<T, E>, std::in_place_index_t<1>, typename detail::traits<E>::rvalue_reference>))
    explicit((!std::is_void_v<U> && !detail::traits<T>::template is_convertible_from<U>) || (!std::is_void_v<V> && !detail::traits<E>::template is_convertible_from<V>))
    constexpr result(result<U, V>&& other)
        : res_(convert(std::move(other))) {}

    constexpr ~result() noexcept(std::is_nothrow_destructible_v<variant<T, E>>) = default;

    constexpr result& operator=(const result&)
        noexcept(std::is_nothrow_copy_assignable_v<variant<T, E>>)
        requires(std::is_copy_assignable_v<variant<T, E>>)
        = default;

    constexpr result& operator=(result&&)
        noexcept(std::is_nothrow_move_assignable_v<variant<T, E>>)
        requires(std::is_move_assignable_v<variant<T, E>>)
        = default;

    template <typename U>
        requires(!std::is_same_v<result, std::remove_cvref_t<U>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 detail::traits<T>::template is_constructible<U> &&
                 detail::traits<T>::template is_assignable<U>)
    constexpr result& operator=(U&& value);

    template <typename U>
        requires((detail::traits<T>::template is_constructible<typename detail::traits<U>::const_reference> &&
                 detail::traits<T>::template is_assignable<typename detail::traits<U>::const_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
    constexpr result& operator=(const ok_t<U>& value);

    template <typename U>
        requires((detail::traits<T>::template is_constructible<typename detail::traits<U>::rvalue_reference> &&
                 detail::traits<T>::template is_assignable<typename detail::traits<U>::rvalue_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
    constexpr result& operator=(ok_t<U>&& value);

    template <typename V>
        requires((detail::traits<E>::template is_constructible<typename detail::traits<V>::const_reference> &&
                 detail::traits<E>::template is_assignable<typename detail::traits<V>::const_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
    constexpr result& operator=(const error_t<V>& error);

    template <typename V>
        requires((detail::traits<E>::template is_constructible<typename detail::traits<V>::rvalue_reference> &&
                 detail::traits<E>::template is_assignable<typename detail::traits<V>::rvalue_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
    constexpr result& operator=(error_t<V>&& value);

    constexpr operator bool() const noexcept;

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

    constexpr error_reference error() & noexcept;

    constexpr error_const_reference error() const& noexcept;

    constexpr error_rvalue_reference error() &&;

    constexpr error_const_rvalue_reference error() const&&;

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

    constexpr result<reference, error_reference> ref() noexcept;

    constexpr result<const_reference, error_const_reference> ref() const noexcept;

    constexpr result<const_reference, error_const_reference> cref() const noexcept;

    constexpr option<T> or_none() const& noexcept;

    constexpr option<T> or_none() &&;

    constexpr option<E> error_or_none() const& noexcept;

    constexpr option<E> error_or_none() &&;

    template <typename... Args>
    constexpr reference emplace(Args&&... args);

    template <typename U, typename... Args>
    constexpr reference emplace(std::initializer_list<U> ilist, Args&&... args);

    constexpr void swap(result& other) noexcept(std::is_nothrow_swappable_v<variant<T, E>>);
};

template <typename T, typename E, typename U, typename V>
    requires(std::is_void_v<T> == std::is_void_v<U> && std::is_void_v<E> == std::is_void_v<V>)
constexpr bool operator==(const result<T, E>& lhs, const result<U, V>& rhs);

template <typename T, typename E, typename U>
constexpr bool operator==(const result<T, E>& lhs, const U& rhs);

template <typename T, typename E, typename V>
constexpr bool operator==(const result<T, E>& lhs, const error_t<V>& rhs);

template <typename T, typename E>
constexpr void swap(result<T, E>& a, result<T, E>& b) noexcept(std::is_nothrow_swappable_v<variant<T, E>>);

template <typename E, typename... Args>
constexpr error_t<E> error(Args&&... args);

template <typename E, typename U, typename... Args>
constexpr error_t<E> error(std::initializer_list<U> ilist, Args&&... args);

template <typename T, typename... Args>
constexpr ok_t<T> ok(Args&&... args);

template <typename T, typename U, typename... Args>
constexpr ok_t<T> ok(std::initializer_list<U> ilist, Args&&... args);

}

#include "sumty/impl/result.hpp"

#endif
