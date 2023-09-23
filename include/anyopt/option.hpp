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

#ifndef ANYOPT_OPTION_HPP
#define ANYOPT_OPTION_HPP

#include <anyopt/uninit.hpp>

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

namespace anyopt {

class bad_option_access : std::exception {
  public:
    bad_option_access() noexcept = default;
    bad_option_access(const bad_option_access&) = default;
    bad_option_access(bad_option_access&&) noexcept = default;
    ~bad_option_access() override = default;
    bad_option_access& operator=(const bad_option_access&) = default;
    bad_option_access& operator=(bad_option_access&&) noexcept = default;

    const char* what() const noexcept override {
        return "bad option access";
    }
};

struct none_t { };

static inline constexpr none_t none{};

constexpr void swap([[maybe_unused]] none_t& a, none_t& b) noexcept { }

template <typename T>
class option;

namespace detail {

template <typename T>
struct option_impl {
  private:
#if defined(_MSC_VER) && !defined(__clang__)
    [[msvc::no_unique_address]]
#else
    [[no_unique_address]]
#endif
    uninit<std::remove_const_t<T>> data_{};
    bool has_value_{false};

  public:
    using value_type = std::remove_const_t<T>;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = std::add_const_t<T>&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;

    constexpr option_impl() noexcept = default;

    constexpr option_impl(const option_impl& other) : has_value_(other.has_value_) {
        data_.construct(*other.data_);
    }

    constexpr option_impl(option_impl&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>) : has_value_(other.has_value_) {
        data_.construct(std::move(*other.data_));
    }

    template <typename... Args>
    constexpr option_impl(std::in_place_t in_place, Args&&... args)
        : data_(in_place, std::forward<Args>(args)...), has_value_(true) {}

    constexpr ~option_impl() noexcept(std::is_nothrow_destructible_v<value_type>) {
        if (has_value_) {
            data_.destroy();
        }
    }

    constexpr option_impl& operator=(const option_impl& rhs) {
        if (has_value_) {
            if (rhs.has_value_) {
                *data_ = *rhs.data_;
            } else {
                data_.destroy();
                has_value_ = false;
            }
        } else if (rhs.has_value_) {
            data_.construct(*rhs.data_);
            has_value_ = true;
        }
        return *this;
    }

    constexpr option_impl& operator=(option_impl&& rhs) noexcept(std::is_nothrow_move_assignable_v<value_type>) {
        if (has_value_) {
            if (rhs.has_value_) {
                data_ = std::move(*rhs.data_);
                rhs.data_.destroy();
                rhs.has_value_ = false;
            } else {
                data_.destroy();
                has_value_ = false;
            }
        } else if (rhs.has_value_) {
            data_.construct(std::move(*rhs.data_));
            rhs.data_.destroy();
            rhs.has_value_ = false;
        }
        return *this;
    }

    constexpr bool has_value() const noexcept {
        return has_value_;
    }

    constexpr reference value() & noexcept {
        return *data_;
    }

    constexpr const_reference value() const& noexcept {
        return *data_;
    }

    constexpr rvalue_reference value() && {
        return std::move(*data_);
    }

    constexpr const_rvalue_reference value() const&& {
        return std::move(*data_);
    }

    constexpr pointer ptr() noexcept {
        return &*data_;
    }

    constexpr const_pointer ptr() const noexcept {
        return &*data_;
    }

    template <typename... Args>
    constexpr void emplace(Args&&... args) {
        if (has_value_) {
            data_.destroy();
        } else {
            has_value_ = true;
        }
        data_.construct(std::forward<Args>(args)...);
    }

    constexpr void reset() noexcept(std::is_nothrow_destructible_v<value_type>) {
        if (has_value_) {
            data_.destroy();
            has_value_ = false;
        }
    }

    constexpr void swap(option_impl& other) noexcept(std::is_nothrow_swappable_v<value_type> && std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_destructible_v<value_type>) {
        using std::swap;
        if (has_value_) {
            if (other.has_value_) {
                swap(*data_, *other.data_);
            } else {
                other.data_.construct(std::move(*data_));
                data_.destroy();
                has_value_ = false;
                other.has_value_ = true;
            }
        } else if (other.has_value_) {
            data_.construct(std::move(*other.data_));
            other.data_.destroy();
            has_value_ = true;
            other.has_value_ = false;
        }
    }
};

template <typename T>
struct option_impl<T&&> : public option_impl<T> {
  public:
    using value_type = typename option_impl<T>::value_type;
    using reference = typename option_impl<T>::reference;
    using const_reference = typename option_impl<T>::const_reference;
    using rvalue_reference = typename option_impl<T>::rvalue_reference;
    using const_rvalue_reference = typename option_impl<T>::const_rvalue_reference;
    using pointer = typename option_impl<T>::pointer;
    using const_pointer = typename option_impl<T>::const_pointer;

