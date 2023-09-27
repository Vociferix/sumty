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

template <size_t N>
struct index_t {
    template <typename S>
    constexpr decltype(auto) operator()(S&& s) const
        requires requires { std::forward<S>(s)[index_t<N>{}]; }
    {
        return std::forward<S>(s)[index_t<N>{}];
    }
};

template <size_t N>
static inline constexpr index_t<N> index{};

template <typename T>
struct type_t {
    template <typename S>
    constexpr decltype(auto) operator()(S&& s) const
        requires requires { std::forward<S>(s)[type_t<T>{}]; }
    {
        return std::forward<S>(s)[type_t<T>{}];
    }
};

template <typename T>
static inline constexpr type_t<T> type{};

struct none_t {};

static inline constexpr none_t none{};

} // namespace sumty

#endif
