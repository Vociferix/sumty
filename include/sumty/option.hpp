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

#ifndef SUMTY_OPTION_HPP
#define SUMTY_OPTION_HPP

#include <sumty/variant.hpp>

#include <cstddef>
#include <exception>
#include <functional>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

namespace sumty {

class bad_option_access : public std::exception {
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
struct is_option : std::false_type {};

template <typename T>
struct is_option<option<T>> : std::true_type {};

template <typename T>
static inline constexpr bool is_option_v = is_option<T>::value;

}

template <typename T>
class option {
  private:
    variant<void, T> opt_{};

  public:
    using value_type = typename detail::traits<T>::value_type;
    using reference = typename detail::traits<T>::reference;
    using const_reference = typename detail::traits<T>::const_reference;
    using rvalue_reference = typename detail::traits<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<T>::const_rvalue_reference;
    using pointer = typename detail::traits<T>::pointer;
    using const_pointer = typename detail::traits<T>::const_pointer;

    constexpr option() noexcept = default;

    constexpr option(const option&) noexcept(detail::traits<T>::is_nothrow_copy_constructible) = default;

    constexpr option(option&&) noexcept(detail::traits<T>::is_nothrow_move_constructible) = default;

    constexpr option([[maybe_unused]] none_t none) noexcept : option() {}

    constexpr option([[maybe_unused]] std::nullopt_t null) noexcept : option() {}

    constexpr option([[maybe_unused]] std::nullptr_t null) noexcept
        requires(std::is_lvalue_reference_v<T>)
        : option() {}

    template <typename U>
        requires(std::is_lvalue_reference_v<value_type> && std::is_convertible_v<U*, detail::traits<T>::pointer>)
    explicit(!std::is_convertible_v<U*, pointer>)
    constexpr option(U* ptr) noexcept {
        if (ptr != nullptr) {
            opt_.template emplace<1>(*ptr);
        }
    }

    template <typename U>
        requires(std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, typename detail::traits<U>::const_reference>)
    explicit(!detail::traits<T>::template is_convertible_from<U>)
    constexpr option(const option<U>& other) : option() {
        if (other.has_value()) {
            opt_.emplace(*other);
        }
    }

    template <typename U>
        requires(std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, typename detail::traits<U>::rvalue_reference>)
    explicit(!detail::traits<T>::template is_convertible_from<U>)
    constexpr option(option<U>&& other) : option() {
        if (other.has_value()) {
            opt_.emplace(*std::move(other));
        }
    }

