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

#include "sumty/detail/fwd.hpp"    // IWYU pragma: export
#include "sumty/detail/traits.hpp" // IWYU pragma: export
#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/result.hpp" // IWYU pragma: keep
#include "sumty/utils.hpp"  // IWYU pragma: export
#include "sumty/variant.hpp"

#include <compare>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

namespace sumty {

/// @class option option.hpp <sumty/option.hpp>
/// @brief Type that either contains some value or none
///
/// @details
/// @ref option is a reimplementation of `std::optional` with several
/// improvements. The key difference is that @ref option can contain `void`
/// and references (lvalue and rvalue). `option<void>` turns out to
/// essentially be a `bool` with the @ref option interface, and `option<T&>`
/// behaves like non-owning, nullable smart pointer to `T`.
///
/// Internally, `option<T>` is represented as a @ref variant<void, T>. Thus,
/// @ref option benefits from the size optimizations implemented by @ref
/// variant (see @ref variant documentation for details). In particular, an
/// @ref option of `void` or an empty type has the size of a `bool`, and an
/// @ref option of an lvalue reference has the size of a raw pointer.
///
/// ```cpp
/// struct empty_t {};
///
/// assert(sizeof(option<void>) == sizeof(bool));
///
/// assert(sizeof(option<empty_t>) == sizeof(bool));
///
/// assert(sizeof(option<int&>) == sizeof(int*));
///
/// assert(sizeof(option<const int&>) == sizeof(const int*));
/// ```
///
/// In practice, the benefit of @ref option over `std::optional` is that @ref
/// option can be used in more places, espeicially with generic code. A generic
/// function (function template) that wants to be able to return a value of any
/// type, but also allow that return value to be "null" or "none" can simply
/// return a `option<T>`, where `T` is now allowed to be a reference or even
/// `void`.
///
/// ```cpp
/// // Returns the result of invoking func, even if that result is void or a
/// // reference, if the condition is true. Otherwise returns none.
/// template <typename F>
/// option<std::invoke_result_t<F>> call_if(bool condition, F&& func) {
///     if (condition) {
///         // We have to specialize for void becuase of language limitations
///         if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
///             std::invoke(std::forward<F>(func));
///             return some<void>();
///         } else {
///             return std::invoke(std::forward<F>(func));
///         }
///     } else {
///         return none;
///     }
/// }
/// ```
///
/// An @ref option of an lvalue reference can also be used as an alternative
/// to raw pointers is many cases. `option<T&>` communicates that the pointer
/// is allowed to be null and that the pointer is non-owning. Depending on a
/// project's C++ coding practices, `option<T&>` may be preferable to `T*`.
///
/// @tparam T The possibly contained type
template <typename T>
class option {
  private:
    variant<void, T> opt_{};

  public:
#ifndef DOXYGEN
    using value_type = typename detail::traits<T>::value_type;
    using reference = typename detail::traits<T>::reference;
    using const_reference = typename detail::traits<T>::const_reference;
    using rvalue_reference = typename detail::traits<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<T>::const_rvalue_reference;
    using pointer = typename detail::traits<T>::pointer;
    using const_pointer = typename detail::traits<T>::const_pointer;
#else
    using value_type = ...;
    using reference = ...;
    using const_reference = ...;
    using rvalue_reference = ...;
    using const_rvalue_reference = ...;
    using pointer = ...;
    using const_pointer = ...;
#endif

