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

#include "sumty/option.hpp"
#include "sumty/result.hpp"

namespace sumty {

template <typename T>
constexpr option<T>::option([[maybe_unused]] none_t none) noexcept : option() {}

template <typename T>
constexpr option<T>::option([[maybe_unused]] std::nullopt_t null) noexcept : option() {}

template <typename T>
constexpr option<T>::option([[maybe_unused]] std::nullptr_t null) noexcept
    requires(std::is_lvalue_reference_v<T>)
    : option() {}

template <typename T>
template <typename U>
    requires(std::is_lvalue_reference_v<T> && std::is_convertible_v<U*, typename option<T>::pointer>)
explicit(!std::is_convertible_v<U*, typename option<T>::pointer>)
constexpr option<T>::option(U* ptr) noexcept {
    if (ptr != nullptr) {
        opt_.template emplace<1>(*ptr);
    }
}

template <typename T>
template <typename U>
    requires(std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, typename detail::traits<U>::const_reference>)
explicit(!detail::traits<T>::template is_convertible_from<U>)
constexpr option<T>::option(const option<U>& other) : option() {
    if (other.has_value()) {
        opt_.emplace(*other);
    }
}

template <typename T>
template <typename U>
    requires(std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, typename detail::traits<U>::rvalue_reference>)
explicit(!detail::traits<T>::template is_convertible_from<U>)
constexpr option<T>::option(option<U>&& other) : option() {
    if (other.has_value()) {
        opt_.emplace(*std::move(other));
    }
}

template <typename T>
template <typename... Args>
explicit(sizeof...(Args) == 0)
constexpr option<T>::option([[maybe_unused]] std::in_place_t in_place, Args&&... args)
    : opt_(std::in_place_index<1>, std::forward<Args>(args)...) {}

template <typename T>
template <typename U, typename... Args>
constexpr option<T>::option([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> init, Args&&... args)
    : opt_(std::in_place_index<1>, init, std::forward<Args>(args)...) {}

template <typename T>
template <typename U>
    requires(std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U&&> && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> && (!std::is_same_v<std::remove_const_t<T>, bool> || !detail::is_option_v<U>))
explicit(!detail::traits<T>::template is_convertible_from<U>)
constexpr option<T>::option(U&& value)
    : opt_(std::in_place_index<1>, std::forward<U>(value)) {}

template <typename T>
constexpr option<T>& option<T>::operator=([[maybe_unused]] none_t none) noexcept(detail::traits<T>::is_nothrow_destructible) {
    opt_.template emplace<0>();
    return *this;
}

template <typename T>
constexpr option<T>& option<T>::operator=([[maybe_unused]] std::nullopt_t null) noexcept(detail::traits<T>::is_nothrow_destructible) {
    opt_.template emplace<0>();
    return *this;
}

template <typename T>
constexpr option<T>& option<T>::operator=([[maybe_unused]] std::nullptr_t null) noexcept
    requires(std::is_lvalue_reference_v<T>)
{
    opt_.template emplace<0>();
    return *this;
}

template <typename T>
template <typename U>
    requires(!std::is_same_v<std::remove_cvref_t<U>, option<T>> && std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U&&> && detail::traits<T>::template is_assignable<U&&> && (!std::is_scalar_v<typename option<T>::value_type> || !std::is_same_v<T, std::decay_t<U>>))
constexpr option<T>& option<T>::operator=(U&& value) {
    opt_.template emplace<1>(std::forward<U>(value));
    return *this;
}

template <typename T>
template <typename U>
    requires(std::is_lvalue_reference_v<typename option<T>::value_type> && std::is_convertible_v<U*, typename option<T>::pointer>)
constexpr option<T>& option<T>::operator=(U* ptr) noexcept {
    opt_.set_ptr(ptr);
    return *this;
}

template <typename T>
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
constexpr option<T>& option<T>::operator=(const option<U>& value) {
    if (value.has_value()) {
        opt_.template emplace<1>(*value);
    } else {
        opt_.template emplace<0>();
    }
    return *this;
}

template <typename T>
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
constexpr option<T>& option<T>::operator=(option<U>&& value) {
    if (value.has_value()) {
        opt_.template emplace<1>(*std::move(value));
    } else {
        opt_.template emplace<0>();
    }
    return *this;
}

template <typename T>
constexpr option<T>::operator bool() const noexcept {
    return opt_.index() != 0;
}

template <typename T>
template <typename U>
    requires(std::is_lvalue_reference_v<T> && std::is_assignable_v<typename option<T>::pointer&, U*>)
explicit(!std::is_convertible_v<typename option<T>::pointer, U*>)
constexpr option<T>::operator U*() const noexcept {
    if (opt_.index() == 0) {
        return nullptr;
    } else {
        return static_cast<U*>(&opt_[index<1>]);
    }
}

template <typename T>
constexpr bool option<T>::has_value() const noexcept {
    return opt_.index() != 0;
}

template <typename T>
constexpr typename option<T>::reference option<T>::operator*() & noexcept {
    return opt_[index<1>];
}

template <typename T>
constexpr typename option<T>::const_reference option<T>::operator*() const& noexcept {
    return opt_[index<1>];
}

template <typename T>
constexpr typename option<T>::rvalue_reference option<T>::operator*() && {
    return std::move(opt_)[index<1>];
}

template <typename T>
constexpr typename option<T>::const_rvalue_reference option<T>::operator*() const&& {
    return std::move(opt_)[index<1>];
}

template <typename T>
constexpr typename option<T>::pointer option<T>::operator->() noexcept {
    return opt_.template get_if<1>();
}

template <typename T>
constexpr typename option<T>::const_pointer option<T>::operator->() const noexcept {
    return opt_.template get_if<1>();
}

template <typename T>
constexpr typename option<T>::reference option<T>::value() & {
    if (opt_.index() == 0) {
        throw bad_option_access();
    }
    return opt_[index<1>];
}

template <typename T>
constexpr typename option<T>::const_reference option<T>::value() const& {
    if (opt_.index() == 0) {
        throw bad_option_access();
    }
    return opt_[index<1>];
}

template <typename T>
constexpr typename option<T>::rvalue_reference option<T>::value() && {
    if (opt_.index() == 0) {
        throw bad_option_access();
    }
    return std::move(opt_)[index<1>];
}

template <typename T>
constexpr typename option<T>::rvalue_reference option<T>::value() const&& {
    if (opt_.index() == 0) {
        throw bad_option_access();
    }
    return std::move(opt_)[index<1>];
}

template <typename T>
template <typename U>
constexpr typename option<T>::value_type option<T>::value_or(U&& default_value) const& {
    if (opt_.index() != 0) {
        return opt_[index<1>];
    } else {
        return static_cast<value_type>(std::forward<U>(default_value));
    }
}

template <typename T>
template <typename U>
constexpr typename option<T>::value_type option<T>::value_or(U&& default_value) && {
    if (opt_.index() != 0) {
        return std::move(opt_)[index<1>];
    } else {
        return static_cast<value_type>(std::forward<U>(default_value));
    }
}

template <typename T>
constexpr typename option<T>::value_type option<T>::value_or() const& {
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

template <typename T>
constexpr typename option<T>::value_type option<T>::value_or() && {
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

template <typename T>
template <typename F>
constexpr auto option<T>::and_then(F&& f) & {
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

template <typename T>
template <typename F>
constexpr auto option<T>::and_then(F&& f) const& {
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

template <typename T>
template <typename F>
constexpr auto option<T>::and_then(F&& f) && {
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

template <typename T>
template <typename F>
constexpr auto option<T>::and_then(F&& f) const&& {
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

template <typename T>
template <typename F>
constexpr decltype(auto) option<T>::transform(F&& f) & {
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

template <typename T>
template <typename F>
constexpr decltype(auto) option<T>::transform(F&& f) const& {
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

template <typename T>
template <typename F>
constexpr decltype(auto) option<T>::transform(F&& f) && {
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

template <typename T>
template <typename F>
constexpr decltype(auto) option<T>::transform(F&& f) const&& {
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

template <typename T>
template <typename F>
constexpr option<T> option<T>::or_else(F&& f) const& {
    if (opt_.index() != 0) {
        return *this;
    } else {
        return std::invoke(std::forward<F>(f));
    }
}

template <typename T>
template <typename F>
constexpr option<T> option<T>::or_else(F&& f) && {
    if (opt_.index() != 0) {
        return std::move(*this);
    } else {
        return std::invoke(std::forward<F>(f));
    }
}

template <typename T>
constexpr void option<T>::swap(option& other) noexcept(noexcept(opt_.swap(other.opt_))) {
    opt_.swap(other.opt_);
}

template <typename T>
constexpr void option<T>::reset() noexcept {
    opt_.template emplace<0>();
}

template <typename T>
template <typename... Args>
constexpr typename option<T>::reference option<T>::emplace(Args&&... args) {
    opt_.template emplace<1>(std::forward<Args>(args)...);
    return opt_[index<1>];
}

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