    constexpr option_impl() noexcept = default;

    template <typename... Args>
    constexpr option_impl(std::in_place_t in_place, Args&&... args)
        : option_impl<T>(in_place, std::forward<Args>(args)...) {}
};

template <typename T>
struct option_impl<T&> {
  private:
    T* data_{nullptr};

  public:
    using value_type = T&;
    using reference = T&;
    using const_reference = T&;
    using rvalue_reference = T&;
    using const_rvalue_reference = T&;
    using pointer = T*;
    using const_pointer = T*;

    constexpr option_impl() noexcept = default;

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr option_impl([[maybe_unused]] std::in_place_t in_place, U& value) noexcept
        : data_(&value) {}

    constexpr bool has_value() const noexcept {
        return data_ != nullptr;
    }

    constexpr reference value() const noexcept {
        return *data_;
    }

    constexpr pointer ptr() const noexcept {
        return data_;
    }

    constexpr void set_ptr(T* ptr) noexcept {
        data_ = ptr;
    }

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr void emplace(U& value) noexcept {
        data_ = &value;
    }

    constexpr void reset() noexcept {
        data_ = nullptr;
    }

    constexpr void swap(option_impl& other) noexcept {
        std::swap(data_, other.data_);
    }
};

template <>
struct option_impl<void> {
  private:
    bool has_value_{false};

  public:
    using value_type = void;
    using reference = void;
    using const_reference = void;
    using rvalue_reference = void;
    using const_rvalue_reference = void;
    using pointer = void;
    using const_pointer = void;

    constexpr option_impl() noexcept = default;

    explicit constexpr option_impl([[maybe_unused]] std::in_place_t in_place) noexcept
        : has_value_(true) {}

    template <typename U>
    constexpr option_impl([[maybe_unused]] std::in_place_t in_place, [[maybe_unused]] U&& value) noexcept
        : has_value_(true) {}

    constexpr bool has_value() const noexcept {
        return has_value_;
    }

    constexpr void value() const noexcept {}

    constexpr void ptr() const noexcept {}

    template <typename U>
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        has_value_ = true;
    }

    constexpr void reset() noexcept {
        has_value_ = false;
    }

    constexpr void swap(option_impl& other) noexcept {
        std::swap(has_value_, other.has_value_);
    }
};

template <typename T>
struct is_option : std::false_type {};

template <typename T>
struct is_option<option<T>> : std::true_type {};

template <typename T>
static inline constexpr bool is_option_v = is_option<T>::value;

}

template <typename T>
class option {
  private:
    detail::option_impl<T> opt_{};

  public:
    using value_type = typename detail::option_impl<T>::value_type;
    using reference = typename detail::option_impl<T>::reference;
    using const_reference = typename detail::option_impl<T>::const_reference;
    using rvalue_reference = typename detail::option_impl<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::option_impl<T>::const_rvalue_reference;
    using pointer = typename detail::option_impl<T>::pointer;
    using const_pointer = typename detail::option_impl<T>::const_pointer;

    constexpr option() noexcept = default;

    constexpr option(const option&) noexcept(std::is_nothrow_copy_constructible_v<detail::option_impl<T>>) = default;

    constexpr option(option&&) noexcept(std::is_nothrow_move_constructible_v<detail::option_impl<T>>) = default;

    constexpr option([[maybe_unused]] none_t none) noexcept : option() {}

    constexpr option([[maybe_unused]] std::nullopt_t null) noexcept : option() {}

    constexpr option([[maybe_unused]] std::nullptr_t null) noexcept
        requires(std::is_lvalue_reference_v<T>)
        : option() {}

    template <typename U>
        requires(std::is_lvalue_reference_v<value_type> && std::is_assignable_v<pointer&, U*>)
    explicit(!std::is_convertible_v<U*, pointer>)
    constexpr option(U* ptr) noexcept
        : opt_(ptr) {}

