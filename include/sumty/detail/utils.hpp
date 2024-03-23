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

#ifndef SUMTY_DETAIL_UTILS_HPP
#define SUMTY_DETAIL_UTILS_HPP

#include "sumty/detail/fwd.hpp"
#include "sumty/utils.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace sumty::detail {

template <uint64_t COUNT, typename = void>
struct discriminant {
    using type = uint64_t;
};

template <uint64_t COUNT>
struct discriminant<COUNT, std::enable_if_t<(COUNT <= 1)>> {
    using type = bool;
};

template <uint64_t COUNT>
struct discriminant<
    COUNT,
    std::enable_if_t<(COUNT > 1 && COUNT <= (std::numeric_limits<uint8_t>::max)())>> {
    using type = uint8_t;
};

template <uint64_t COUNT>
struct discriminant<COUNT,
                    std::enable_if_t<(COUNT <= (std::numeric_limits<uint16_t>::max)() &&
                                      COUNT > (std::numeric_limits<uint8_t>::max)())>> {
    using type = uint16_t;
};

template <uint64_t COUNT>
struct discriminant<COUNT,
                    std::enable_if_t<(COUNT <= (std::numeric_limits<uint32_t>::max)() &&
                                      COUNT > (std::numeric_limits<uint16_t>::max)())>> {
    using type = uint32_t;
};

template <uint64_t COUNT>
using discriminant_t = typename discriminant<COUNT>::type;

template <size_t IDX, typename... T>
struct select;

template <typename T0, typename... TN>
struct select<0, T0, TN...> {
    using type = T0;
};

template <size_t IDX, typename T0, typename... TN>
struct select<IDX, T0, TN...> {
    using type = typename select<IDX - 1, TN...>::type;
};

template <size_t IDX>
struct select<IDX> {
    using type = void;
};

template <size_t IDX, typename... T>
using select_t = typename select<IDX, T...>::type;

template <typename... T>
using first_t = select_t<0, T...>;

template <typename... T>
struct type_list {
    template <size_t I>
    using nth = select_t<I, T...>;

    template <typename U>
    using push_back = type_list<T..., U>;

    template <typename U>
    using push_front = type_list<U, T...>;
};

template <typename F, typename RealArgs, typename... Args>
struct invoke_result_impl;

template <typename F, typename... Args>
struct invoke_result_impl<F, type_list<Args...>> {
    using type = std::invoke_result_t<F, Args...>;
};

template <typename F, typename RealArgs, typename... Args>
struct invoke_result_impl<F, RealArgs, void, Args...>
    : invoke_result_impl<F, RealArgs, void_t, Args...> {};

template <typename F, typename RealArgs, typename FirstArg, typename... Args>
struct invoke_result_impl<F, RealArgs, FirstArg, Args...>
    : invoke_result_impl<F, typename RealArgs::template push_back<FirstArg>, Args...> {};

template <typename F, typename... Args>
struct invoke_result : invoke_result_impl<F, type_list<>, Args...> {};

template <typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

template <size_t N, typename T, typename... U>
struct type_count_impl;

template <size_t N, typename T>
struct type_count_impl<N, T> : std::integral_constant<size_t, N> {};

template <size_t N, typename T, typename U0, typename... UN>
struct type_count_impl<N, T, U0, UN...> : type_count_impl<N, T, UN...> {};

template <size_t N, typename T, typename... UN>
struct type_count_impl<N, T, T, UN...> : type_count_impl<N + 1, T, UN...> {};

template <typename T, typename... U>
struct type_count : type_count_impl<0, T, U...> {};

template <typename T, typename... U>
static inline constexpr size_t type_count_v = type_count<T, U...>::value;

template <typename T, typename... U>
struct is_unique : std::integral_constant<bool, type_count_v<T, U...> == 1> {};

template <typename T, typename... U>
static inline constexpr bool is_unique_v = is_unique<T, U...>::value;

template <typename... T>
struct all_unique;

template <>
struct all_unique<> : std::true_type {};

template <typename T>
struct all_unique<T> : std::true_type {};

template <typename T0, typename... TN>
struct all_unique<T0, TN...>
    : std::integral_constant<bool, is_unique_v<T0, TN...> && all_unique<TN...>::value> {};

template <typename... T>
static inline constexpr bool all_unique_v = all_unique<T...>::value;