    /// @brief Default constructor
    ///
    /// @details
    /// Initializes the @ref option as none or null.
    ///
    /// ## Example:
    /// ```cpp
    /// option<int> opt;
    ///
    /// assert(!opt);
    ///
    /// assert(opt.has_value() == false);
    /// ```
    constexpr option() noexcept
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    /// @brief Copy constructor
    ///
    /// @details
    /// If the source @ref option is not none, the new @ref option is
    /// initialized with a value copy constructed from the value contained in
    /// the source @ref option.
    ///
    /// ## Example
    /// ```cpp
    /// option<int> opt1;
    /// option<int> opt2 = 42;
    ///
    /// option<int> opt1_copy{opt1};
    /// option<int> opt2_copy{opt2};
    ///
    /// assert(!opt1_copy);
    ///
    /// assert(opt2_copy);
    /// assert(*opt2_copy == 42);
    /// ```
    constexpr option(const option&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_copy_constructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Move constructor
    ///
    /// @details
    /// If the source @ref option is not none, the new @ref option is
    /// initialized with a value move constructed from the value contained in
    /// the source @ref option. The source @ref option will be none after the
    /// move.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1;
    /// option<int> opt2;
    ///
    /// option<int> opt1_moved{std::move(opt1)};
    /// option<int> opt2_moved{std::move(opt2)};
    ///
    /// assert(!opt1_copy);
    ///
    /// assert(opt2_copy);
    /// assert(*opt2_copy == 42);
    /// ```
    constexpr option(option&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_constructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr option([[maybe_unused]] none_t null) noexcept : option() {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr option([[maybe_unused]] std::nullopt_t null) noexcept : option() {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr option([[maybe_unused]] std::nullptr_t null) noexcept
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<T>)
#endif
        : option() {
    }

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<T> && std::is_convertible_v<U*, pointer>)
    explicit(!std::is_convertible_v<U*, pointer>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr option(U* ptr) noexcept {
        if (ptr != nullptr) { opt_.template emplace<1>(*ptr); }
    }

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<void, T>,
                                         std::in_place_index_t<1>,
                                         typename detail::traits<U>::const_reference>)
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr option(const option<U>& other)
        : option() {
        if (other.has_value()) { opt_.template emplace<1>(*other); }
    }

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<void, T>,
                                         std::in_place_index_t<1>,
                                         typename detail::traits<U>::rvalue_reference>)
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr option(option<U>&& other)
        : option() {
        if (other.has_value()) { opt_.template emplace<1>(*std::move(other)); }
    }

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr option([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : opt_(std::in_place_index<1>, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr option([[maybe_unused]] std::in_place_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : opt_(std::in_place_index<1>, init, std::forward<Args>(args)...) {}

    template <typename U>
#ifndef DOXYGEN
        requires(
            std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U &&> &&
            !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
            !std::is_same_v<std::remove_const_t<T>, bool> &&
            !detail::is_option_v<std::remove_cvref_t<U>>)
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr option(U&& value)
        : opt_(std::in_place_index<1>, std::forward<U>(value)) {
    }

    constexpr ~option()
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr option& operator=(const option&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_copy_assignable &&
                 detail::traits<T>::is_nothrow_copy_constructible &&
                 detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr option& operator=(option&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_assignable &&
                 detail::traits<T>::is_nothrow_move_constructible &&
                 detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr option& operator=([[maybe_unused]] none_t null)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_destructible)
#else
        CONDTIONALLY_NOEXCEPT
#endif
    {
        opt_.template emplace<0>();
        return *this;
    }

    constexpr option& operator=([[maybe_unused]] std::nullopt_t null)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_destructible)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        opt_.template emplace<0>();
        return *this;
    }

    constexpr option& operator=([[maybe_unused]] std::nullptr_t null) noexcept
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<T>)
#endif
    {
        opt_.template emplace<0>();
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires(
            !detail::is_option_v<std::remove_cvref_t<U>> &&
            std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U &&> &&
            detail::traits<T>::template is_assignable<U &&> &&
            (!std::is_scalar_v<value_type> || !std::is_same_v<T, std::decay_t<U>>))
#endif
    constexpr option& operator=(U&& value) {
        opt_.template emplace<1>(std::forward<U>(value));
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<value_type> &&
                 std::is_convertible_v<U*, pointer>)
#endif
    constexpr option& operator=(U* ptr) noexcept {
        if (ptr == nullptr) {
            opt_.template emplace<0>();
        } else {
            opt_.template emplace<1>(*ptr);
        }
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires(!detail::traits<T>::template is_constructible<option<U>&> &&
                 !detail::traits<T>::template is_constructible<const option<U>&> &&
                 !detail::traits<T>::template is_constructible<option<U> &&> &&
                 !detail::traits<T>::template is_constructible<const option<U> &&> &&
                 !detail::traits<T>::template is_convertible_from<option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<option<U> &&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U> &&> &&
                 !detail::traits<T>::template is_assignable<option<U>&> &&
                 !detail::traits<T>::template is_assignable<const option<U>&> &&
                 !detail::traits<T>::template is_assignable<option<U> &&> &&
                 !detail::traits<T>::template is_assignable<const option<U> &&> &&
                 (std::is_lvalue_reference_v<T> ||
                  detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::const_reference>) &&
                 detail::traits<T>::template is_assignable<
                     typename detail::traits<U>::const_reference>)
#endif
    constexpr option& operator=(const option<U>& value) {
        if (value.has_value()) {
            opt_.template emplace<1>(*value);
        } else {
            opt_.template emplace<0>();
        }
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires(!detail::traits<T>::template is_constructible<option<U>&> &&
                 !detail::traits<T>::template is_constructible<const option<U>&> &&
                 !detail::traits<T>::template is_constructible<option<U> &&> &&
                 !detail::traits<T>::template is_constructible<const option<U> &&> &&
                 !detail::traits<T>::template is_convertible_from<option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U>&> &&
                 !detail::traits<T>::template is_convertible_from<option<U> &&> &&
                 !detail::traits<T>::template is_convertible_from<const option<U> &&> &&
                 !detail::traits<T>::template is_assignable<option<U>&> &&
                 !detail::traits<T>::template is_assignable<const option<U>&> &&
                 !detail::traits<T>::template is_assignable<option<U> &&> &&
                 !detail::traits<T>::template is_assignable<const option<U> &&> &&
                 (std::is_lvalue_reference_v<T> ||
                  detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::rvalue_reference>) &&
                 detail::traits<T>::template is_assignable<
                     typename detail::traits<U>::rvalue_reference>)
#endif
    constexpr option& operator=(option<U>&& value) {
        if (value.has_value()) {
            opt_.template emplace<1>(*std::move(value));
        } else {
            opt_.template emplace<0>();
        }
        return *this;
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator bool() const noexcept { return opt_.index() != 0; }

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<T> &&
                 requires(pointer src, U* dst) { dst = static_cast<U*>(src); })
    explicit(!std::is_convertible_v<pointer, U*>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        constexpr
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        operator U*() const noexcept {
        if (opt_.index() == 0) {
            return nullptr;
        } else {
            return static_cast<U*>(&opt_[index<1>]);
        }
    }

    [[nodiscard]] constexpr bool has_value() const noexcept { return opt_.index() != 0; }

    [[nodiscard]] constexpr reference operator*() & noexcept { return opt_[index<1>]; }

    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return opt_[index<1>];
    }

    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(opt_)[index<1>];
    }

    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(opt_)[index<1>];
    }

    [[nodiscard]] constexpr pointer operator->() noexcept {
        return opt_.template get_if<1>();
    }

    [[nodiscard]] constexpr const_pointer operator->() const noexcept {
        return opt_.template get_if<1>();
    }

    [[nodiscard]] constexpr reference value() & {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return opt_[index<1>];
    }

    [[nodiscard]] constexpr const_reference value() const& {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return opt_[index<1>];
    }

    [[nodiscard]] constexpr rvalue_reference value() && {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return std::move(opt_)[index<1>];
    }

    [[nodiscard]] constexpr rvalue_reference value() const&& {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return std::move(opt_)[index<1>];
    }

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) const& {
        if (opt_.index() != 0) {
            return opt_[index<1>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) && {
        if (opt_.index() != 0) {
            return std::move(opt_)[index<1>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    [[nodiscard]] constexpr value_type value_or() const& {
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

    [[nodiscard]] constexpr value_type value_or() && {
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
    [[nodiscard]] constexpr value_type value_or_else(F&& f) const& {
        if (opt_.index() != 0) {
            return opt_[index<1>];
        } else {
            if constexpr (std::is_void_v<value_type>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return std::invoke(std::forward<F>(f));
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr value_type value_or_else(F&& f) && {
        if (opt_.index() != 0) {
            return std::move(opt_)[index<1>];
        } else {
            if constexpr (std::is_void_v<value_type>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return std::invoke(std::forward<F>(f));
            }
        }
    }

    template <typename E>
    [[nodiscard]] constexpr result<T, std::remove_cvref_t<E>> ok_or(E&& err) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return opt_[index<1>];
            }
        } else {
            return result<T, std::remove_cvref_t<E>>{in_place_error, std::forward<E>(err)};
        }
    }

    template <typename E>
    [[nodiscard]] constexpr result<T, std::remove_cvref_t<E>> ok_or(E&& err) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return std::move(opt_)[index<1>];
            }
        } else {
            return result<T, std::remove_cvref_t<E>>{in_place_error, std::forward<E>(err)};
        }
    }

    template <typename F>
    [[nodiscard]] constexpr result<T, std::invoke_result_t<F>> ok_or_else(F&& f) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return opt_[index<1>];
            }
        } else {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::invoke(std::forward<F>(f));
                return result<T, std::invoke_result_t<F>>(in_place_error);
            } else {
                return result<T, std::invoke_result_t<F>>(in_place_error,
                                                          std::invoke(std::forward<F>(f)));
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr result<T, std::invoke_result_t<F>> ok_or_else(F&& f) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return std::move(opt_)[index<1>];
            }
        } else {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::invoke(std::forward<F>(f));
                return result<T, std::invoke_result_t<F>>(in_place_error);
            } else {
                return result<T, std::invoke_result_t<F>>(in_place_error,
                                                          std::invoke(std::forward<F>(f)));
            }
        }
    }

    template <typename U>
    [[nodiscard]] constexpr result<std::remove_cvref_t<U>, T> error_or(U&& value) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<std::remove_cvref_t<U>, T>{in_place_error};
            } else {
                return result<std::remove_cvref_t<U>, T>{in_place_error, opt_[index<1>]};
            }
        } else {
            return result<std::remove_cvref_t<U>, T>{std::in_place, std::forward<U>(value)};
        }
    }

