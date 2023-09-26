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

#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <variant> // for std::variant_size and std::variant_alternative

#if defined(_MSC_VER) && !defined(__clang__)
#define SUMTY_NO_UNIQ_ADDR [[msvc::no_unique_address]]
#else
#define SUMTY_NO_UNIQ_ADDR [[no_unique_address]]
#endif

namespace sumty {

class bad_variant_access : public std::exception {
  public:
    bad_variant_access() noexcept = default;
    bad_variant_access(const bad_variant_access&) = default;
    bad_variant_access(bad_variant_access&&) noexcept = default;
    ~bad_variant_access() override = default;
    bad_variant_access& operator=(const bad_variant_access&) = default;
    bad_variant_access& operator=(bad_variant_access&&) noexcept = default;

    const char* what() const noexcept override {
        return "bad variant access";
    }
};

template <size_t I>
struct index_t {};

template <size_t I>
static inline constexpr index_t<I> index{};

template <typename T>
struct type_t {};

template <typename T>
static inline constexpr type_t<T> type{};

template <typename... T>
class variant;

namespace detail {

template <typename T>
struct traits {
    using value_type = std::remove_const_t<T>;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = const T&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;

    static inline constexpr bool is_default_constructible = std::is_default_constructible_v<value_type>;
    static inline constexpr bool is_nothrow_default_constructible = std::is_nothrow_default_constructible_v<value_type>;
    static inline constexpr bool is_copy_constructible = std::is_copy_constructible_v<value_type>;
    static inline constexpr bool is_nothrow_copy_constructible = std::is_nothrow_copy_constructible_v<value_type>;
    static inline constexpr bool is_move_constructible = std::is_move_constructible_v<value_type>;
    static inline constexpr bool is_nothrow_move_constructible = std::is_nothrow_move_constructible_v<value_type>;
    static inline constexpr bool is_destructible = std::is_destructible_v<value_type>;
    static inline constexpr bool is_nothrow_destructible = std::is_nothrow_destructible_v<value_type>;
    static inline constexpr bool is_copy_assignable = std::is_copy_assignable_v<T>;
    static inline constexpr bool is_nothrow_copy_assignable = std::is_nothrow_copy_assignable_v<T>;
    static inline constexpr bool is_move_assignable = std::is_move_assignable_v<T>;
    static inline constexpr bool is_nothrow_move_assignable = std::is_nothrow_move_assignable_v<T>;
    static inline constexpr bool is_swappable = std::is_swappable_v<T>;
    static inline constexpr bool is_nothrow_swappable = std::is_nothrow_swappable_v<T>;

    template <typename U>
    static inline constexpr bool is_convertible_from = std::is_convertible_v<U, value_type>;

    template <typename... U>
    static inline constexpr bool is_constructible = std::is_constructible_v<value_type, U...>;

    template <typename U>
    static inline constexpr bool is_assignable = std::is_assignable_v<reference, U>;

    template <typename... U>
    static inline constexpr bool is_nothrow_constructible = std::is_nothrow_constructible_v<value_type, U...>;
};

template <typename T>
struct traits<T&&> : traits<T> {};

template <typename T>
struct traits<T&> {
    using value_type = T&;
    using reference = T&;
    using const_reference = T&;
    using rvalue_reference = T&;
    using const_rvalue_reference = T&;
    using pointer = T*;
    using const_pointer = T*;

    static inline constexpr bool is_default_constructible = false;
    static inline constexpr bool is_nothrow_default_constructible = false;
    static inline constexpr bool is_copy_constructible = true;
    static inline constexpr bool is_nothrow_copy_constructible = true;
    static inline constexpr bool is_move_constructible = true;
    static inline constexpr bool is_nothrow_move_constructible = true;
    static inline constexpr bool is_destructible = true;
    static inline constexpr bool is_nothrow_destructible = true;
    static inline constexpr bool is_copy_assignable = true;
    static inline constexpr bool is_nothrow_copy_assignable = true;
    static inline constexpr bool is_move_assignable = true;
    static inline constexpr bool is_nothrow_move_assignable = true;
    static inline constexpr bool is_swappable = true;
    static inline constexpr bool is_nothrow_swappable = true;

    template <typename U>
    static inline constexpr bool is_convertible_from = std::is_lvalue_reference_v<U> && std::is_convertible_v<typename traits<U>::pointer, pointer>;

    template <typename... U>
    struct is_constructible_t : std::false_type {};

    template <typename U>
    struct is_constructible_t<U> : std::integral_constant<bool, std::is_convertible_v<U, pointer>> {};

    template <typename... U>
    static inline constexpr bool is_constructible = is_constructible_t<U...>::value;