template <typename T, typename... U>
struct is_uniquely_convertible
    : std::integral_constant<bool,
                             (0 + ... + static_cast<size_t>(std::is_convertible_v<T, U>)) ==
                                 1> {};

template <typename T, typename... U>
static inline constexpr bool is_uniquely_convertible_v =
    is_uniquely_convertible<T, U...>::value;

template <typename T, typename... U>
struct is_uniquely_constructible
    : std::integral_constant<bool,
                             (0 + ... +
                              static_cast<size_t>(std::is_constructible_v<U, T>)) == 1> {};

template <typename T, typename... U>
static inline constexpr bool is_uniquely_constructible_v =
    is_uniquely_constructible<T, U...>::value;

template <typename T, typename... U>
struct is_uniquely_explicitly_constructible
    : std::integral_constant<bool,
                             (0 + ... +
                              static_cast<size_t>(std::is_constructible_v<U, T> &&
                                                  !std::is_convertible_v<T, U>)) == 1> {};

template <typename T, typename... U>
static inline constexpr bool is_uniquely_explicitly_constructible_v =
    is_uniquely_explicitly_constructible<T, U...>::value;

template <typename T, typename... U>
struct is_uniquely_assignable
    : std::integral_constant<bool,
                             (0 + ... + static_cast<size_t>(std::is_assignable_v<U, T>)) ==
                                 1> {};

template <typename T, typename... U>
static inline constexpr bool is_uniquely_assignable_v =
    is_uniquely_assignable<T, U...>::value;

template <typename T, typename U0, typename... UN>
constexpr size_t index_of_impl() noexcept {
    if constexpr (std::is_same_v<T, U0>) {
        return 0;
    } else {
        return 1 + index_of_impl<T, UN...>();
    }
}

template <typename T, typename... U>
struct index_of : std::integral_constant<size_t, index_of_impl<T, U...>()> {};

template <typename T, typename... U>
static inline constexpr size_t index_of_v = index_of<T, U...>::value;

template <typename T>
struct is_variant : std::false_type {};

template <typename... T>
struct is_variant<variant<T...>> : std::true_type {};

template <typename T>
static inline constexpr bool is_variant_v = is_variant<T>::value;

template <typename T>
struct is_option : std::false_type {};

template <typename T>
struct is_option<option<T>> : std::true_type {};

template <typename T>
static inline constexpr bool is_option_v = is_option<T>::value;

template <typename T>
struct is_result : std::false_type {};

template <typename T, typename E>
struct is_result<result<T, E>> : std::true_type {};

template <typename T>
static inline constexpr bool is_result_v = is_result<T>::value;

template <typename T>
struct is_ok : std::false_type {};

template <typename T>
struct is_ok<ok_t<T>> : std::true_type {};

template <typename T>
static inline constexpr bool is_ok_v = is_ok<T>::value;

template <typename T>
struct is_error : std::false_type {};

template <typename E>
struct is_error<error_t<E>> : std::true_type {};

template <typename T>
static inline constexpr bool is_error_v = is_error<T>::value;

template <typename ES1, typename ES2>
struct is_subset_of_impl;

template <typename... T>
struct is_subset_of_impl<error_set<>, error_set<T...>> : std::true_type {};

template <typename T11, typename... T1N, typename... T2>
struct is_subset_of_impl<error_set<T11, T1N...>, error_set<T2...>>
    : std::integral_constant<
          bool,
          is_unique_v<T11, T2...> &&
              is_subset_of_impl<error_set<T1N...>, error_set<T2...>>::value> {};

template <typename ES1, typename ES2>
static inline constexpr bool is_subset_of_impl_v = is_subset_of_impl<ES1, ES2>::value;

template <typename ES1, typename ES2>
struct is_equivalent_impl : std::integral_constant<bool,
                                                   is_subset_of_impl_v<ES1, ES2> &&
                                                       is_subset_of_impl_v<ES2, ES1>> {};

template <typename ES1, typename ES2>
static inline constexpr bool is_equivalent_impl_v = is_equivalent_impl<ES1, ES2>::value;

template <typename T>
struct is_error_set : std::false_type {};

template <typename... T>
struct is_error_set<error_set<T...>> : std::true_type {};

template <typename T>
static inline constexpr bool is_error_set_v = is_error_set<T>::value;

} // namespace sumty::detail

#endif