    template <typename U>
    [[nodiscard]] constexpr result<std::remove_cvref_t<U>, T> error_or(U&& value) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<std::remove_cvref_t<U>, T>{in_place_error};
            } else {
                return result<std::remove_cvref_t<U>, T>{in_place_error,
                                                         std::move(opt_)[index<1>]};
            }
        } else {
            return result<std::remove_cvref_t<U>, T>{std::in_place, std::forward<U>(value)};
        }
    }

    template <typename F>
    [[nodiscard]] constexpr result<std::invoke_result_t<F>, T> error_or_else(F&& f) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<std::invoke_result_t<F>, T>{in_place_error};
            } else {
                return result<std::invoke_result_t<F>, T>{in_place_error, opt_[index<1>]};
            }
        } else {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::invoke(std::forward<F>(f));
                return result<std::invoke_result_t<F>, T>{std::in_place};
            } else {
                return result<std::invoke_result_t<F>, T>{std::in_place,
                                                          std::invoke(std::forward<F>(f))};
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr result<std::invoke_result_t<F>, T> error_or_else(F&& f) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<std::invoke_result_t<F>, T>{in_place_error};
            } else {
                return result<std::invoke_result_t<F>, T>{in_place_error,
                                                          std::move(opt_)[index<1>]};
            }
        } else {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::invoke(std::forward<F>(f));
                return result<std::invoke_result_t<F>, T>{std::in_place};
            } else {
                return result<std::invoke_result_t<F>, T>{std::in_place,
                                                          std::invoke(std::forward<F>(f))};
            }
        }
    }

    [[nodiscard]] constexpr option<reference> ref() noexcept {
        if (opt_.index() != 0) {
            return option<reference>{std::in_place, opt_[index<1>]};
        } else {
            return option<reference>{};
        }
    }

    [[nodiscard]] constexpr option<const_reference> ref() const noexcept {
        if (opt_.index() != 0) {
            return option<const_reference>{std::in_place, opt_[index<1>]};
        } else {
            return option<const_reference>{};
        }
    }

    [[nodiscard]] constexpr option<const_reference> cref() const noexcept { return ref(); }

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
                return std::remove_cvref_t<
                    std::invoke_result_t<F, const_rvalue_reference>>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
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
                    return option<res_t>{std::in_place,
                                         std::invoke(std::forward<F>(f), opt_[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
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
                    return option<res_t>{std::in_place,
                                         std::invoke(std::forward<F>(f), opt_[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
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
                    return option<res_t>{
                        std::in_place,
                        std::invoke(std::forward<F>(f), std::move(opt_)[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
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
                    return option<res_t>{
                        std::in_place,
                        std::invoke(std::forward<F>(f), std::move(opt_)[index<1>])};
                }
            } else {
                return option<res_t>{};
            }
        }
    }

    template <typename F>
    constexpr auto map(F&& f) & {
        return transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map(F&& f) const& {
        return transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map(F&& f) && {
        return std::move(*this).transform(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map(F&& f) const&& {
        return std::move(*this).transform(std::forward<F>(f));
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

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) & {
        return opt_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) const& {
        return opt_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) && {
        return std::move(opt_).visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) const&& {
        return std::move(opt_).visit(std::forward<V>(visitor));
    }

    constexpr void swap(option& other)
#ifndef DOXYGEN
        noexcept(noexcept(opt_.swap(other.opt_)))
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        opt_.swap(other.opt_);
    }

    constexpr void reset() noexcept { opt_.template emplace<0>(); }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        opt_.template emplace<1>(std::forward<Args>(args)...);
        return opt_[index<1>];
    }
};

/// @relates option
template <size_t IDX, typename T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, void, T>>::reference
#else
    REFERENCE
#endif
    get(option<T>& opt) {
    if constexpr (IDX == 0) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(IDX == 1, "Invalid get index for sumty::option");
        return opt.value();
    }
}

/// @relates option
template <size_t IDX, typename T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, void, T>>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const option<T>& opt) {
    if constexpr (IDX == 0) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(IDX == 1, "Invalid get index for sumty::option");
        return opt.value();
    }
}

/// @relates option
template <size_t IDX, typename T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, void, T>>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    get(option<T>&& opt) {
    if constexpr (IDX == 0) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(IDX == 1, "Invalid get index for sumty::option");
        return std::move(opt).value();
    }
}

/// @relates option
template <size_t IDX, typename T>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, void, T>>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const option<T>&& opt) {
    if constexpr (IDX == 0) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(IDX == 1, "Invalid get index for sumty::option");
        return std::move(opt).value();
    }
}

