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

#ifndef SUMTY_DETAIL_AUTO_UNION_HPP
#define SUMTY_DETAIL_AUTO_UNION_HPP

#include "sumty/detail/traits.hpp"
#include "sumty/detail/utils.hpp"
#include "sumty/utils.hpp" // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

// NOLINTBEGIN(cert-oop54-cpp)
// NOLINTBEGIN(bugprone-unhandled-self-assignment)

namespace sumty::detail {

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
    [[nodiscard]] constexpr typename traits<select_t<IDX, T0, TN...>>::reference
    get() noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX - 1>();
        }
    }

    template <size_t IDX>
    [[nodiscard]] constexpr typename traits<select_t<IDX, T0, TN...>>::const_reference get()
        const noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX - 1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct(Args&&... args) {
        if constexpr (IDX == 0) {
            std::construct_at(&head_, std::forward<Args>(args)...);
        } else {
            tail_.template construct<IDX - 1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX == 0) {
            std::destroy_at(&head_);
        } else {
            tail_.template destroy<IDX - 1>();
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
    [[nodiscard]] constexpr typename traits<select_t<IDX, T0, TN...>>::reference
    get() noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX - 1>();
        }
    }

    template <size_t IDX>
    [[nodiscard]] constexpr typename traits<select_t<IDX, T0, TN...>>::const_reference get()
        const noexcept {
        if constexpr (IDX == 0) {
            return head_;
        } else {
            return tail_.template get<IDX - 1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct(Args&&... args) {
        if constexpr (IDX == 0) {
            std::construct_at(&head_, std::forward<Args>(args)...);
        } else {
            tail_.template construct<IDX - 1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX == 0) {
            std::destroy_at(&head_);
        } else {
            tail_.template destroy<IDX - 1>();
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
    [[nodiscard]] constexpr typename traits<select_t<IDX, T0&, TN...>>::reference
    get() noexcept {
        if constexpr (IDX == 0) {
            return *head_;
        } else {
            return tail_.template get<IDX - 1>();
        }
    }

    template <size_t IDX>
    [[nodiscard]] constexpr typename traits<select_t<IDX, T0&, TN...>>::const_reference
    get() const noexcept {
        if constexpr (IDX == 0) {
            return *head_;
        } else {
            return tail_.template get<IDX - 1>();
        }
    }

    template <size_t IDX, typename... Args>
    void construct(Args&&... args) {
        if constexpr (IDX == 0) {
            head_ = std::addressof(std::forward<Args>(args)...);
        } else {
            tail_.template construct<IDX - 1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX != 0) { tail_.template destroy<IDX - 1>(); }
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
    [[nodiscard]] constexpr typename traits<select_t<IDX, void, TN...>>::reference
    get() noexcept {
        if constexpr (IDX != 0) {
            return tail_.template get<IDX - 1>();
        } else {
            return;
        }
    }

    template <size_t IDX>
    [[nodiscard]] constexpr typename traits<select_t<IDX, void, TN...>>::const_reference
    get() const noexcept {
        if constexpr (IDX != 0) {
            return tail_.template get<IDX - 1>();
        } else {
            return;
        }
    }

    template <size_t IDX, typename... Args>
    void construct([[maybe_unused]] Args&&... args) {
        if constexpr (IDX != 0) {
            tail_.template construct<IDX - 1>(std::forward<Args>(args)...);
        }
    }

    template <size_t IDX>
    void destroy() {
        if constexpr (IDX != 0) { tail_.template destroy<IDX - 1>(); }
    }
};

} // namespace sumty::detail

// NOLINTEND(bugprone-unhandled-self-assignment)
// NOLINTEND(cert-oop54-cpp)

#endif