    template <typename U>
        requires(std::is_constructible_v<detail::option_impl<T>, std::in_place_t, typename option<U>::const_reference>)
    explicit(!std::is_convertible_v<typename option<U>::const_reference, value_type>)
    constexpr option(const option<U>& other) : option() {
        if (other.has_value()) {
            opt_.emplace(*other);
        }
    }

    template <typename U>
        requires(std::is_convertible_v<detail::option_impl<T>, std::in_place_t, typename option<U>::rvalue_reference>)
    explicit(!std::is_convertible_v<typename option<U>::rvalue_reference, value_type>)
    constexpr option(option<U>&& other) : option() {
        if (other.has_value()) {
            opt_.emplace(*std::move(other));
        }
    }

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
    constexpr option(std::in_place_t in_place, Args&&... args)
        : opt_(in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr option(std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
        : opt_(in_place, init, std::forward<Args>(args)...) {}

    template <typename U>
        requires(std::is_constructible_v<detail::option_impl<T>, std::in_place_t, U&&> && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> && (!std::is_same_v<std::remove_const_t<T>, bool> || !detail::is_option_v<U>))
    explicit(!std::is_convertible_v<U&&, value_type>)
    constexpr option(U&& value)
        : opt_(std::in_place, std::forward<U>(value)) {}

    constexpr ~option() noexcept(std::is_nothrow_destructible_v<detail::option_impl<T>>) = default;

    constexpr option& operator=(const option&) noexcept(std::is_nothrow_copy_assignable_v<detail::option_impl<T>>) = default;

    constexpr option& operator=(option&&) noexcept(std::is_nothrow_move_assignable_v<detail::option_impl<T>>) = default;

    constexpr option& operator=([[maybe_unused]] none_t none) noexcept(std::is_nothrow_destructible_v<detail::option_impl<T>>) {
        opt_.reset();
        return *this;
    }

    constexpr option& operator=([[maybe_unused]] std::nullopt_t null) noexcept(std::is_nothrow_destructible_v<detail::option_impl<T>>) {
        opt_.reset();
        return *this;
    }

    constexpr option& operator=([[maybe_unused]] std::nullptr_t null) noexcept
        requires(std::is_lvalue_reference_v<T>)
    {
        opt_.reset();
        return *this;
    }

    template <typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, option<T>> && std::is_constructible_v<detail::option_impl<T>, std::in_place_t, U&&> && std::is_assignable_v<value_type, U&&> && (!std::is_scalar_v<value_type> || !std::is_same_v<T, std::decay_t<U>>))
    constexpr option& operator=(U&& value) {
        opt_.emplace(std::forward<U>(value));
        return *this;
    }

    template <typename U>
        requires(std::is_lvalue_reference_v<value_type> && std::is_assignable_v<pointer&, U*> && std::is_convertible_v<U*, pointer>)
    constexpr option& operator=(U* ptr) noexcept {
        opt_.set_ptr(ptr);
        return *this;
    }

    template <typename U>
        requires(!std::is_constructible_v<value_type, option<U>&> &&
                 !std::is_constructible_v<value_type, const option<U>&> &&
                 !std::is_constructible_v<value_type, option<U>&&> &&
                 !std::is_constructible_v<value_type, const option<U>&&> &&
                 !std::is_convertible_v<option<U>&, value_type> &&
                 !std::is_convertible_v<const option<U>&, value_type> &&
                 !std::is_convertible_v<option<U>&&, value_type> &&
                 !std::is_convertible_v<const option<U>&&, value_type> &&
                 !std::is_assignable_v<reference, option<U>&> &&
                 !std::is_assignable_v<reference, const option<U>&> &&
                 !std::is_assignable_v<reference, option<U>&&> &&
                 !std::is_assignable_v<reference, const option<U>&&> &&
                 (std::is_lvalue_reference_v<value_type> || std::is_constructible_v<value_type, typename option<U>::const_reference>) &&
                 std::is_assignable_v<reference, typename option<U>::const_reference>)
    constexpr option& operator=(const option<U>& value) {
        if (value.has_value()) {
            opt_.emplace(*value);
        } else {
            opt_.reset();
        }
        return *this;
    }

    template <typename U>
        requires(!std::is_constructible_v<value_type, option<U>&> &&
                 !std::is_constructible_v<value_type, const option<U>&> &&
                 !std::is_constructible_v<value_type, option<U>&&> &&
                 !std::is_constructible_v<value_type, const option<U>&&> &&
                 !std::is_convertible_v<option<U>&, value_type> &&
                 !std::is_convertible_v<const option<U>&, value_type> &&
                 !std::is_convertible_v<option<U>&&, value_type> &&
                 !std::is_convertible_v<const option<U>&&, value_type> &&
                 !std::is_assignable_v<reference, option<U>&> &&
                 !std::is_assignable_v<reference, const option<U>&> &&
                 !std::is_assignable_v<reference, option<U>&&> &&
                 !std::is_assignable_v<reference, const option<U>&&> &&
                 (std::is_lvalue_reference_v<value_type> || std::is_constructible_v<value_type, typename option<U>::rvalue_reference>) &&
                 std::is_assignable_v<reference, typename option<U>::rvalue_reference>)
    constexpr option& operator=(option<U>&& value) {
        if (value.has_value()) {
            opt_.emplace(*std::move(value));
        } else {
            opt_.reset();
        }
        return *this;
    }

    constexpr operator bool() const noexcept {
        return opt_.has_value();
    }

    template <typename U>
        requires(std::is_lvalue_reference_v<value_type> && std::is_assignable_v<pointer&, U*>)
    explicit(!std::is_convertible_v<U*, pointer>)
    constexpr operator U*() const noexcept {
        return opt_.ptr();
    }

    constexpr bool has_value() const noexcept {
        return opt_.has_value();
    }

    constexpr reference operator*() & noexcept {
        return opt_.value();
    }

    constexpr const_reference operator*() const& noexcept {
        return opt_.value();
    }

    constexpr rvalue_reference operator*() && {
        return std::move(opt_).value();
    }

    constexpr const_rvalue_reference operator*() const&& {
        return std::move(opt_).value();
    }

    constexpr pointer operator->() noexcept {
        return opt_.ptr();
    }

    constexpr const_pointer operator->() const noexcept {
        return opt_.ptr();
    }

    constexpr reference value() & {
        if (!opt_.has_value()) {
            throw bad_option_access();
        }
        return opt_.value();
    }

    constexpr const_reference value() const& {
        if (!opt_.has_value()) {
            throw bad_option_access();
        }
        return opt_.value();
    }

    constexpr rvalue_reference value() && {
        if (!opt_.has_value()) {
            throw bad_option_access();
        }
        return std::move(opt_).value();
    }

    constexpr rvalue_reference value() const&& {
        if (!opt_.has_value()) {
            throw bad_option_access();
        }
        return std::move(opt_).value();
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) const& {
        if (opt_.has_value()) {
            return opt_.value();
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) && {
        if (opt_.has_value()) {
            return std::move(opt_).value();
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    constexpr value_type value_or() const& {
        if (opt_.has_value()) {
            return opt_.value();
        } else {
            if constexpr (std::is_void_v<value_type>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    constexpr value_type value_or() && {
        if (opt_.has_value()) {
            return std::move(opt_).value();
        } else {
            if constexpr (std::is_void_v<value_type>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) & {
        if constexpr (std::is_void_v<reference>) {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f), opt_.value());
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const& {
        if constexpr (std::is_void_v<const_reference>) {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f), opt_.value());
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) && {
        if constexpr (std::is_void_v<rvalue_reference>) {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f), std::move(opt_).value());
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const&& {
        if constexpr (std::is_void_v<const_rvalue_reference>) {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.has_value()) {
                return std::invoke(std::forward<F>(f), std::move(opt_).value());
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, const_rvalue_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
        if constexpr (std::is_void_v<reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, reference>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), opt_.value());
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f), opt_.value())};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
        if constexpr (std::is_void_v<const_reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, const_reference>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), opt_.value());
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f), opt_.value())};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
        if constexpr (std::is_void_v<rvalue_reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, rvalue_reference>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(opt_).value());
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f), std::move(opt_).value())};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
        if constexpr (std::is_void_v<const_rvalue_reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, const_rvalue_reference>;
            if (opt_.has_value()) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(opt_).value());
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::invoke(std::forward<F>(f), std::move(opt_).value())};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr option or_else(F&& f) const& {
        if (opt_.has_value()) {
            return *this;
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    template <typename F>
    constexpr option or_else(F&& f) && {
        if (opt_.has_value()) {
            return std::move(*this);
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    constexpr void swap(option& other) noexcept(noexcept(opt_.swap(other.opt_))) {
        opt_.swap(other.opt_);
    }

    constexpr void reset() noexcept {
        opt_.reset();
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        opt_.emplace(std::forward<Args>(args)...);
        return opt_.value();
    }
};

template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value()) {
        return rhs.has_value() && *lhs == *rhs;
    } else {
        return !rhs.has_value();
    }
}

template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value()) {
        return !rhs.has_value() || *lhs != *rhs;
    } else {
        return rhs.has_value();
    }
}