    template <typename U>
    static inline constexpr bool is_assignable = std::is_lvalue_reference_v<U> && std::is_convertible_v<typename traits<U>::pointer, pointer>;

    template <typename... U>
    static inline constexpr bool is_nothrow_constructible = is_constructible_t<U...>::value;
};

template <>
struct traits<void> {
    using value_type = void;
    using reference = void;
    using const_reference = void;
    using rvalue_reference = void;
    using const_rvalue_reference = void;
    using pointer = void;
    using const_pointer = void;

    static inline constexpr bool is_default_constructible = true;
    static inline constexpr bool is_nothrow_default_constructible = true;
    static inline constexpr bool is_copy_constructible = true;
    static inline constexpr bool is_nothrow_copy_constructible = true;
    static inline constexpr bool is_move_constructible = true;
    static inline constexpr bool is_nothrow_move_constructible = true;
    static inline constexpr bool is_destructible = true;
    static inline constexpr bool is_nothrow_destructible = true;
    static inline constexpr bool is_copy_assignable = true;
    static inline constexpr bool is_nothrow_copy_assignable = true;
    static inline constexpr bool is_move_assignable = true;
    static inline constexpr bool is_nothrow_move_assignable = true;
    static inline constexpr bool is_swappable = true;
    static inline constexpr bool is_nothrow_swappable = true;

    template <typename U>
    static inline constexpr bool is_convertible_from = true;

    template <typename... U>
    static inline constexpr bool is_constructible = sizeof...(U) <= 1;

    template <typename U>
    static inline constexpr bool is_assignable = true;

    template <typename... U>
    static inline constexpr bool is_nothrow_constructible = sizeof...(U) <= 1;
};

template <typename... T>
union auto_union;

template <>
union auto_union<> {
    constexpr auto_union() noexcept {}

    constexpr auto_union([[maybe_unused]] const auto_union& other) noexcept {}

    constexpr auto_union([[maybe_unused]] auto_union&& other) noexcept {}

    constexpr ~auto_union() noexcept {}

    constexpr auto_union& operator=([[maybe_unused]] const auto_union& rhs) noexcept {
        return *this;
    }

    constexpr auto_union& operator=([[maybe_unused]] auto_union&& rhs) noexcept {
        return *this;
    }
};

template <typename T0, typename... TN>
union auto_union<T0, TN...> {
    SUMTY_NO_UNIQ_ADDR std::remove_const_t<T0> head_;
    SUMTY_NO_UNIQ_ADDR auto_union<TN...> tail_;

    constexpr auto_union() noexcept {}

    constexpr auto_union([[maybe_unused]] const auto_union& other) noexcept {}

    constexpr auto_union([[maybe_unused]] auto_union&& other) noexcept {}

    constexpr ~auto_union() noexcept {}

    constexpr auto_union& operator=([[maybe_unused]] const auto_union& rhs) noexcept {
        return *this;
    }

    constexpr auto_union& operator=([[maybe_unused]] auto_union&& rhs) noexcept {
        return *this;
    }

    template <size_t IDX>
    constexpr decltype(auto) get() noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX-1>();
        }
    }

    template <size_t IDX>
    constexpr decltype(auto) get() const noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX-1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct(Args&&... args) {
        if constexpr (IDX == 0) {
            std::construct_at(&head_, std::forward<Args>(args)...);
        } else {
            tail_.template construct<IDX-1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX == 0) {
            std::destroy_at(&head_);
        } else {
            tail_.template destroy<IDX-1>();
        }
    }
};

template <typename T0, typename... TN>
union auto_union<T0&&, TN...> {
    SUMTY_NO_UNIQ_ADDR std::remove_const_t<T0> head_;
    SUMTY_NO_UNIQ_ADDR auto_union<TN...> tail_;

    constexpr auto_union() noexcept {}

    constexpr auto_union([[maybe_unused]] const auto_union& other) noexcept {}

    constexpr auto_union([[maybe_unused]] auto_union&& other) noexcept {}

    constexpr ~auto_union() noexcept {}

    constexpr auto_union& operator=([[maybe_unused]] const auto_union& rhs) noexcept {
        return *this;
    }

    constexpr auto_union& operator=([[maybe_unused]] auto_union&& rhs) noexcept {
        return *this;
    }

    template <size_t IDX>
    constexpr decltype(auto) get() noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX-1>();
        }
    }

    template <size_t IDX>
    constexpr decltype(auto) get() const noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX-1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct(Args&&... args) {
        if constexpr (IDX == 0) {
            std::construct_at(&head_, std::forward<Args>(args)...);
        } else {
            tail_.template construct<IDX-1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX == 0) {
            std::destroy_at(&head_);
        } else {
            tail_.template destroy<IDX-1>();
        }
    }
};

