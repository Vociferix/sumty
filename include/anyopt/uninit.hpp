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

#ifndef ANYOPT_UNINIT_HPP
#define ANYOPT_UNINIT_HPP

#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace anyopt {

template <typename T>
class uninit {
  private:
    union {
#if defined(_MSC_VER) && !defined(__clang__)
        [[msvc::no_unique_address]]
#else
        [[no_unique_address]]
#endif
        T data_;
    };

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = const T&&;
    using pointer = T*;
    using const_pointer = const T*;

    constexpr uninit() noexcept {}

    constexpr uninit([[maybe_unused]] const uninit& other) noexcept {}

    constexpr uninit([[maybe_unused]] uninit&& other) noexcept {}

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
    constexpr uninit([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : data_{std::forward<Args>(args)...} {}

    template <typename U, typename... Args>
    constexpr uninit([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
        : data_{init, std::forward<Args>(args)...} {}

    constexpr ~uninit() noexcept {}

    constexpr uninit& operator=([[maybe_unused]] const uninit& rhs) noexcept {
        return *this;
    }

    constexpr uninit& operator=([[maybe_unused]] uninit&& rhs) noexcept {
        return *this;
    }

    template <typename... Args>
    constexpr T& construct(Args&&... args) {
        return *std::construct_at(&data_, std::forward<Args>(args)...);
    }

    constexpr void destroy() noexcept(std::is_nothrow_destructible_v<T>) {
        std::destroy_at(&data_);
    }

    constexpr reference operator*() noexcept {
        return data_;
    }

    constexpr const_reference operator*() const noexcept {
        return data_;
    }
};

}

#endif