template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const option<U>& rhs) {
    return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const option<U>& rhs) {
    return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
}

template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const option<U>& rhs) {
    return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
}

template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const option<U>& rhs) {
    return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
}

template <typename T, typename U>
    requires(std::three_way_comparable_with<std::remove_cvref_t<U>, std::remove_cvref_t<T>>)
constexpr std::compare_three_way_result_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>
operator<=>(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value() && rhs.has_value()) {
        return *lhs <=> *rhs;
    } else {
        return lhs.has_value() <=> rhs.has_value();
    }
}

template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

template <typename T, typename U>
constexpr bool operator==(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs == *rhs;
}

template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs != rhs;
}

template <typename T, typename U>
constexpr bool operator!=(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs != *rhs;
}

template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs < rhs;
}

template <typename T, typename U>
constexpr bool operator<(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs < *rhs;
}

template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() && *lhs > rhs;
}

template <typename T, typename U>
constexpr bool operator>(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs > *lhs;
}

template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs <= rhs;
}

template <typename T, typename U>
constexpr bool operator<=(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs <= *rhs;
}

template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs >= rhs;
}

template <typename T, typename U>
constexpr bool operator>=(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs >= *rhs;
}

template <typename T, typename U>
    requires(std::three_way_comparable_with<std::remove_cvref_t<U>, std::remove_cvref_t<T>>)