/// @relates option
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<U>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::reference
#else
    REFERENCE
#endif
    get(option<U>& opt) {
    if constexpr (std::is_void_v<T>) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(std::is_same_v<T, U>, "Invalid get type for sumty::option");
        return opt.value();
    }
}

/// @relates option
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<U>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const option<U>& opt) {
    if constexpr (std::is_void_v<T>) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(std::is_same_v<T, U>, "Invalid get type for sumty::option");
        return opt.value();
    }
}

/// @relates option
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<U>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    get(option<U>&& opt) {
    if constexpr (std::is_void_v<T>) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(std::is_same_v<T, U>, "Invalid get type for sumty::option");
        return std::move(opt).value();
    }
}

/// @relates option
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<U>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<T>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const option<U>&& opt) {
    if constexpr (std::is_void_v<T>) {
        if (opt.has_value()) { throw bad_option_access(); }
    } else {
        static_assert(std::is_same_v<T, U>, "Invalid get type for sumty::option");
        return std::move(opt).value();
    }
}

/// @relates option
template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value()) {
        return rhs.has_value() && *lhs == *rhs;
    } else {
        return !rhs.has_value();
    }
}

/// @relates option
template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value()) {
        return !rhs.has_value() || *lhs != *rhs;
    } else {
        return rhs.has_value();
    }
}