template <typename T0, typename... TN>
union auto_union<T0&, TN...> {
    T0* head_;
    SUMTY_NO_UNIQ_ADDR auto_union<TN...> tail_;

    constexpr auto_union() noexcept {}

    constexpr auto_union([[maybe_unused]] const auto_union& other) noexcept {}

    constexpr auto_union([[maybe_unused]] auto_union&& other) noexcept {}

    constexpr ~auto_union() noexcept {}

    constexpr auto_union& operator=([[maybe_unused]] const auto_union& rhs) noexcept {
        return *this;
    }

    constexpr auto_union& operator=([[maybe_unused]] auto_union&& rhs) noexcept {
        return *this;
    }

    template <size_t IDX>
    constexpr decltype(auto) get() const noexcept {
        if constexpr (IDX == 0) {
            return *head_;
        } else {
            return tail_.template get<IDX-1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct(Args&&... args) {
        if constexpr (IDX == 0) {
            head_ = std::addressof(std::forward<Args>(args)...);
        } else {
            tail_.template construct<IDX-1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX != 0) {
            tail_.template destroy<IDX-1>();
        }
    }
};

template <typename... TN>
union auto_union<void, TN...> {
    SUMTY_NO_UNIQ_ADDR auto_union<TN...> tail_;

    constexpr auto_union() noexcept {}

    constexpr auto_union([[maybe_unused]] const auto_union& other) noexcept {}

    constexpr auto_union([[maybe_unused]] auto_union&& other) noexcept {}

    constexpr ~auto_union() noexcept {}

    constexpr auto_union& operator=([[maybe_unused]] const auto_union& rhs) noexcept {
        return *this;
    }

    constexpr auto_union& operator=([[maybe_unused]] auto_union&& rhs) noexcept {
        return *this;
    }

    template <size_t IDX>
    constexpr decltype(auto) get() const noexcept {
        if constexpr (IDX != 0) {
            return tail_.template get<IDX-1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct([[maybe_unused]] Args&&... args) {
        if constexpr (IDX != 0) {
            tail_.template construct<IDX-1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX != 0) {
            tail_.template destroy<IDX-1>();
        }
    }
};

template <uint64_t N, typename = void>
struct smallest_discrim {
    using type = uint64_t;
};

template <uint64_t N>
struct smallest_discrim<N, std::enable_if_t<(N <= 1)>> {
    using type = bool;
};

template <uint64_t N>
struct smallest_discrim<N, std::enable_if_t<(N <= static_cast<uint64_t>(~uint8_t{0}))>> {
    using type = uint8_t;
};

template <uint64_t N>
struct smallest_discrim<N, std::enable_if_t<(N <= static_cast<uint64_t>(~uint16_t{0}) && N > static_cast<uint64_t>(~uint8_t{0}))>> {
    using type = uint16_t;
};

template <uint64_t N>
struct smallest_discrim<N, std::enable_if_t<(N <= static_cast<uint64_t>(~uint32_t{0}) && N > static_cast<uint64_t>(~uint16_t{0}))>> {
    using type = uint32_t;
};

template <uint64_t N>
using smallest_discrim_t = typename smallest_discrim<N>::type;

template <size_t IDX, typename... T>
struct select;

template <typename T0, typename... TN>
struct select<0, T0, TN...> {
    using type = T0;
};

template <size_t IDX, typename T0, typename... TN>
struct select<IDX, T0, TN...> {
    using type = typename select<IDX-1, TN...>::type;
};

template <size_t IDX>
struct select<IDX> {
    using type = void;
};

template <size_t IDX, typename... T>
using select_t = typename select<IDX, T...>::type;

template <typename... T>
using first_t = select_t<0, T...>;

template <size_t N, typename T, typename... U>
struct type_count_impl;

template <size_t N, typename T>
struct type_count_impl<N, T> : std::integral_constant<size_t, N> {};

template <size_t N, typename T, typename U0, typename... UN>
struct type_count_impl<N, T, U0, UN...> : type_count_impl<N, T, UN...> {};

template <size_t N, typename T, typename... UN>
struct type_count_impl<N, T, T, UN...> : type_count_impl<N+1, T, UN...> {};

template <typename T, typename... U>
struct type_count : type_count_impl<0, T, U...> {};

template <typename T, typename... U>
static inline constexpr size_t type_count_v = type_count<T, U...>::value;

template <typename T, typename... U>
struct is_unique : std::integral_constant<bool, type_count_v<T, U...> == 1> {};

template <typename T, typename... U>
static inline constexpr bool is_unique_v = is_unique<T, U...>::value;

template <size_t IDX, typename T, typename... U>
struct index_of_impl;

template <size_t IDX, typename T, typename U0, typename... UN>
struct index_of_impl<IDX, T, U0, UN...> : index_of_impl<IDX+1, T, UN...> {};

template <size_t IDX, typename T, typename... UN>
struct index_of_impl<IDX, T, T, UN...> : std::integral_constant<size_t, IDX> {};

template <typename T, typename... U>
struct index_of : index_of_impl<0, T, U...> {};

template <typename T, typename... U>
static inline constexpr size_t index_of_v = index_of<T, U...>::value;

template <typename Enable, typename... T>
class variant_impl {
  private:
    using discrim_t = smallest_discrim_t<static_cast<uint64_t>(sizeof...(T))>;

    SUMTY_NO_UNIQ_ADDR auto_union<T...> data_;
    discrim_t discrim_{};

    template <size_t I>
    constexpr void copy_construct(const auto_union<T...>& data) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template construct<I>(data.template get<I>());
            } else {
                copy_construct<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void move_construct(auto_union<T...>& data) noexcept((true && ... && traits<T>::is_nothrow_move_constructible)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template construct<I>(std::move(data.template get<I>()));
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
                data_.template get<I>() = data.template get<I>();
            } else {
                copy_assign<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void move_assign(auto_union<T...>& data)
        noexcept((true && ... && traits<T>::is_nothrow_move_assignable)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template get<I>() = std::move(data.template get<I>());
            } else {
                move_assign<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void same_swap(auto_union<T...>& data) noexcept((true && ... && traits<T>::is_nothrow_swappable)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (!std::is_void_v<select_t<I, T...>>) {
                    using std::swap;
                    swap(data_.template get<I>(), data.template get<I>());
                }
            } else {
                same_swap<I + 1>(data);
            }
        }
    }

    template <size_t I, size_t J>
    constexpr void diff_swap_impl(variant_impl<T...>& other) noexcept((true && ... && (traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
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
    constexpr void diff_swap_nested(variant_impl<T...>& other) noexcept((true && ... && (traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        if constexpr (J < sizeof...(T)) {
            if (other.discrim_ == static_cast<discrim_t>(J)) {
                diff_swap_impl<I, J>(other);
            } else {
                diff_swap_nested<I, J + 1>(other);
            }
        }
    }

    template <size_t I>
    constexpr void diff_swap(variant<T...>& other) noexcept((true && ... && (traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                diff_swap_nested<I, 0>(data_.template get<I>(), other);
            } else {
                diff_swap<I + 1>(other);
            }
        }
    }

  public:
    constexpr variant_impl() noexcept(traits<first_t<T...>>::is_nothrow_default_constructible)
        : variant_impl(std::in_place_index<0>) {}

    constexpr variant_impl(const variant_impl& other) : discrim_(other.discrim_) {
        copy_construct<0>(other.data_);
    }

    constexpr variant_impl(variant_impl&& other)
        noexcept((true && ... && traits<T>::is_nothrow_move_constructible))
        : discrim_(other.discrim_) {
        move_construct<0>(other.data_);
    }

    template <size_t I, typename... Args>
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<I> in_place, Args&&... args)
        noexcept(traits<select_t<I, T...>>::template is_nothrow_constructible<Args...>) {
        data_.template construct<I>(std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept((true && ... && traits<T>::is_nothrow_destructible)) {
        destroy<0>();
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (discrim_ == rhs.discrim_) {
                copy_assign<0>(rhs.data_);
            } else {
                destroy<0>();
                copy_construct<0>(rhs.data_);
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs)
        noexcept((true && ... && (traits<T>::is_nothrow_move_assignable && traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
        if (discrim_ == rhs.discrim_) {
            move_assign<0>(rhs.data_);
        } else {
            destroy<0>();
            move_construct<0>(rhs.data_);
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(discrim_);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::reference get() & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::const_reference get() const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::rvalue_reference get() && {
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::const_rvalue_reference get() const&& {
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::pointer ptr() noexcept {
        if constexpr (!std::is_void_v<select_t<I, T...>>) {
            return &data_.template get<I>();
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::const_pointer ptr() const noexcept {
        if constexpr (!std::is_void_v<select_t<I, T...>>) {
            return &data_.template get<I>();
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) {
        destroy<0>();
        data_.template construct<I>(std::forward<Args>(args)...);
        discrim_ = static_cast<discrim_t>(I);
    }

    constexpr void swap(variant_impl& other) noexcept((true && ... && (traits<T>::is_nothrow_swappable && traits<T>::is_nothrow_move_constructible && traits<T>::is_nothrow_destructible))) {
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

    constexpr variant_impl(variant_impl&& other) noexcept(traits<T>::is_nothrow_move_constructible) {
        data_.template construct<0>(other.data_.template get<0>());
    }

    template <typename... Args>
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place, Args&&... args) {
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

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(traits<T>::is_nothrow_move_assignable) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            data_.template construct<0>(rhs.data_.template get<0>());
        } else {
            data_.template get<0>() = std::move(rhs.data_.template get<0>());
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return 0;
    }

    template <size_t I>
    constexpr typename traits<T>::reference get() & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<T>::const_reference get() const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<T>::rvalue_reference get() && {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<T>::const_rvalue_reference get() const&& {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<T>::pointer ptr() noexcept {
        if constexpr (!std::is_void_v<T>) {
            return &data_.template get<I>();
        }
    }

    template <size_t I>
    constexpr typename traits<T>::const_pointer ptr() const noexcept {
        if constexpr (!std::is_void_v<T>) {
            return &data_.template get<I>();
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

    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place) noexcept {}

    template <typename T>
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place, [[maybe_unused]] T&& value) noexcept {}

    constexpr size_t index() const noexcept { return 0; }

    template <size_t I>
    constexpr void get() const noexcept { }

    template <size_t I>
    constexpr void ptr() const noexcept { }

    constexpr void swap([[maybe_unused]] variant_impl& other) noexcept { }
};

template <typename T>
class variant_impl<void, T&, void> {
  private:
    T* data_;

  public:
    variant_impl() = delete;

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place, U& value) noexcept
        : data_(&value) {}

    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place) noexcept
        : data_(nullptr) {}

    template <typename U>
    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place, [[maybe_unused]] U&& value) noexcept
        : data_(nullptr) {}

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(data_ == nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, void>>::const_reference get() const noexcept {
        if constexpr (I == 0) {
            return *data_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, void>>::const_pointer ptr() const noexcept {
        if constexpr (I == 0) {
            return data_;
        }
    }

    template <size_t I>
    constexpr void emplace() noexcept {
        static_assert(I == 1, "no matching constructor for reference");
        data_ = nullptr;
    }

    template <size_t I, typename U>
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        if constexpr (I == 0) {
            static_assert(std::is_lvalue_reference_v<U>, "no matching constructor for reference");
            data_ = &value;
        } else {
            data_ = nullptr;
        }
    }

    constexpr void swap(variant_impl& other) noexcept {
        std::swap(data_, other.data_);
    }
};

template <typename T>
class variant_impl<void, void, T&> {
  private:
    T* data_{nullptr};

  public:
    constexpr variant_impl() noexcept = default;

    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place) noexcept
        : data_(nullptr) {}

    template <typename U>
    explicit constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place, [[maybe_unused]] U&& value) noexcept
        : data_(nullptr) {}

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place, U& value) noexcept
        : data_(&value) {}

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(data_ != nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, void, T&>>::const_reference get() const noexcept {
        if constexpr (I == 1) {
            return *data_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, void, T&>>::const_pointer ptr() const noexcept {
        if constexpr (I == 1) {
            return data_;
        }
    }

    template <size_t I>
    constexpr void emplace() noexcept {
        static_assert(I == 0, "no matching constructor for reference");
        data_ = nullptr;
    }

    template <size_t I, typename U>
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        if constexpr (I == 1) {
            static_assert(std::is_lvalue_reference_v<U>, "no matching constructor for reference");
            data_ = &value;
        } else {
            data_ = nullptr;
        }
    }

    constexpr void swap(variant_impl& other) noexcept {
        std::swap(data_, other.data_);
    }
};

template <typename T, typename U>
class variant_impl<std::enable_if_t<(sizeof(U) <= sizeof(bool))>, T&, U> {
  private:
    T* head_;
    union {
        SUMTY_NO_UNIQ_ADDR U tail_;
    };

  public:
    variant_impl() = delete;

    constexpr variant_impl(const variant_impl& other) : head_(other.head_) {
        if (head_ == nullptr) {
            std::construct_at(&tail_, other.tail_);
        }
    }

    constexpr variant_impl(variant_impl&& other) noexcept(std::is_nothrow_move_constructible_v<U>)
        : head_(other.head_) {
        if (head_ == nullptr) {
            std::construct_at(&tail_, std::move(other.tail_));
        }
    }

    template <typename V>
        requires(std::is_convertible_v<V*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place, V& value) noexcept
        : head_(&value) {}

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place, Args&&... args)
        : head_(nullptr) {
        std::construct_at(&tail_, std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept(std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            std::destroy_at(&tail_);
        }
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (head_ == nullptr) {
                head_ = rhs.head_;
                if (head_ == nullptr) {
                    tail_ = rhs.tail_;
                } else {
                    std::destroy_at(&tail_);
                }
            } else {
                head_ == rhs.head_;
                if (head_ == nullptr) {
                    std::construct_at(&tail_, rhs.tail_);
                }
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(std::is_nothrow_move_assignable_v<U> && std::is_nothrow_move_constructible_v<U> && std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            head_ = rhs.head_;
            if (head_ == nullptr) {
                tail_ = std::move(rhs.tail_);
            } else {
                std::destroy_at(&tail_);
            }
        } else {
            head_ == rhs.head_;
            if (head_ == nullptr) {
                std::construct_at(&tail_, std::move(rhs.tail_));
            }
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(head_ == nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::reference get() & noexcept {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::const_reference get() const& noexcept {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::rvalue_reference get() && {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return std::move(tail_);
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::const_rvalue_reference get() const&& {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return std::move(tail_);
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::pointer ptr() noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return &tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::const_pointer ptr() const noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return &tail_;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        if constexpr (I == 0) {
            static_assert((true && ... && std::is_lvalue_reference_v<Args>) && sizeof...(Args) == 1, "no matching constructor for reference");
            if (head_ != nullptr) {
                std::destroy_at(&tail_);
            }
            head_ = std::addressof(std::forward<Args>(args)...);
        } else {
            if (head_ == nullptr) {
                std::destroy_at(&tail_);
            } else {
                head_ = nullptr;
            }
            std::construct_at(&tail_, std::forward<Args>(args)...);
        }
    }

    constexpr void swap(variant_impl& other) noexcept(std::is_nothrow_swappable_v<U> && std::is_nothrow_move_constructible_v<U> && std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            if (other.head_ == nullptr) {
                using std::swap;
                swap(tail_, other.tail_);
            } else {
                head_ = other.head_;
                other.head_ = nullptr;
                std::construct_at(&other.tail_, std::move(tail_));
                std::destroy_at(&tail_);
            }
        } else {
            if (other.head_ == nullptr) {
                other.head_ = head_;
                head_ = nullptr;
                std::construct_at(&tail_, std::move(other.tail_));
                std::destroy_at(&other.tail_);
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
    union {
        SUMTY_NO_UNIQ_ADDR T head_;
    };

  public:
    constexpr variant_impl() noexcept(std::is_nothrow_default_constructible_v<T>) {
        std::construct_at(&head_);
    }

    constexpr variant_impl(const variant_impl& other) : tail_(other.tail_) {
        if (tail_ == nullptr) {
            std::construct_at(&head_, other.head_);
        }
    }

    constexpr variant_impl(variant_impl&& other) noexcept(std::is_nothrow_move_constructible_v<T>) : tail_(other.tail_) {
        if (tail_ == nullptr) {
            std::construct_at(&head_, std::move(other.head_));
        }
    }

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place, Args&&... args) {
        std::construct_at(&head_, std::forward<Args>(args)...);
    }

    template <typename V>
        requires(std::is_convertible_v<V*, U*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place, V& value) noexcept
        : tail_(&value) {}

    constexpr ~variant_impl() noexcept(std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            std::destroy_at(&head_);
        }
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (tail_ == nullptr) {
                tail_ = rhs.tail_;
                if (tail_ == nullptr) {
                    head_ = rhs.head_;
                } else {
                    std::destroy_at(&head_);
                }
            } else {
                tail_ == rhs.tail_;
                if (head_ == nullptr) {
                    std::construct_at(&head_, rhs.head_);
                }
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            tail_ = rhs.tail_;
            if (tail_ == nullptr) {
                head_ = std::move(rhs.head_);
            } else {
                std::destroy_at(&head_);
            }
        } else {
            tail_ == rhs.tail_;
            if (head_ == nullptr) {
                std::construct_at(&head_, std::move(rhs.head_));
            }
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(tail_ != nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::reference get() & noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::const_reference get() const& noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::rvalue_reference get() && {
        if constexpr (I == 0) {
            return std::move(head_);
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::const_rvalue_reference get() const&& {
        if constexpr (I == 0) {
            return std::move(head_);
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::pointer ptr() noexcept {
        if constexpr (I == 0) {
            return &head_;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::const_pointer ptr() const noexcept {
        if constexpr (I == 0) {
            return &head_;
        } else {
            return tail_;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        if constexpr (I == 1) {
            static_assert((true && ... && std::is_lvalue_reference_v<Args>) && sizeof...(Args) == 1, "no matching constructor for reference");
            if (tail_ != nullptr) {
                std::destroy_at(&head_);
            }
            tail_ = std::addressof(std::forward<Args>(args)...);
        } else {
            if (tail_ == nullptr) {
                std::destroy_at(&head_);
            } else {
                tail_ = nullptr;
            }
            std::construct_at(&head_, std::forward<Args>(args)...);
        }
    }

    constexpr void swap(variant_impl& other) noexcept(std::is_nothrow_swappable_v<T> && std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            if (other.tail_ == nullptr) {
                using std::swap;
                swap(head_, other.head_);
            } else {
                tail_ = other.tail_;
                other.tail_ = nullptr;
                std::construct_at(&other.head_, std::move(head_));
                std::destroy_at(&head_);
            }
        } else {
            if (other.tail_ == nullptr) {
                other.tail_ = tail_;
                tail_ = nullptr;
                std::construct_at(&head_, std::move(other.head_));
                std::destroy_at(&other.head_);
            } else {
                std::swap(tail_, other.tail_);
            }
        }
    }
};

template <typename... T>
using variant_impl_t = variant_impl<void, T...>;

}

template <typename... T>
class variant {
  private:
    SUMTY_NO_UNIQ_ADDR detail::variant_impl_t<T...> data_;

    template <size_t IDX, typename V, typename U>
    static constexpr decltype(auto) visit_impl(V&& visitor, U&& var) {
        if (var.index() == IDX || IDX + 1 == sizeof...(T)) {
            if constexpr (std::is_void_v<detail::select_t<IDX, T...>>) {
                return std::invoke(std::forward<V>(visitor));
            } else {
                return std::invoke(std::forward<V>(visitor), std::forward<U>(var).template get<IDX>());
            }
        } else {
            return visit_impl<IDX + 1>(std::forward<V>(visitor), std::forward<U>(var));
        }
    }

    template <size_t IDX, typename U>
    constexpr bool holds_alt_impl() const noexcept {
        if constexpr (IDX == sizeof...(T)) {
            return false;
        } else if constexpr (std::is_same_v<detail::select_t<IDX, T...>, U>) {
            if (index() == IDX) {
                return true;
            } else {
                return holds_alt_impl<IDX+1, U>();
            }
        } else {
            return holds_alt_impl<IDX+1, U>();
        }
    }

  public:
    constexpr variant() noexcept(detail::traits<detail::first_t<T...>>::is_nothrow_default_constructible) requires(detail::traits<detail::first_t<T...>>::is_default_constructible) = default;

    constexpr variant(const variant&) requires(true && ... && detail::traits<T>::is_copy_constructible) = default;

    constexpr variant(variant&&) noexcept((true && ... && detail::traits<T>::is_nothrow_move_constructible)) requires(true && ... && detail::traits<T>::is_move_constructible) = default;

    template <size_t IDX, typename... Args>
    constexpr variant(std::in_place_index_t<IDX> in_place, Args&&... args)
        : data_(in_place, std::forward<Args>(args)...) {}

    template <size_t IDX, typename U, typename... Args>
    constexpr variant(std::in_place_index_t<IDX> in_place, std::initializer_list<U> init, Args&&... args)
        : data_(in_place, init, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr variant([[maybe_unused]] std::in_place_type_t<U> in_place, Args&&... args)
        : data_(std::in_place_index<detail::index_of_v<U, T...>>, std::forward<Args>(args)...) {}

    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr variant([[maybe_unused]] std::in_place_type_t<U> in_place, std::initializer_list<V> init, Args&&... args)
        : data_(std::in_place_index<detail::index_of_v<U, T...>>, init, std::forward<Args>(args)...) {}

    constexpr ~variant() noexcept((true && ... && detail::traits<T>::is_nothrow_destructible)) = default;

    constexpr variant& operator=(const variant& rhs)
        requires(true && ... && detail::traits<T>::is_copy_assignable)
         = default;

    constexpr variant& operator=(variant&& rhs)
        noexcept((true && ... && (detail::traits<T>::is_nothrow_move_assignable && detail::traits<T>::is_nothrow_destructible && detail::traits<T>::is_nothrow_move_constructible)))
        requires(true && ... && detail::traits<T>::is_move_assignable)
        = default;

    constexpr size_t index() const noexcept {
        return data_.index();
    }

    template <size_t I, typename... Args>
    constexpr decltype(auto) emplace(Args&&... args) {
        data_.template emplace<I>(std::forward<Args>(args)...);
        return data_.template get<I>();
    }

    template <size_t I, typename U, typename... Args>
    constexpr decltype(auto) emplace(std::initializer_list<U> ilist, Args&&... args) {
        data_.template emplace<I>(ilist, std::forward<Args>(args)...);
        return data_.template get<I>();
    }

    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) emplace(Args&&... args) {
        data_.template emplace<detail::index_of_v<U, T...>>(std::forward<Args>(args)...);
        return data_.template get<detail::index_of_v<U, T...>>();
    }

    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) emplace(std::initializer_list<V> ilist, Args&&... args) {
        data_.template emplace<detail::index_of_v<U, T...>>(ilist, std::forward<Args>(args)...);
        return data_.template get<detail::index_of_v<U, T...>>();
    }

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) && {
        return std::move(data_).template get<I>();
    }

    template <size_t I>
    constexpr decltype(auto) operator[](index_t<I> index) const&& {
        return std::move(data_).template get<I>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) & noexcept {
        return data_.template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) const& noexcept {
        return data_.template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) && {
        return std::move(data_).template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) operator[](type_t<U> type) const&& {
        return std::move(data_).template get<detail::index_of_v<U, T...>>();
    }

    template <size_t I>
    constexpr decltype(auto) get() & {
        if (index() != I) {
            throw bad_variant_access();
        }
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr decltype(auto) get() const& {
        if (index() != I) {
            throw bad_variant_access();
        }
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr decltype(auto) get() && {
        if (index() != I) {
            throw bad_variant_access();
        }
        return std::move(data_).template get<I>();
    }

    template <size_t I>
    constexpr decltype(auto) get() const&& {
        if (index() != I) {
            throw bad_variant_access();
        }
        return std::move(data_).template get<I>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() & {
        if (index() == detail::index_of_v<U, T...>) {
            throw bad_variant_access();
        }
        return data_.template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() const& {
        if (index() == detail::index_of_v<U, T...>) {
            throw bad_variant_access();
        }
        return data_.template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() && {
        if (index() == detail::index_of_v<U, T...>) {
            throw bad_variant_access();
        }
        return std::move(data_).template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
        requires(detail::is_unique_v<U, T...>)
    constexpr decltype(auto) get() const&& {
        if (index() == detail::index_of_v<U, T...>) {
            throw bad_variant_access();
        }
        return std::move(data_).template get<detail::index_of_v<U, T...>>();
    }

    template <typename U>
    constexpr bool holds_alternative() const noexcept {
        if constexpr (detail::is_unique_v<U, T...>) {
            return index() == detail::index_of_v<U, T...>;
        } else {
            return holds_alt_impl<0, U>();
        }
    }

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) & {
        return visit_impl(std::forward<V>(visitor), *this);
    }

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) const& {
        return visit_impl(std::forward<V>(visitor), *this);
    }

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) && {
        return visit_impl(std::forward<V>(visitor), std::move(*this));
    }

    template <typename V>
    constexpr decltype(auto) visit(V&& visitor) const&& {
        return visit_impl(std::forward<V>(visitor), std::move(*this));
    }

    constexpr void swap(variant& other) noexcept(noexcept(data_.swap(other.data_))) {
        data_.swap(other.data_);
    }
};

template <typename T, typename... U>
constexpr bool holds_alternative(const variant<U...>& v) noexcept {
    return v.template holds_alternative<T>();
}

template <typename T, typename... U>
constexpr decltype(auto) get(variant<U...>& v) {
    return v.template get<T>();
}

template <typename T, typename... U>
constexpr decltype(auto) get(const variant<U...>& v) {
    return v.template get<T>();
}

template <typename T, typename... U>
constexpr decltype(auto) get(variant<U...>&& v) {
    return std::move(v).template get<T>();
}

template <typename T, typename... U>
constexpr decltype(auto) get(const variant<U...>&& v) {
    return std::move(v).template get<T>();
}

template <typename V>
constexpr decltype(auto) visit(V&& visitor) {
    return std::invoke(std::forward<V>(visitor));
}

template <typename V, typename T0, typename... TN>
constexpr decltype(auto) visit(V&& visitor, T0&& var0, TN&&... varn) {
    return std::forward<T0>(var0).visit([visitor=std::forward<V>(visitor), ...varn=std::forward<TN>(varn)](auto&& value) -> decltype(auto) {
        return visit([visitor=std::forward<V>(visitor), value=std::forward<decltype(value)>(value)](auto&&... args) -> decltype(auto) {
            return std::invoke(std::forward<V>(visitor), std::forward<decltype(value)>(value), std::forward<decltype(args)>(args)...);
        }, std::forward<TN>(varn)...);
    });
}

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

#endif
