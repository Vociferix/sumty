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

#ifndef SUMTY_UTILS_HPP
#define SUMTY_UTILS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#if defined(_MSC_VER) && !defined(__clang__)
#define SUMTY_NO_UNIQ_ADDR [[msvc::no_unique_address]]
#else
#define SUMTY_NO_UNIQ_ADDR [[no_unique_address]]
#endif

namespace sumty {

using std::in_place_index_t;
using std::in_place_t;
using std::in_place_type_t;
using in_place_error_t = in_place_index_t<1>;

using std::in_place;
using std::in_place_index;
using std::in_place_type;
static inline constexpr in_place_error_t in_place_error = in_place_index<1>;

/// @relates variant
template <size_t N>
struct index_t {
    template <typename S>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        operator()(S&& s) const
#ifndef DOXYGEN
        requires requires { std::forward<S>(s)[index_t<N>{}]; }
#endif
    {
        return std::forward<S>(s)[index_t<N>{}];
    }
};

/// @relates index_t
template <size_t N>
static inline constexpr index_t<N> index{};

/// @relates index_t
template <size_t N>
static inline constexpr index_t<N> index_v{};

/// @relates variant
template <typename T>
struct type_t {
    template <typename S>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        operator()(S&& s) const
#ifndef DOXYGEN
        requires requires { std::forward<S>(s)[type_t<T>{}]; }
#endif
    {
        return std::forward<S>(s)[type_t<T>{}];
    }
};

/// @relates type_t
template <typename T>
static inline constexpr type_t<T> type{};

/// @relates type_t
template <typename T>
static inline constexpr type_t<T> type_v{};

/// @relates variant
struct void_t {
    constexpr void_t() noexcept = default;

    constexpr void_t(const void_t&) noexcept = default;

    constexpr void_t(void_t&&) noexcept = default;

    constexpr ~void_t() noexcept = default;

    constexpr void_t& operator=(const void_t&) noexcept = default;

    constexpr void_t& operator=(void_t&&) noexcept = default;

    template <typename T>
        requires(std::is_default_constructible_v<T>)
    explicit constexpr operator T() const
        noexcept(std::is_nothrow_default_constructible_v<T>) {
        return T{};
    }
};

/// @relates void_t
static inline constexpr void_t void_v{};

/// @relates option
struct none_t {};

/// @relates none_t
static inline constexpr none_t none{};

/// @relates none_t
static inline constexpr none_t none_v{};

template <typename... T>
struct overload_t : T... {
    using T::operator()...;
};

/// @relates overload_t
template <typename... T>
constexpr overload_t<T...> overload(T&&... funcs) {
    return overload_t<T...>{std::forward<T>(funcs)...};
}

} // namespace sumty

#endif
