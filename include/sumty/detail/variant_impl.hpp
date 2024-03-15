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

#ifndef SUMTY_DETAIL_VARIANT_IMPL_HPP
#define SUMTY_DETAIL_VARIANT_IMPL_HPP

#include "sumty/detail/auto_union.hpp"
#include "sumty/detail/traits.hpp"
#include "sumty/detail/utils.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

// IWYU pragma: no_include <string>
// IWYU pragma: no_include <variant>

namespace sumty::detail {

template <typename Enable, typename... T>
class variant_impl {
  private:
    using discrim_t = discriminant_t<sizeof...(T)>;

    SUMTY_NO_UNIQ_ADDR auto_union<T...> data_;
    discrim_t discrim_{};

    template <size_t I>
    constexpr void copy_construct(const auto_union<T...>& data) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (std::is_void_v<select_t<I, T...>>) {
                    data_.template construct<I>();
                } else {
                    data_.template construct<I>(data.template get<I>());
                }
            } else {
                copy_construct<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void move_construct(auto_union<T...>& data) noexcept(
        (true && ... && traits<T>::is_nothrow_move_constructible)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (std::is_void_v<select_t<I, T...>>) {
                    data_.template construct<I>();
                } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
                    data_.template construct<I>(data.template get<I>());
                } else {
                    data_.template construct<I>(std::move(data.template get<I>()));
                }
            } else {
                move_construct<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void destroy() noexcept((true && ... && traits<T>::is_nothrow_destructible)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template destroy<I>();
            } else {
                destroy<I + 1>();
            }
        }
    }

    template <size_t I>
    constexpr void copy_assign(const auto_union<T...>& data) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (std::is_void_v<select_t<I, T...>>) {
                    data_.template construct<I>();
                } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
                    data_.template construct<I>(data.template get<I>());
                } else {
                    data_.template get<I>() = data.template get<I>();
                }
            } else {
                copy_assign<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void move_assign(auto_union<T...>& data) noexcept(
        (true && ... && traits<T>::is_nothrow_move_assignable)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (std::is_void_v<select_t<I, T...>>) {
                    data_.template construct<I>();
                } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
                    data_.template construct<I>(data.template get<I>());
                } else {
                    data_.template get<I>() = std::move(data.template get<I>());
                }
            } else {
                move_assign<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void same_swap(auto_union<T...>& data) noexcept(
        (true && ... && traits<T>::is_nothrow_swappable)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
                    auto* tmp = &data.template get<I>();
                    data.template construct<I>(data_.template get<I>());
                    data_.template construct<I>(*tmp);
                } else if constexpr (!std::is_void_v<select_t<I, T...>>) {
                    using std::swap;
                    swap(data_.template get<I>(), data.template get<I>());
                }
            } else {
                same_swap<I + 1>(data);
            }
        }
    }

    template <size_t I, size_t J>
    constexpr void diff_swap_impl(variant_impl& other) noexcept((
        true && ... &&
        (traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        std::swap(discrim_, other.discrim_);
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            if constexpr (std::is_void_v<select_t<J, T...>>) {
                return;
            } else if constexpr (std::is_lvalue_reference_v<select_t<J, T...>>) {
                data_.template construct<J>(other.data_.template get<J>());
                other.data_.template destroy<J>();
                other.data_.template construct<I>();
            } else {
                data_.template construct<J>(std::move(other.data_.template get<J>()));
                other.data_.template destroy<J>();
                other.data_.template construct<I>();
            }
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            if constexpr (std::is_void_v<select_t<J, T...>>) {
                other.data_.template construct<I>(data_.template get<I>());
                data_.template destroy<I>();
                data_.template construct<J>();
            } else if constexpr (std::is_lvalue_reference_v<select_t<J, T...>>) {
                auto& tmp = other.data_.template get<J>();
                other.data_.template construct<I>(data_.template get<I>());
                data_.template construct<J>(tmp);
            } else {
                auto& tmp = data_.template get<I>();
                data_.template construct<J>(std::move(other.data_.template get<J>()));
                other.data_.template destroy<J>();
                other.data_.template construct<I>(tmp);
            }
        } else {
            if constexpr (std::is_void_v<select_t<J, T...>>) {
                other.data_.template construct<I>(std::move(data_.template get<I>()));
                data_.template destroy<I>();
                data_.template construct<J>();
            } else if constexpr (std::is_lvalue_reference_v<select_t<J, T...>>) {
                auto& tmp = other.data_.template get<J>();
                other.data_.template construct<I>(std::move(data_.template get<I>()));
                data_.template destroy<I>();
                data_.template construct<J>(tmp);
            } else {
                auto&& tmp = std::move(other.data_.template get<J>());
                other.data_.template destroy<J>();
                other.data_.template construct<I>(std::move(data_.template get<I>()));
                data_.template destroy<I>();
                data_.template construct<J>(std::move(tmp));
            }
        }
    }

    template <size_t I, size_t J>
    constexpr void diff_swap_nested(variant_impl& other) noexcept((
        true && ... &&
        (traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        if constexpr (J < sizeof...(T)) {
            if (other.discrim_ == static_cast<discrim_t>(J)) {
                diff_swap_impl<I, J>(other);
            } else {
                diff_swap_nested<I, J + 1>(other);
            }
        }
    }

    template <size_t I>
    constexpr void diff_swap(variant_impl& other) noexcept((
        true && ... &&
        (traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                diff_swap_nested<I, 0>(other);
            } else {
                diff_swap<I + 1>(other);
            }
        }
    }

  public:
    constexpr variant_impl() noexcept(
        traits<first_t<T...>>::is_nothrow_default_constructible)
        : variant_impl(std::in_place_index<0>) {}

    constexpr variant_impl(const variant_impl& other) : discrim_(other.discrim_) {
        copy_construct<0>(other.data_);
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        (true && ... && traits<T>::is_nothrow_move_constructible))
        : discrim_(other.discrim_) {
        move_construct<0>(other.data_);
    }

    template <size_t I, typename... Args>
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr explicit(sizeof...(Args) == 0) variant_impl(
        [[maybe_unused]] std::in_place_index_t<I> inplace,
        Args&&... args) noexcept(traits<select_t<I, T...>>::
                                     template is_nothrow_constructible<Args...>)
        : discrim_(static_cast<discrim_t>(I)) {
        data_.template construct<I>(std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept((true && ... &&
                                        traits<T>::is_nothrow_destructible)) {
        destroy<0>();
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (discrim_ == rhs.discrim_) {
                copy_assign<0>(rhs.data_);
            } else {
                destroy<0>();
                discrim_ = rhs.discrim_;
                copy_construct<0>(rhs.data_);
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept((
        true && ... &&
        (traits<T>::is_nothrow_move_assignable &&
         traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        if (discrim_ == rhs.discrim_) {
            move_assign<0>(rhs.data_);
        } else {
            destroy<0>();
            discrim_ = rhs.discrim_;
            move_construct<0>(rhs.data_);
        }
        return *this;
    }

    [[nodiscard]] constexpr size_t index() const noexcept {
        return static_cast<size_t>(discrim_);
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T...>>::reference get() & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T...>>::const_reference get()
        const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T...>>::rvalue_reference get() && {
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            return data_.template get<I>();
        } else {
            auto ret = std::move(data_.template get<I>());
            return ret;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T...>>::const_rvalue_reference get()
        const&& {
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            return data_.template get<I>();
        } else {
            auto ret = std::move(data_.template get<I>());
            return ret;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T...>>::pointer ptr() noexcept {
        if constexpr (!std::is_void_v<select_t<I, T...>>) {
            return &data_.template get<I>();
        } else {
            return;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T...>>::const_pointer ptr()
        const noexcept {
        if constexpr (!std::is_void_v<select_t<I, T...>>) {
            return &data_.template get<I>();
        } else {
            return;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) {
        destroy<0>();
        data_.template construct<I>(std::forward<Args>(args)...);
        discrim_ = static_cast<discrim_t>(I);
    }

    constexpr void swap(variant_impl& other) noexcept(
        (true && ... &&
         (traits<T>::is_nothrow_swappable && traits<T>::is_nothrow_move_constructible &&
          traits<T>::is_nothrow_destructible))) {
        if (discrim_ == other.discrim_) {
            same_swap<0>(other.data_);
        } else {
            diff_swap<0>(other);
        }
    }
};

template <>
class variant_impl<void> {};

template <typename T>
class variant_impl<void, T> {
  private:
    SUMTY_NO_UNIQ_ADDR auto_union<T> data_;

  public:
    constexpr variant_impl() noexcept(traits<T>::is_nothrow_default_constructible) {
        data_.template construct<0>();
    }

    constexpr variant_impl(const variant_impl& other) {
        data_.template construct<0>(other.data_.template get<0>());
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        traits<T>::is_nothrow_move_constructible) {
        data_.template construct<0>(other.data_.template get<0>());
    }

    template <typename... Args>
    constexpr explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        variant_impl([[maybe_unused]] std::in_place_index_t<0> inplace, Args&&... args) {
        data_.template construct<0>(std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept(traits<T>::is_nothrow_destructible) {
        data_.template destroy<0>();
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if constexpr (std::is_lvalue_reference_v<T>) {
                data_.template construct<0>(rhs.data_.template get<0>());
            } else {
                data_.template get<0>() = rhs.data_.template get<0>();
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        traits<T>::is_nothrow_move_assignable) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            data_.template construct<0>(rhs.data_.template get<0>());
        } else {
            data_.template get<0>() = std::move(rhs.data_.template get<0>());
        }
        return *this;
    }

    [[nodiscard]] static constexpr size_t index() noexcept { return 0; }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<T>::reference get() & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<T>::const_reference get() const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<T>::rvalue_reference get() && {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<T>::const_rvalue_reference get() const&& {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<T>::pointer ptr() noexcept {
        if constexpr (!std::is_void_v<T>) {
            return &data_.template get<I>();
        } else {
            return;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<T>::const_pointer ptr() const noexcept {
        if constexpr (!std::is_void_v<T>) {
            return &data_.template get<I>();
        } else {
            return;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) {
        data_.template destroy<0>();
        data_.template construct<0>(std::forward<Args>(args)...);
    }

    constexpr void swap(variant_impl& other) noexcept(traits<T>::is_nothrow_swappable) {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            auto& tmp = other.data_.template get<0>();
            other.data_.template construct<0>(data_.template get<0>());
            data_.template construct<0>(tmp);
        } else {
            using std::swap;
            swap(data_.template get<0>(), other.data_.template get<0>());
        }
    }
};

template <>
class variant_impl<void, void> {
  public:
    constexpr variant_impl() noexcept = default;

    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<0> inplace) noexcept {}

    template <typename T>
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> inplace,
                           // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
                           [[maybe_unused]] T&& value) noexcept {}

    [[nodiscard]] static constexpr size_t index() noexcept { return 0; }

    template <size_t I>
    static constexpr void get() noexcept {}

    template <size_t I>
    static constexpr void ptr() noexcept {}

    static constexpr void swap([[maybe_unused]] variant_impl& other) noexcept {}
};

template <typename T>
class variant_impl<void, T&, void> {
  private:
    T* data_;

  public:
    variant_impl() = delete;

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> inplace,
                           U& value) noexcept
        : data_(&value) {}

    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<1> inplace) noexcept
        : data_(nullptr) {}

    template <typename U>
    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> inplace,
                                    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
                                    [[maybe_unused]] U&& value) noexcept
        : data_(nullptr) {}

    [[nodiscard]] constexpr size_t index() const noexcept {
        return static_cast<size_t>(data_ == nullptr);
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, void>>::const_reference get()
        const noexcept {
        if constexpr (I == 0) {
            return *data_;
        } else {
            return;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, void>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 0) {
            return data_;
        } else {
            return;
        }
    }

    template <size_t I>
    constexpr void emplace() noexcept {
        static_assert(I == 1, "no matching constructor for reference");
        data_ = nullptr;
    }

    template <size_t I, typename U>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        if constexpr (I == 0) {
            static_assert(std::is_lvalue_reference_v<U>,
                          "no matching constructor for reference");
            data_ = &value;
        } else {
            data_ = nullptr;
        }
    }

    constexpr void swap(variant_impl& other) noexcept { std::swap(data_, other.data_); }
};

template <typename T>
class variant_impl<void, void, T&> {
  private:
    T* data_{nullptr};

  public:
    constexpr variant_impl() noexcept = default;

    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<0> inplace) noexcept
        : data_(nullptr) {}

    template <typename U>
    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> inplace,
                                    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
                                    [[maybe_unused]] U&& value) noexcept
        : data_(nullptr) {}

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> inplace,
                           U& value) noexcept
        : data_(&value) {}

    [[nodiscard]] constexpr size_t index() const noexcept {
        return static_cast<size_t>(data_ != nullptr);
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, void, T&>>::const_reference get()
        const noexcept {
        if constexpr (I == 1) {
            return *data_;
        } else {
            return;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, void, T&>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 1) {
            return data_;
        } else {
            return;
        }
    }

    template <size_t I>
    constexpr void emplace() noexcept {
        static_assert(I == 0, "no matching constructor for reference");
        data_ = nullptr;
    }

    template <size_t I, typename U>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        if constexpr (I == 1) {
            static_assert(std::is_lvalue_reference_v<U>,
                          "no matching constructor for reference");
            data_ = &value;
        } else {
            data_ = nullptr;
        }
    }

    constexpr void swap(variant_impl& other) noexcept { std::swap(data_, other.data_); }
};

template <typename T, typename U>
class variant_impl<std::enable_if_t<(sizeof(U) <= sizeof(bool))>, T&, U> {
  private:
    T* head_;
    union Tail {
        SUMTY_NO_UNIQ_ADDR U tail;
    };
    SUMTY_NO_UNIQ_ADDR Tail tail_;

  public:
    variant_impl() = delete;

    constexpr variant_impl(const variant_impl& other) : head_(other.head_) {
        if (head_ == nullptr) { std::construct_at(&tail_.tail, other.tail_.tail); }
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        std::is_nothrow_move_constructible_v<U>)
        : head_(other.head_) {
        if (head_ == nullptr) {
            std::construct_at(&tail_.tail, std::move(other.tail_.tail));
        }
    }

    template <typename V>
        requires(std::is_convertible_v<V*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> inplace,
                           V& value) noexcept
        : head_(&value) {}

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> inplace,
                               Args&&... args)
        : head_(nullptr) {
        std::construct_at(&tail_.tail, std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept(std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) { std::destroy_at(&tail_.tail); }
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (head_ == nullptr) {
                head_ = rhs.head_;
                if (head_ == nullptr) {
                    tail_.tail = rhs.tail_.tail;
                } else {
                    std::destroy_at(&tail_.tail);
                }
            } else {
                head_ = rhs.head_;
                if (head_ == nullptr) { std::construct_at(&tail_.tail, rhs.tail_.tail); }
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        std::is_nothrow_move_assignable_v<U> && std::is_nothrow_move_constructible_v<U> &&
        std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            head_ = rhs.head_;
            if (head_ == nullptr) {
                tail_.tail = std::move(rhs.tail_.tail);
            } else {
                std::destroy_at(&tail_.tail);
            }
        } else {
            head_ = rhs.head_;
            if (head_ == nullptr) {
                std::construct_at(&tail_.tail, std::move(rhs.tail_.tail));
            }
        }
        return *this;
    }

    [[nodiscard]] constexpr size_t index() const noexcept {
        return static_cast<size_t>(head_ == nullptr);
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, U>>::reference
    get() & noexcept {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return tail_.tail;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, U>>::const_reference get()
        const& noexcept {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return tail_.tail;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, U>>::rvalue_reference get() && {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return std::move(tail_.tail);
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, U>>::const_rvalue_reference
    get() const&& {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return std::move(tail_.tail);
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, U>>::pointer ptr() noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return &tail_.tail;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T&, U>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return &tail_.tail;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        if constexpr (I == 0) {
            static_assert(
                (true && ... && std::is_lvalue_reference_v<Args>)&&sizeof...(Args) == 1,
                "no matching constructor for reference");
            if (head_ != nullptr) { std::destroy_at(&tail_.tail); }
            head_ = std::addressof(std::forward<Args>(args)...);
        } else {
            if (head_ == nullptr) {
                std::destroy_at(&tail_.tail);
            } else {
                head_ = nullptr;
            }
            std::construct_at(&tail_.tail, std::forward<Args>(args)...);
        }
    }

    constexpr void swap(variant_impl& other) noexcept(
        std::is_nothrow_swappable_v<U> && std::is_nothrow_move_constructible_v<U> &&
        std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            if (other.head_ == nullptr) {
                using std::swap;
                swap(tail_.tail, other.tail_.tail);
            } else {
                head_ = other.head_;
                other.head_ = nullptr;
                std::construct_at(&other.tail_.tail, std::move(tail_.tail));
                std::destroy_at(&tail_.tail);
            }
        } else {
            if (other.head_ == nullptr) {
                other.head_ = head_;
                head_ = nullptr;
                std::construct_at(&tail_.tail, std::move(other.tail_.tail));
                std::destroy_at(&other.tail_.tail);
            } else {
                std::swap(head_, other.head_);
            }
        }
    }
};

template <typename T, typename U>
class variant_impl<std::enable_if_t<(sizeof(T) <= sizeof(bool))>, T, U&> {
  private:
    U* tail_{nullptr};
    union Head {
        SUMTY_NO_UNIQ_ADDR T head;
    };
    SUMTY_NO_UNIQ_ADDR Head head_;

  public:
    constexpr variant_impl() noexcept(std::is_nothrow_default_constructible_v<T>) {
        std::construct_at(&head_.head);
    }

    constexpr variant_impl(const variant_impl& other) : tail_(other.tail_) {
        if (tail_ == nullptr) { std::construct_at(&head_.head, other.head_.head); }
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : tail_(other.tail_) {
        if (tail_ == nullptr) {
            std::construct_at(&head_.head, std::move(other.head_.head));
        }
    }

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> inplace,
                               Args&&... args) {
        std::construct_at(&head_.head, std::forward<Args>(args)...);
    }

    template <typename V>
        requires(std::is_convertible_v<V*, U*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> inplace,
                           V& value) noexcept
        : tail_(&value) {}

    constexpr ~variant_impl() noexcept(std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) { std::destroy_at(&head_.head); }
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (tail_ == nullptr) {
                tail_ = rhs.tail_;
                if (tail_ == nullptr) {
                    head_.head = rhs.head_.head;
                } else {
                    std::destroy_at(&head_.head);
                }
            } else {
                tail_ = rhs.tail_;
                if (tail_ == nullptr) { std::construct_at(&head_.head, rhs.head_.head); }
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            tail_ = rhs.tail_;
            if (tail_ == nullptr) {
                head_.head = std::move(rhs.head_.head);
            } else {
                std::destroy_at(&head_.head);
            }
        } else {
            tail_ = rhs.tail_;
            if (tail_ == nullptr) {
                std::construct_at(&head_.head, std::move(rhs.head_.head));
            }
        }
        return *this;
    }

    [[nodiscard]] constexpr size_t index() const noexcept {
        return static_cast<size_t>(tail_ != nullptr);
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T, U&>>::reference
    get() & noexcept {
        if constexpr (I == 0) {
            return head_.head;
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T, U&>>::const_reference get()
        const& noexcept {
        if constexpr (I == 0) {
            return head_.head;
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T, U&>>::rvalue_reference get() && {
        if constexpr (I == 0) {
            return std::move(head_.head);
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T, U&>>::const_rvalue_reference
    get() const&& {
        if constexpr (I == 0) {
            return std::move(head_.head);
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T, U&>>::pointer ptr() noexcept {
        if constexpr (I == 0) {
            return &head_.head;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    [[nodiscard]] constexpr typename traits<select_t<I, T, U&>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 0) {
            return &head_.head;
        } else {
            return tail_;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        if constexpr (I == 1) {
            static_assert(
                (true && ... && std::is_lvalue_reference_v<Args>)&&sizeof...(Args) == 1,
                "no matching constructor for reference");
            if (tail_ != nullptr) { std::destroy_at(&head_.head); }
            tail_ = std::addressof(std::forward<Args>(args)...);
        } else {
            if (tail_ == nullptr) {
                std::destroy_at(&head_.head);
            } else {
                tail_ = nullptr;
            }
            std::construct_at(&head_.head, std::forward<Args>(args)...);
        }
    }

    constexpr void swap(variant_impl& other) noexcept(
        std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            if (other.tail_ == nullptr) {
                using std::swap;
                swap(head_.head, other.head_.head);
            } else {
                tail_ = other.tail_;
                other.tail_ = nullptr;
                std::construct_at(&other.head_.head, std::move(head_.head));
                std::destroy_at(&head_.head);
            }
        } else {
            if (other.tail_ == nullptr) {
                other.tail_ = tail_;
                tail_ = nullptr;
                std::construct_at(&head_.head, std::move(other.head_.head));
                std::destroy_at(&other.head_.head);
            } else {
                std::swap(tail_, other.tail_);
            }
        }
    }
};

} // namespace sumty::detail

#endif