    template <typename... Args>
    explicit(sizeof...(Args) == 0)
    constexpr option([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : opt_(std::in_place_index<1>, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr option([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
        : opt_(std::in_place_index<1>, init, std::forward<Args>(args)...) {}

    template <typename U>
        requires(std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U&&> && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> && (!std::is_same_v<std::remove_const_t<T>, bool> || !detail::is_option_v<U>))
    explicit(!detail::traits<T>::template is_convertible_from<U>)
    constexpr option(U&& value)
        : opt_(std::in_place_index<1>, std::forward<U>(value)) {}

    constexpr ~option() noexcept(detail::traits<T>::is_nothrow_destructible) = default;

    constexpr option& operator=(const option&) noexcept(detail::traits<T>::is_nothrow_copy_assignable && detail::traits<T>::is_nothrow_copy_constructible && detail::traits<T>::is_nothrow_destructible) = default;

    constexpr option& operator=(option&&) noexcept(detail::traits<T>::is_nothrow_move_assignable && detail::traits<T>::is_nothrow_move_constructible && detail::traits<T>::is_nothrow_destructible) = default;

    constexpr option& operator=([[maybe_unused]] none_t none) noexcept(detail::traits<T>::is_nothrow_destructible) {
        opt_.template emplace<0>();
        return *this;
    }

    constexpr option& operator=([[maybe_unused]] std::nullopt_t null) noexcept(detail::traits<T>::is_nothrow_destructible) {
        opt_.template emplace<0>();
        return *this;
    }

    constexpr option& operator=([[maybe_unused]] std::nullptr_t null) noexcept
        requires(std::is_lvalue_reference_v<T>)
    {
        opt_.template emplace<0>();
        return *this;
    }

    template <typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, option<T>> && std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U&&> && detail::traits<T>::template is_assignable<U&&> && (!std::is_scalar_v<value_type> || !std::is_same_v<T, std::decay_t<U>>))
    constexpr option& operator=(U&& value) {
        opt_.template emplace<1>(std::forward<U>(value));
        return *this;
    }

    template <typename U>
        requires(std::is_lvalue_reference_v<value_type> && std::is_convertible_v<U*, pointer>)
    constexpr option& operator=(U* ptr) noexcept {
        opt_.set_ptr(ptr);
        return *this;
    }

    template <typename U>
        requires(!detail::traits<T>::template is_constructible<option<U>&> &&
                 !detail::traits<T>::template is_constructible<const option<U>&> &&
                 !detail::traits<T>::template is_constructible<option<U>&&> &&
                 !detail::traits<T>::template is_constructible<const option<U>&&> &&
                 !detail::traits<T>::template is_convertible_from<option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<option<U>&&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U>&&> &&
                 !detail::traits<T>::template is_assignable<option<U>&> &&
                 !detail::traits<T>::template is_assignable<const option<U>&> &&
                 !detail::traits<T>::template is_assignable<option<U>&&> &&
                 !detail::traits<T>::template is_assignable<const option<U>&&> &&
                 (std::is_lvalue_reference_v<T> || detail::traits<T>::template is_constructible<typename detail::traits<U>::const_reference>) &&
                 detail::traits<T>::template is_assignable<typename detail::traits<U>::const_reference>)
    constexpr option& operator=(const option<U>& value) {
        if (value.has_value()) {
            opt_.template emplace<1>(*value);
        } else {
            opt_.template emplace<0>();
        }
        return *this;
    }

    template <typename U>
        requires(!detail::traits<T>::template is_constructible<option<U>&> &&
                 !detail::traits<T>::template is_constructible<const option<U>&> &&
                 !detail::traits<T>::template is_constructible<option<U>&&> &&
                 !detail::traits<T>::template is_constructible<const option<U>&&> &&
                 !detail::traits<T>::template is_convertible_from<option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<option<U>&&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U>&&> &&
                 !detail::traits<T>::template is_assignable<option<U>&> &&
                 !detail::traits<T>::template is_assignable<const option<U>&> &&
                 !detail::traits<T>::template is_assignable<option<U>&&> &&
                 !detail::traits<T>::template is_assignable<const option<U>&&> &&
                 (std::is_lvalue_reference_v<T> || detail::traits<T>::template is_constructible<typename detail::traits<U>::rvalue_reference>) &&
                 detail::traits<T>::template is_assignable<typename detail::traits<U>::rvalue_reference>)
    constexpr option& operator=(option<U>&& value) {
        if (value.has_value()) {
            opt_.template emplace<1>(*std::move(value));
        } else {
            opt_.template emplace<0>();
        }
        return *this;
    }

    constexpr operator bool() const noexcept {
        return opt_.index() != 0;
    }

    template <typename U>
        requires(std::is_lvalue_reference_v<T> && std::is_assignable_v<pointer&, U*>)
    explicit(!std::is_convertible_v<pointer, U*>)
    constexpr operator U*() const noexcept {
        if (opt_.index() == 0) {
            return nullptr;
        } else {
            return static_cast<U*>(&opt_[index<1>]);
        }
    }

    constexpr bool has_value() const noexcept {
        return opt_.index() != 0;
    }

    constexpr reference operator*() & noexcept {
        return opt_[index<1>];
    }

    constexpr const_reference operator*() const& noexcept {
        return opt_[index<1>];
    }

    constexpr rvalue_reference operator*() && {
        return std::move(opt_)[index<1>];
    }

    constexpr const_rvalue_reference operator*() const&& {
        return std::move(opt_)[index<1>];
    }

    constexpr pointer operator->() noexcept {
        return opt_.template get_if<1>();
    }

    constexpr const_pointer operator->() const noexcept {
        return opt_.template get_if<1>();
    }

    constexpr reference value() & {
        if (opt_.index() == 0) {
            throw bad_option_access();
        }
        return opt_[index<1>];
    }

    constexpr const_reference value() const& {
        if (opt_.index() == 0) {
            throw bad_option_access();
        }
        return opt_[index<1>];
    }

    constexpr rvalue_reference value() && {
        if (opt_.index() == 0) {
            throw bad_option_access();
        }
        return std::move(opt_)[index<1>];
    }

    constexpr rvalue_reference value() const&& {
        if (opt_.index() == 0) {
            throw bad_option_access();
        }
        return std::move(opt_)[index<1>];
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) const& {
        if (opt_.index() != 0) {
            return opt_[index<1>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) && {
        if (opt_.index() != 0) {
            return std::move(opt_)[index<1>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    constexpr value_type value_or() const& {
        if (opt_.index() != 0) {
            return opt_[index<1>];
        } else {
            if constexpr (std::is_void_v<value_type>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    constexpr value_type value_or() && {
        if (opt_.index() != 0) {
            return std::move(opt_)[index<1>];
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
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f), opt_[index<1>]);
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const& {
        if constexpr (std::is_void_v<const_reference>) {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f), opt_[index<1>]);
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) && {
        if constexpr (std::is_void_v<rvalue_reference>) {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f), std::move(opt_)[index<1>]);
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const&& {
        if constexpr (std::is_void_v<const_rvalue_reference>) {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F>>{};
            }
        } else {
            if (opt_.index() != 0) {
                return std::invoke(std::forward<F>(f), std::move(opt_)[index<1>]);
            } else {
                return std::remove_cvref_t<std::invoke_result_t<F, const_rvalue_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr decltype(auto) transform(F&& f) & {
        if constexpr (std::is_void_v<reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, reference>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), opt_[index<1>]);
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f), opt_[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr decltype(auto) transform(F&& f) const& {
        if constexpr (std::is_void_v<const_reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, const_reference>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), opt_[index<1>]);
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f), opt_[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr decltype(auto) transform(F&& f) && {
        if constexpr (std::is_void_v<rvalue_reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, rvalue_reference>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(opt_)[index<1>]);
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f), std::move(opt_)[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr decltype(auto) transform(F&& f) const&& {
        if constexpr (std::is_void_v<const_rvalue_reference>) {
            using res_t = std::invoke_result_t<F>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                return option<res_t>{};
            }
        } else {
            using res_t = std::invoke_result_t<F, const_rvalue_reference>;
            if (opt_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(opt_)[index<1>]);
                    return option<res_t>{std::in_place};
                } else {
                    return option<res_t>{std::in_place, std::invoke(std::forward<F>(f), std::move(opt_)[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr option or_else(F&& f) const& {
        if (opt_.index() != 0) {
            return *this;
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    template <typename F>
    constexpr option or_else(F&& f) && {
        if (opt_.index() != 0) {
            return std::move(*this);
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    constexpr void swap(option& other) noexcept(noexcept(opt_.swap(other.opt_))) {
        opt_.swap(other.opt_);
    }

    constexpr void reset() noexcept {
        opt_.template emplace<0>();
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        opt_.template emplace<1>(std::forward<Args>(args)...);
        return opt_[index<1>];
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
