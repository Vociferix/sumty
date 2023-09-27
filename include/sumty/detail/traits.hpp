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

#ifndef SUMTY_DETAIL_TRAITS_HPP
#define SUMTY_DETAIL_TRAITS_HPP

#include <type_traits>

namespace sumty::detail {

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
    static inline constexpr bool is_convertible_from = std::is_convertible_v<U, value_type> || (std::is_void_v<U> && is_default_constructible);

    template <typename... U>
    static inline constexpr bool is_constructible = std::is_constructible_v<value_type, U...> || ((sizeof...(U) == 1 && is_default_constructible) && ... && std::is_void_v<U>);

    template <typename U>
    static inline constexpr bool is_assignable = std::is_assignable_v<reference, U> || (std::is_void_v<U> && is_default_constructible);

    template <typename... U>
    static inline constexpr bool is_nothrow_constructible = std::is_nothrow_constructible_v<value_type, U...> || ((sizeof...(U) == 1 && is_nothrow_default_constructible) && ... && std::is_void_v<U>);

    template <typename U>
    static inline constexpr bool is_nothrow_assignable = std::is_nothrow_assignable_v<reference, U> || (std::is_void_v<U> && is_nothrow_default_constructible);
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

    template <typename U>
    static inline constexpr bool is_nothrow_assignable = is_assignable<U>;
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

    template <typename U>
    static inline constexpr bool is_nothrow_assignable = true;
};

}

#endif
