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

#ifndef SUMTY_IMPL_VARIANT_HPP
#define SUMTY_IMPL_VARIANT_HPP

#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/utils.hpp"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <utility>

namespace sumty {

template <typename... T>
template <size_t IDX, typename V, typename U>
constexpr decltype(auto) variant<T...>::visit_impl(V&& visitor, U&& var) {
    if (var.index() == IDX || IDX + 1 == sizeof...(T)) {
        if constexpr (std::is_void_v<detail::select_t<IDX, T...>>) {
            return std::invoke(std::forward<V>(visitor));
        } else {
            return std::invoke(std::forward<V>(visitor),
                               std::forward<U>(var).template get<IDX>());
        }
    } else {
        return visit_impl<IDX + 1>(std::forward<V>(visitor), std::forward<U>(var));
    }
}

template <typename... T>
template <size_t IDX, typename U>
constexpr bool variant<T...>::holds_alt_impl() const noexcept {
    if constexpr (IDX == sizeof...(T)) {
        return false;
    } else if constexpr (std::is_same_v<detail::select_t<IDX, T...>, U>) {
        if (index() == IDX) {
            return true;
        } else {
            return holds_alt_impl<IDX + 1, U>();
        }
    } else {
        return holds_alt_impl<IDX + 1, U>();
    }
}

template <typename... T>
template <size_t IDX, typename... Args>
constexpr variant<T...>::variant(std::in_place_index_t<IDX> inplace, Args&&... args)
    : data_(inplace, std::forward<Args>(args)...) {}

template <typename... T>
template <size_t IDX, typename U, typename... Args>
constexpr variant<T...>::variant(std::in_place_index_t<IDX> inplace,
                                 std::initializer_list<U> init,
                                 Args&&... args)
    : data_(inplace, init, std::forward<Args>(args)...) {}

template <typename... T>
template <typename U, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr variant<T...>::variant([[maybe_unused]] std::in_place_type_t<U> inplace,
                                 Args&&... args)
    : data_(std::in_place_index<detail::index_of_v<U, T...>>, std::forward<Args>(args)...) {
}

template <typename... T>
template <typename U, typename V, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr variant<T...>::variant([[maybe_unused]] std::in_place_type_t<U> inplace,
                                 std::initializer_list<V> init,
                                 Args&&... args)
    : data_(std::in_place_index<detail::index_of_v<U, T...>>,
            init,
            std::forward<Args>(args)...) {}

template <typename... T>
template <size_t I, typename... Args>
constexpr decltype(auto) variant<T...>::emplace(Args&&... args) {
    data_.template emplace<I>(std::forward<Args>(args)...);
    return data_.template get<I>();
}

template <typename... T>
template <size_t I, typename U, typename... Args>
constexpr decltype(auto) variant<T...>::emplace(std::initializer_list<U> ilist,
                                                Args&&... args) {
    data_.template emplace<I>(ilist, std::forward<Args>(args)...);
    return data_.template get<I>();
}

template <typename... T>
template <typename U, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::emplace(Args&&... args) {
    data_.template emplace<detail::index_of_v<U, T...>>(std::forward<Args>(args)...);
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U, typename V, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::emplace(std::initializer_list<V> ilist,
                                                Args&&... args) {
    data_.template emplace<detail::index_of_v<U, T...>>(ilist, std::forward<Args>(args)...);
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](
    [[maybe_unused]] index_t<I> index) & noexcept {
    return data_.template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](
    [[maybe_unused]] index_t<I> index) const& noexcept {
    return data_.template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[]([[maybe_unused]] index_t<I> index) && {
    return std::move(data_).template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](
    [[maybe_unused]] index_t<I> index) const&& {
    return std::move(data_).template get<I>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](
    [[maybe_unused]] type_t<U> type) & noexcept {
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](
    [[maybe_unused]] type_t<U> type) const& noexcept {
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[]([[maybe_unused]] type_t<U> type) && {
    return std::move(data_).template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](
    [[maybe_unused]] type_t<U> type) const&& {
    return std::move(data_).template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::get() & {
    if (index() != I) { throw bad_variant_access(); }
    return data_.template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::get() const& {
    if (index() != I) { throw bad_variant_access(); }
    return data_.template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::get() && {
    if (index() != I) { throw bad_variant_access(); }
    return std::move(data_).template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::get() const&& {
    if (index() != I) { throw bad_variant_access(); }
    return std::move(data_).template get<I>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::get() & {
    if (index() == detail::index_of_v<U, T...>) { throw bad_variant_access(); }
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::get() const& {
    if (index() == detail::index_of_v<U, T...>) { throw bad_variant_access(); }
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::get() && {
    if (index() == detail::index_of_v<U, T...>) { throw bad_variant_access(); }
    return std::move(data_).template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::get() const&& {
    if (index() == detail::index_of_v<U, T...>) { throw bad_variant_access(); }
    return std::move(data_).template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
constexpr bool variant<T...>::holds_alternative() const noexcept {
    if constexpr (detail::is_unique_v<U, T...>) {
        return index() == detail::index_of_v<U, T...>;
    } else {
        return holds_alt_impl<0, U>();
    }
}

template <typename... T>
template <typename V>
constexpr decltype(auto) variant<T...>::visit(V&& visitor) & {
    return visit_impl(std::forward<V>(visitor), *this);
}

template <typename... T>
template <typename V>
constexpr decltype(auto) variant<T...>::visit(V&& visitor) const& {
    return visit_impl(std::forward<V>(visitor), *this);
}

template <typename... T>
template <typename V>
constexpr decltype(auto) variant<T...>::visit(V&& visitor) && {
    return visit_impl(std::forward<V>(visitor), std::move(*this));
}

template <typename... T>
template <typename V>
constexpr decltype(auto) variant<T...>::visit(V&& visitor) const&& {
    return visit_impl(std::forward<V>(visitor), std::move(*this));
}

template <typename... T>
constexpr void variant<T...>::swap(variant& other) noexcept(
    noexcept(data_.swap(other.data_))) {
    data_.swap(other.data_);
}

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
    return std::forward<T0>(var0).visit(
        [visitor = std::forward<V>(visitor),
         ... varn = std::forward<TN>(varn)](auto&& value) -> decltype(auto) {
            return visit(
                [visitor = std::forward<V>(visitor),
                 value = std::forward<decltype(value)>(value)](
                    auto&&... args) -> decltype(auto) {
                    return std::invoke(std::forward<V>(visitor),
                                       std::forward<decltype(value)>(value),
                                       std::forward<decltype(args)>(args)...);
                },
                std::forward<TN>(varn)...);
        });
}

} // namespace sumty

#endif
