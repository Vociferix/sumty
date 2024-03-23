/* Copyright 2024 Jack A Bernard Jr.
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

#ifndef SUMTY_ERROR_SET_HPP
#define SUMTY_ERROR_SET_HPP

#include "sumty/detail/fwd.hpp"    // IWYU pragma: export
#include "sumty/detail/traits.hpp" // IWYU pragma: export
#include "sumty/detail/utils.hpp"
#include "sumty/utils.hpp"
#include "sumty/variant.hpp"

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace sumty {

template <typename... T>
class error_set {
  private:
    static_assert(detail::all_unique_v<T...>,
                  "All error types in an error_set must be unqiue");

    SUMTY_NO_UNIQ_ADDR variant<T...> set_;

    template <typename...>
    friend class error_set;

  public:
    constexpr error_set()
#ifndef DOXYGEN
        noexcept(std::is_nothrow_default_constructible_v<variant<T...>>)
        requires(std::is_default_constructible_v<variant<T...>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr error_set(const error_set&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_copy_constructible_v<variant<T...>>)
        requires(std::is_copy_constructible_v<variant<T...>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr error_set(error_set&&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_move_constructible_v<variant<T...>>)
        requires(std::is_move_constructible_v<variant<T...>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <size_t IDX, typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_set(std::in_place_index_t<IDX> inplace, Args&&... args)
        : set_(inplace, std::forward<Args>(args)...) {
    }

    template <size_t IDX, typename U, typename... Args>
    constexpr error_set(std::in_place_index_t<IDX> inplace,
                        std::initializer_list<U> init,
                        Args&&... args)
        : set_(inplace, init, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_set([[maybe_unused]] std::in_place_type_t<U> inplace,
                            Args&&... args)
        : set_(inplace, std::forward<Args>(args)...) {
    }

    template <typename U, typename V, typename... Args>
    constexpr error_set([[maybe_unused]] std::in_place_type_t<U> inplace,
                        std::initializer_list<V> init,
                        Args&&... args)
        : set_(inplace, init, std::forward<Args>(args)...) {}

    template <typename U>
#ifndef DOXYGEN
        requires(!detail::is_error_set_v<std::remove_cvref_t<U>> &&
                 detail::is_uniquely_constructible_v<U, T...>)
    explicit(detail::is_uniquely_explicitly_constructible_v<U, T...>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_set(U&& value)
        : set_(std::forward<U>(value)) {
    }

    template <typename U>
#ifndef DOXYGEN
        requires(detail::is_uniquely_constructible_v<std::initializer_list<U>, T...>)
    explicit(detail::is_uniquely_explicitly_constructible_v<std::initializer_list<U>, T...>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        constexpr error_set(std::initializer_list<U> init)
        : set_(init) {
    }

    template <typename... U>
#ifndef DOXYGEN
        requires(!std::is_same_v<error_set<U...>, error_set<T...>> &&
                 detail::is_subset_of_impl_v<error_set<U...>, error_set<T...>>)
    explicit(detail::is_uniquely_explicitly_constructible_v<U, T...>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_set(const error_set<U...>& other)
        : set_(detail::uninit) {
        other.set_.visit_informed([this](auto&& value, auto info) {
            set_.template uninit_emplace<detail::index_of_v<typename decltype(info)::type, T...>>(value);
        });
    }

    template <typename... U>
#ifndef DOXYGEN
        requires(!std::is_same_v<error_set<U...>, error_set<T...>> &&
                 detail::is_subset_of_impl_v<error_set<U...>, error_set<T...>>)
#endif
    // clang-format off
    // NOLINTNEXTLINE(hicpp-explicit-conversions,cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr error_set(error_set<U...>&& other) : set_(detail::uninit) {
        // clang-format on
        std::move(other.set_).visit_informed([this](auto&& value, auto info) {
            set_.template uninit_emplace<detail::index_of_v<typename decltype(info)::type, T...>>(info.forward(value));
        });
    }

    constexpr ~error_set()
#ifndef DOXYGEN
        noexcept(std::is_nothrow_destructible_v<variant<T...>>) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr error_set& operator=(const error_set&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_copy_assignable_v<variant<T...>>)
        requires(std::is_copy_assignable_v<variant<T...>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr error_set& operator=(error_set&&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_move_assignable_v<variant<T...>>)
        requires(std::is_move_assignable_v<variant<T...>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename U>
#ifndef DOXYGEN
        requires(!detail::is_error_set_v<std::remove_cvref_t<U>> &&
                 detail::is_uniquely_assignable_v<U, T...>)
#endif
    constexpr error_set& operator=(U&& rhs) {
        set_ = std::forward<U>(rhs);
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires(detail::is_uniquely_assignable_v<std::initializer_list<U>, T...>)
#endif
    constexpr error_set& operator=(std::initializer_list<U> rhs) {
        set_ = rhs;
        return *this;
    }

    template <typename... U>
#ifndef DOXYGEN
        requires(!std::is_same_v<error_set<U...>, error_set<T...>> &&
                 detail::is_subset_of_impl_v<error_set<U...>, error_set<T...>>)
#endif
    constexpr error_set& operator=(const error_set<U...>& rhs) {
        rhs.set_.visit_informed([this](auto&& value, auto info) {
            set_.template emplace<detail::index_of_v<typename decltype(info)::type, T...>>(value);
        });
        return *this;
    }

    template <typename... U>
#ifndef DOXYGEN
        requires(!std::is_same_v<error_set<U...>, error_set<T...>> &&
                 detail::is_subset_of_impl_v<error_set<U...>, error_set<T...>>)
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr error_set& operator=(error_set<U...>&& rhs) {
        std::move(rhs.set_).visit_informed([this](auto&& value, auto info) {
            set_.template emplace<detail::index_of_v<typename decltype(info)::type, T...>>(info.forward(value));
        });
        return *this;
    }

    [[nodiscard]] constexpr size_t index() const noexcept { return set_.index(); }

    template <size_t I, typename... Args>
    constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::reference
#else
        REFERENCE
#endif
        emplace(Args&&... args) {
        return set_.template emplace<I>(std::forward<Args>(args)...);
    }

    template <size_t I, typename U, typename... Args>
    constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::reference
#else
        REFERENCE
#endif
        emplace(std::initializer_list<U> ilist, Args&&... args) {
        return set_.template emplace<I>(ilist, std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::reference
#else
        REFERENCE
#endif
        emplace(Args&&... args) {
        return set_.template emplace<U>(std::forward<Args>(args)...);
    }

    template <typename U, typename V, typename... Args>
    constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::reference
#else
        REFERENCE
#endif
        emplace(std::initializer_list<V> ilist, Args&&... args) {
        return set_.template emplace<U>(ilist, std::forward<Args>(args)...);
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::reference
#else
        REFERENCE
#endif
        operator[]([[maybe_unused]] index_t<I> index) & noexcept {
        return set_[index];
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::const_reference
#else
        CONST_REFERENCE
#endif
        operator[]([[maybe_unused]] index_t<I> index) const& noexcept {
        return set_[index];
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
#else
        RVALUE_REFERENCE
#endif
        operator[]([[maybe_unused]] index_t<I> index) && {
        return std::move(set_)[index];
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
#else
        CONST_RVALUE_REFERENCE
#endif
        operator[]([[maybe_unused]] index_t<I> index) const&& {
        return std::move(set_)[index];
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::reference
#else
        REFERENCE
#endif
        operator[]([[maybe_unused]] type_t<U> type) & noexcept {
        return set_[type];
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::const_reference
#else
        CONST_REFERENCE
#endif
        operator[]([[maybe_unused]] type_t<U> type) const& noexcept {
        return set_[type];
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::rvalue_reference
#else
        RVALUE_REFERENCE
#endif
        operator[]([[maybe_unused]] type_t<U> type) && {
        return std::move(set_)[type];
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::const_rvalue_reference
#else
        CONST_RVALUE_REFERENCE
#endif
        operator[]([[maybe_unused]] type_t<U> type) const&& {
        return std::move(set_)[type];
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::reference
#else
        REFERENCE
#endif
        get() & {
        return set_.template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::const_reference
#else
        CONST_REFERENCE
#endif
        get() const& {
        return set_.template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
#else
        RVALUE_REFERENCE
#endif
        get() && {
        return std::move(set_).template get<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
#else
        CONST_RVALUE_REFERENCE
#endif
        get() const&& {
        return std::move(set_).template get<I>();
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::reference
#else
        REFERENCE
#endif
        get() & {
        return set_.template get<U>();
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::const_reference
#else
        CONST_REFERENCE
#endif
        get() const& {
        return set_.template get<U>();
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::rvalue_reference
#else
        RVALUE_REFERENCE
#endif
        get() && {
        return std::move(set_).template get<U>();
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::const_rvalue_reference
#else
        CONST_RVALUE_REFERENCE
#endif
        get() const&& {
        return std::move(set_).template get<U>();
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::pointer
#else
        POINTER
#endif
        get_if() noexcept {
        return set_.template get_if<I>();
    }

    template <size_t I>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<detail::select_t<I, T...>>::const_pointer
#else
        CONST_POINTER
#endif
        get_if() const noexcept {
        return set_.template get_if<I>();
    }

    template <typename U>
#ifndef DOXYGEN
        requires detail::is_unique_v<U, T...>
#endif
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::pointer
#else
        POINTER
#endif
        get_if() noexcept {
        return set_.template get_if<U>();
    }

    template <typename U>
    [[nodiscard]] constexpr
#ifndef DOXYGEN
        typename detail::traits<U>::const_pointer
#else
        CONST_POINTER
#endif
        get_if() const noexcept {
        return set_.template get_if<U>();
    }

    template <typename U>
    [[nodiscard]] constexpr bool holds_alternative() const noexcept {
        return set_.template holds_alternative<U>();
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::reference>
#else
        DEDUCED
#endif
        visit(V&& visitor) & {
        return set_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::const_reference>
#else
        DEDUCED
#endif
        visit(V&& visitor) const& {
        return set_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::rvalue_reference>
#else
        DEDUCED
#endif
        visit(V&& visitor) && {
        return std::move(set_).visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::const_rvalue_reference>
#else
        DEDUCED
#endif
        visit(V&& visitor) const&& {
        return std::move(set_).visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::reference,
            detail::alternative_info<0, variant<T...>>>
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) & {
        return set_.visit_informed(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::const_reference,
            detail::alternative_info<0, variant<T...>>>
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) const& {
        return set_.visit_informed(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::rvalue_reference,
            detail::alternative_info<0, variant<T...>>>
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) && {
        return std::move(set_).visit_informed(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        detail::invoke_result_t<
            V&&,
            typename detail::traits<detail::select_t<0, T...>>::const_rvalue_reference,
            detail::alternative_info<0, variant<T...>>>
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) const&& {
        return std::move(set_).visit_informed(std::forward<V>(visitor));
    }

    constexpr void swap(error_set& other)
#ifndef DOXYGEN
        noexcept(noexcept(set_.swap(other.set_)))
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        set_.swap(other.set_);
    }
};

template <typename T, typename... U>
constexpr bool holds_alternative(const error_set<U...>& v) noexcept {
    return v.template holds_alternative<T>();
}

template <size_t I, typename... T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<I, T...>>::reference
#else
    REFERENCE
#endif
    get(error_set<T...>& e) {
    return e.template get<I>();
}

template <size_t I, typename... T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<I, T...>>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const error_set<T...>& e) {
    return e.template get<I>();
}

template <size_t I, typename... T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    get(error_set<T...>&& e) {
    return std::move(e).template get<I>();
}

template <size_t I, typename... T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const error_set<T...>&& e) {
    return std::move(e).template get<I>();
}

template <typename T, typename... U>
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::reference
#else
    REFERENCE
#endif
    get(error_set<U...>& e) {
    return e.template get<T>();
}

template <typename T, typename... U>
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const error_set<U...>& e) {
    return e.template get<T>();
}

template <typename T, typename... U>
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    get(error_set<U...>&& e) {
    return std::move(e).template get<T>();
}

template <typename T, typename... U>
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const error_set<U...>&& e) {
    return std::move(e).template get<T>();
}

template <size_t I, typename... T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<I, T...>>::pointer
#else
    POINTER
#endif
    get_if(error_set<T...>& e) noexcept {
    return e.template get_if<I>();
}

template <size_t I, typename... T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<I, T...>>::const_pointer
#else
    CONST_POINTER
#endif
    get_if(const error_set<T...>& e) noexcept {
    return e.template get_if<I>();
}

template <typename T, typename... U>
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::pointer
#else
    POINTER
#endif
    get_if(error_set<U...>& e) noexcept {
    return e.template get_if<T>();
}

template <typename T, typename... U>
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::const_pointer
#else
    CONST_POINTER
#endif
    get_if(const error_set<U...>& e) noexcept {
    return e.template get_if<T>();
}

template <typename... T>
constexpr void swap(error_set<T...>& a, error_set<T...>& b)
#ifndef DOXYGEN
    noexcept(noexcept(a.swap(b)))
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

namespace detail {

template <typename T>
struct error_set_size_helper;

template <typename... T>
struct error_set_size_helper<error_set<T...>>
    : std::integral_constant<size_t, sizeof...(T)> {};

template <typename... T>
struct error_set_size_helper<const error_set<T...>>
    : std::integral_constant<size_t, sizeof...(T)> {};

template <size_t I, typename T>
struct error_set_alternative_helper;

template <size_t I, typename... T>
struct error_set_alternative_helper<I, error_set<T...>> {
    using type = detail::select_t<I, T...>;
};

template <size_t I, typename... T>
struct error_set_alternative_helper<I, const error_set<T...>>
    : error_set_alternative_helper<I, error_set<T...>> {};

template <typename T, typename U>
struct merge_error_set_helper;

template <typename... T, typename U>
    requires(all_unique_v<T..., U>)
struct merge_error_set_helper<error_set<T...>, error_set<U>> {
    using type = error_set<T..., U>;
};

template <typename... T, typename U>
    requires(!all_unique_v<T..., U>)
struct merge_error_set_helper<error_set<T...>, error_set<U>> {
    using type = error_set<T...>;
};

template <typename... T, typename U0, typename... UN>
    requires(all_unique_v<T..., U0>)
struct merge_error_set_helper<error_set<T...>, error_set<U0, UN...>>
    : merge_error_set_helper<error_set<T..., U0>, error_set<UN...>> {};

template <typename... T, typename U0, typename... UN>
    requires(!all_unique_v<T..., U0>)
struct merge_error_set_helper<error_set<T...>, error_set<U0, UN...>>
    : merge_error_set_helper<error_set<T...>, error_set<UN...>> {};

template <typename T, typename U>
    requires(!is_error_set_v<std::remove_cvref_t<T>> &&
             !is_error_set_v<std::remove_cvref_t<U>>)
struct merge_error_set_helper<T, U> : merge_error_set_helper<error_set<T>, error_set<U>> {};

template <typename T, typename U>
    requires(is_error_set_v<std::remove_cvref_t<T>> &&
             !is_error_set_v<std::remove_cvref_t<U>>)
struct merge_error_set_helper<T, U> : merge_error_set_helper<T, error_set<U>> {};

template <typename T, typename U>
    requires(!is_error_set_v<std::remove_cvref_t<T>> &&
             is_error_set_v<std::remove_cvref_t<U>>)
struct merge_error_set_helper<T, U> : merge_error_set_helper<error_set<T>, U> {};

template <typename... T>
struct make_error_set_helper;

template <typename... T>
struct make_error_set_helper<error_set<T...>> {
    using type = error_set<T...>;
};

template <typename T>
    requires(!is_error_set_v<T>)
struct make_error_set_helper<T> {
    using type = error_set<T>;
};

template <typename T0, typename T1, typename... TN>
struct make_error_set_helper<T0, T1, TN...>
    : make_error_set_helper<typename merge_error_set_helper<T0, T1>::type, TN...> {};

} // namespace detail

template <typename ES1, typename ES2>
struct is_subset_of : detail::is_subset_of_impl<ES1, ES2> {};

template <typename ES1, typename ES2>
static inline constexpr bool is_subset_of_v = is_subset_of<ES1, ES2>::value;

template <typename ES1, typename ES2>
struct is_equivalent : detail::is_equivalent_impl<ES1, ES2> {};

template <typename ES1, typename ES2>
static inline constexpr bool is_equivalent_v = is_equivalent<ES1, ES2>::value;

template <typename T>
struct error_set_size : detail::error_set_size_helper<T> {};

template <typename T>
static inline constexpr size_t error_set_size_v = error_set_size<T>::value;

template <typename... T>
struct variant_size<error_set<T...>> : error_set_size<error_set<T...>> {};

template <typename... T>
struct variant_size<const error_set<T...>> : error_set_size<const error_set<T...>> {};

template <size_t I, typename ES>
struct error_set_alternative : detail::error_set_alternative_helper<I, ES> {};

template <size_t I, typename ES>
using error_set_alternative_t = typename error_set_alternative<I, ES>::type;

template <size_t I, typename... T>
struct variant_alternative<I, error_set<T...>> : error_set_alternative<I, error_set<T...>> {
};

template <size_t I, typename... T>
struct variant_alternative<I, const error_set<T...>>
    : error_set_alternative<I, const error_set<T...>> {};

template <typename ES1, typename ES2>
struct merge_error_set : detail::merge_error_set_helper<ES1, ES2> {};

template <typename ES1, typename ES2>
using merge_error_set_t = typename merge_error_set<ES1, ES2>::type;

template <typename... T>
struct make_error_set : detail::make_error_set_helper<T...> {};

template <typename... T>
using make_error_set_t = typename make_error_set<T...>::type;

} // namespace sumty

#endif
