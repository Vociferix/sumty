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
struct discriminant<COUNT,
                    std::enable_if_t<(COUNT <= static_cast<uint64_t>(~uint8_t{0}))>> {
    using type = uint8_t;
};

template <uint64_t COUNT>
struct discriminant<COUNT,
                    std::enable_if_t<(COUNT <= static_cast<uint64_t>(~uint16_t{0}) &&
                                      COUNT > static_cast<uint64_t>(~uint8_t{0}))>> {
    using type = uint16_t;
};

template <uint64_t COUNT>
struct discriminant<COUNT,
                    std::enable_if_t<(COUNT <= static_cast<uint64_t>(~uint32_t{0}) &&
                                      COUNT > static_cast<uint64_t>(~uint16_t{0}))>> {
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

template <size_t IDX, typename T, typename... U>
struct index_of_impl;

template <size_t IDX, typename T, typename U0, typename... UN>
struct index_of_impl<IDX, T, U0, UN...> : index_of_impl<IDX + 1, T, UN...> {};

template <size_t IDX, typename T, typename... UN>
struct index_of_impl<IDX, T, T, UN...> : std::integral_constant<size_t, IDX> {};

template <typename T, typename... U>
struct index_of : index_of_impl<0, T, U...> {};

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

} // namespace sumty::detail

#endif