constexpr std::compare_three_way_result_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>
operator<=>(const option<T>& lhs, const U& rhs) {
    if (lhs.has_value()) {
        return *lhs <=> rhs;
    } else {
        return std::strong_ordering::less;
    }
}

template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return !lhs.has_value();
}

template <typename T>
constexpr bool operator==([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value();
}

template <typename T>
constexpr bool operator!=([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return false;
}

template <typename T>
constexpr bool operator<([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value();
}

template <typename T>
constexpr bool operator>([[maybe_unused]] none_t lhs, [[maybe_unused]] const option<T>& rhs) {
    return false;
}

template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return !lhs.has_value();
}

template <typename T>
constexpr bool operator<=([[maybe_unused]] none_t lhs, [[maybe_unused]] const option<T>& rhs) {
    return true;
}

template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return true;
}

template <typename T>
constexpr bool operator>=([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value() <=> false;
}

template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return !lhs.has_value();
}

template <typename T>
constexpr bool operator==([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value();
}

template <typename T>
constexpr bool operator!=([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return false;
}

template <typename T>
constexpr bool operator<([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value();
}

template <typename T>
constexpr bool operator>([[maybe_unused]] std::nullopt_t lhs, [[maybe_unused]] const option<T>& rhs) {
    return false;
}

template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return !lhs.has_value();
}

template <typename T>
constexpr bool operator<=([[maybe_unused]] std::nullopt_t lhs, [[maybe_unused]] const option<T>& rhs) {
    return true;
}

template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return true;
}

template <typename T>
constexpr bool operator>=([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value() <=> false;
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return !lhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator==([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator!=([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<([[maybe_unused]] const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return false;
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>([[maybe_unused]] std::nullptr_t lhs, [[maybe_unused]] const option<T>& rhs) {
    return false;
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return !lhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator<=([[maybe_unused]] std::nullptr_t lhs, [[maybe_unused]] const option<T>& rhs) {
    return true;
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return true;
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr bool operator>=([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
constexpr std::strong_ordering operator<=>(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value() <=> false;
}

template <typename T, typename... Args>
constexpr option<T> some(Args&&... args) {
    return option<T>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename U, typename... Args>
constexpr option<T> some(std::initializer_list<U> ilist, Args&&... args) {
    return option<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

}

#endif