/// @relates option
template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const option<U>& rhs) {
    return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

/// @relates option
template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const option<U>& rhs) {
    return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
}

/// @relates option
template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const option<U>& rhs) {
    return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
}

/// @relates option
template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const option<U>& rhs) {
    return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
}

/// @relates option
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::three_way_comparable_with<std::remove_cvref_t<U>, std::remove_cvref_t<T>>)
#endif
constexpr
#ifndef DOXYGEN
    std::compare_three_way_result_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>
#else
    auto
#endif
    operator<=>(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value() && rhs.has_value()) {
        return *lhs <=> *rhs;
    } else {
        return lhs.has_value() <=> rhs.has_value();
    }
}

/// @relates option
template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator==(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs == *rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs != rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator!=(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs != *rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs < rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator<(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs < *rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() && *lhs > rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator>(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs > *lhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs <= rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator<=(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs <= *rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs >= rhs;
}

/// @relates option
template <typename T, typename U>
constexpr bool operator>=(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs >= *rhs;
}

/// @relates option
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!detail::is_option_v<U>)
#endif
constexpr
#ifndef DOXYGEN
    std::compare_three_way_result_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>
#else
    auto
#endif
    operator<=>(const option<T>& lhs, const U& rhs)
#ifndef DOXYGEN
    requires(std::three_way_comparable_with<std::remove_cvref_t<U>, std::remove_cvref_t<T>>)
#endif
{
    if (lhs.has_value()) {
        return *lhs <=> rhs;
    } else {
        return std::strong_ordering::less;
    }
}

/// @relates option
template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return !lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator==([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator!=([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] none_t rhs) {
    return false;
}

/// @relates option
template <typename T>
constexpr bool operator<([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator>([[maybe_unused]] none_t lhs,
                         [[maybe_unused]] const option<T>& rhs) {
    return false;
}

/// @relates option
template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return !lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator<=([[maybe_unused]] none_t lhs,
                          [[maybe_unused]] const option<T>& rhs) {
    return true;
}

/// @relates option
template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] none_t rhs) {
    return true;
}

/// @relates option
template <typename T>
constexpr bool operator>=([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] none_t rhs) {
    return lhs.has_value() <=> false;
}

/// @relates option
template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return !lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator==([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator!=([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] std::nullopt_t rhs) {
    return false;
}

/// @relates option
template <typename T>
constexpr bool operator<([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator>([[maybe_unused]] std::nullopt_t lhs,
                         [[maybe_unused]] const option<T>& rhs) {
    return false;
}

/// @relates option
template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return !lhs.has_value();
}

/// @relates option
template <typename T>
constexpr bool operator<=([[maybe_unused]] std::nullopt_t lhs,
                          [[maybe_unused]] const option<T>& rhs) {
    return true;
}

/// @relates option
template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] std::nullopt_t rhs) {
    return true;
}

/// @relates option
template <typename T>
constexpr bool operator>=([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value() <=> false;
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return !lhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator==([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator!=([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] std::nullptr_t rhs) {
    return false;
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>([[maybe_unused]] std::nullptr_t lhs,
                         [[maybe_unused]] const option<T>& rhs) {
    return false;
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return !lhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<=([[maybe_unused]] std::nullptr_t lhs,
                          [[maybe_unused]] const option<T>& rhs) {
    return true;
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] std::nullptr_t rhs) {
    return true;
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>=([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value() <=> false;
}

/// @relates option
template <typename T, typename... Args>
constexpr option<T> some(Args&&... args) {
    return option<T>{std::in_place, std::forward<Args>(args)...};
}

/// @relates option
template <typename T, typename U, typename... Args>
constexpr option<T> some(std::initializer_list<U> ilist, Args&&... args) {
    return option<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

/// @relates option
template <typename T>
constexpr void swap(option<T>& a, option<T>& b)
#ifndef DOXYGEN
    noexcept(noexcept(a.swap(b)))
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

} // namespace sumty

#endif
