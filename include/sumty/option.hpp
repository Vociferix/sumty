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

    /// @brief None constructor
    ///
    /// @details
    /// Initializes the @ref option to the `none` value. That is, the @ref
    /// option will contain no value, and `.has_value()` will return `false`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{none};
    ///
    /// assert(!opt);
    /// assert(opt.has_value() == false);
    /// assert(opt == none);
    /// ```
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr option([[maybe_unused]] none_t null) noexcept : option() {}

    /// @brief `std::nullopt` constructor
    ///
    /// @details
    /// Initializes the @ref option to the `none` value. That is, the @ref
    /// option will contain no value, and `.has_value()` will return `false`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{std::nullopt};
    ///
    /// assert(!opt);
    /// assert(opt.has_value() == false);
    /// assert(opt == std::nullopt);
    /// ```
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr option([[maybe_unused]] std::nullopt_t null) noexcept : option() {}

    /// @brief `nullptr` constructor
    ///
    /// @details
    /// Initializes the @ref option to the `none` value. That is, the @ref
    /// option will contain no value, and `.has_value()` will return `false`.
    ///
    /// This constructor only participates in overload resolution if the
    /// contained value type is an lvalue reference, in which case the @ref
    /// option behaves like a smart pointer.
    ///
    /// ## Example
    /// ```
    /// option<int&> opt{nullptr};
    ///
    /// assert(!opt);
    /// assert(opt.has_value() == false);
    /// assert(opt == nullptr);
    /// ```
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr option([[maybe_unused]] std::nullptr_t null) noexcept
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<T>)
#endif
        : option() {
    }

    /// @brief Pointer constructor
    ///
    /// @details
    /// Initializes the @ref option to contain a reference to the value that
    /// the pointer points to. If the pointer is null, the @ref option will
    /// be `none`. That is, the @ref option will contain no value, and
    /// `.has_value()` will return `false`.
    ///
    /// This constructor only participates in overload resolution if the
    /// contained value type is an lvalue reference, in which case the @ref
    /// option behaves like a smart pointer.
    ///
    /// This constructor is `explicit` if the pointer is not implicitly
    /// convertible to `T*`.
    ///
    /// ## Example
    /// ```
    /// int value = 42;
    /// int* ptr = &value;
    /// int* null_ptr = nullptr;
    ///
    /// option<int&> opt1{ptr};
    /// option<int&> opt2{null_ptr};
    ///
    /// assert(opt1);
    /// assert(opt1 == ptr);
    ///
    /// assert(!opt2);
    /// assert(opt2 == null_ptr);
    /// ```
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

    /// @brief Converting copy constructor
    ///
    /// @details
    /// Initializes the @ref option to contain the value of the source @ref
    /// option where the contained value `U` is converted to `T`, as if by
    /// `T{U_value}`. If the source @ref option is `none`, then the destination
    /// @ref option will also be `none`.
    ///
    /// This constructor only participates in overload resolution if the
    /// value type `T` of the destination @ref option is constructible from
    /// the contained value of the source @ref option `U`.
    ///
    /// This constructor is `explicit` if `U` is not implicitly convertible to
    /// `T`.
    ///
    /// Note that `void` is considered to be convertible to any type that is
    /// default constructible.
    ///
    /// ## Example
    /// ```
    /// option<float> opt1{3.14};
    ///
    /// option<int> opt2{opt1};
    ///
    /// assert(opt2);
    ///
    /// assert(*opt2 == 3);
    /// ```
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

    /// @brief Converting move constructor
    ///
    /// @details
    /// Initializes the @ref option to contain the value of the source @ref
    /// option where the contained value `U` is moved and converted to `T`, as
    /// if by `T{std::move(U_value)}`. If the source @ref option is `none`,
    /// the the destination @ref option will also be `none`.
    ///
    /// This constructor only participates in overload resolution if the
    /// value type `T` of the destination @ref option is convertible from
    /// the contained value of the source @ref option `U`.
    ///
    /// This constructor is `explicit` if `U` is not implicitly convertible to
    /// `T`.
    ///
    /// Note that `void` is considered to be convertible to any type that is
    /// default constructible.
    ///
    /// ## Example
    /// ```
    /// option<float> opt1{3.14};
    ///
    /// option<int> opt2{std::move(opt1)};
    ///
    /// assert(opt2);
    ///
    /// assert(*opt2 == 3);
    /// ```
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

    /// @brief Emplacement constructor
    ///
    /// @details
    /// The @ref option is initialized such that it contains a value that is
    /// constructed in place from the forwarded arguments.
    ///
    /// This constructor is `explicit` if `inplace` is the only argument.
    ///
    /// ## Example
    /// ```
    /// option<std::string> opt{std::in_place, 5, 'a'};
    ///
    /// assert(opt);
    ///
    /// assert(*opt == "aaaaa");
    /// ```
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

    /// @brief Emplacement constructor with initializer list
    ///
    /// @details
    /// The @ref option is initialized such that it contains a value that is
    /// constructed in place from the forwarded arguments.
    ///
    /// ## Example
    /// ```
    /// option<std::vector<int>> opt{std::in_place, {1, 2, 3, 4, 5}};
    ///
    /// assert(opt);
    ///
    /// assert(opt->size() == 5);
    /// ```
    template <typename U, typename... Args>
    constexpr option([[maybe_unused]] std::in_place_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : opt_(std::in_place_index<1>, init, std::forward<Args>(args)...) {}

    /// @brief Forwarding constructor
    ///
    /// @details
    /// The @ref option is initialized such that it contains a value that is
    /// constructed in place with the passed value forwarded to the value
    /// type's constructor.
    ///
    /// This constructor only participates in overload resolution if the
    /// contained value type is constructible from the passed value, the passed
    /// value is not of type `std::in_place_t`, and the contained value type is
    /// not `bool`.
    ///
    /// This constructor is `explicit` if the passed value is not implicitly
    /// convertible to `T`.
    ///
    /// ## Example
    /// ```
    /// float value = 3.14;
    ///
    /// option<int> opt{value};
    ///
    /// assert(opt);
    ///
    /// assert(*opt == 3);
    /// ```
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

    /// @brief Destructor
    ///
    /// @details
    /// If the @ref option is not `none`, the contained value is destroy in
    /// place.
    ///
    /// The destructor is `noexcept` if the contained value type is nothrow
    /// destructible.
    constexpr ~option()
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Copy assignment operator
    ///
    /// @details
    /// The destination @ref option is reassigned such that it contains the
    /// value of the source @ref option, if it has any.
    ///
    /// If both the source and desitnation @ref option are not `none`, then the
    /// value contained in the destination @ref option is copy assigned from
    /// the value contained in the source @ref option.
    ///
    /// If the source @ref option contains a value and the destination is
    /// `none`, the value contained in the source @ref option is destroyed in
    /// place, making the destination @ref option `none`.
    ///
    /// If the source @ref option is `none` and the destination contains a
    /// value, a new value is copy constructed in place in the source @ref
    /// option.
    ///
    /// If both the source and destination @ref option are `none`, both remain
    /// `none`.
    ///
    /// This function is `noexcept` if the contained value type is nothrow
    /// copy assignable, nothrow copy constructible, and nothrow destructible.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{42};
    /// option<int> opt2{};
    ///
    /// opt2 = opt1;
    ///
    /// assert(opt2);
    /// assert(*opt2 == 42);
    ///
    /// opt1 = none;
    /// opt2 = opt1;
    ///
    /// assert(!opt2);
    /// ```
    constexpr option& operator=(const option&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_copy_assignable &&
                 detail::traits<T>::is_nothrow_copy_constructible &&
                 detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Move assignment operator
    ///
    /// @details
    /// The destination @ref option is reassigned such that it contains the
    /// moved value of the source @ref option, if it has any. In all cases,
    /// the source @ref option will be `none` after the move.
    ///
    /// If both the source and destination @ref option are not `none`, then the
    /// value contained in the destination @ref option is move assigned from
    /// the moved value from the source @ref option. The old value in the
    /// source @ref option is destroyed in place.
    ///
    /// If the source @ref option contains a value and the desitnation is
    /// `none`, the value contained in the source @ref option is destroyed in
    /// place, making the desitnation @ref option `none`.
    ///
    /// If the source @ref option is `none` and the destination contains a
    /// value, a new value is move constructed in place in the source @ref
    /// option. The old value in the source @ref option is destroyed in place.
    ///
    /// If both the source and destination @ref option are `none`, both remain
    /// `none`.
    ///
    /// This function is `noexcept` if the contained value type is nothrow
    /// move assignable, nothrow move constructible, and nothrow destructible.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{42};
    /// option<int> opt2{};
    ///
    /// opt2 = std::move(opt1);
    ///
    /// assert(!opt1);
    /// assert(opt2);
    /// assert(*opt2 == 42);
    ///
    /// opt1 = std::move(opt2);
    ///
    /// assert(!opt2);
    /// assert(opt1);
    /// assert(*opt1 == 42);
    /// ```
    constexpr option& operator=(option&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_assignable &&
                 detail::traits<T>::is_nothrow_move_constructible &&
                 detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief `none` assignment operator
    ///
    /// @details
    /// If the @ref option contains a value, the value is destroyed in place.
    /// If the @ref option is already `none`, this function has no effect.
    ///
    /// This function is `noexcept` if the contained value type is nothrow
    /// destructible.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{42};
    ///
    /// opt = none;
    ///
    /// assert(!opt);
    /// ```
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

    /// @brief `std::nullopt` assignment operator
    ///
    /// @details
    /// If the @ref option contains a value, the value is destroyed in place.
    /// If the @ref option is already `none`, this function has no effect.
    ///
    /// This function is `noexcept` if the contained value type is nothrow
    /// destructible.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{42};
    ///
    /// opt = std::nullopt;
    ///
    /// assert(!opt);
    /// ```
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

    /// @brief `nullptr` assignment operator
    ///
    /// @details
    /// Sets an lvalue reference @ref option to `none`.
    ///
    /// This function only participates in overload resolution if the contained
    /// value type is an lvalue reference.
    ///
    /// ## Example
    /// ```
    /// int value = 42;
    /// option<int&> opt{value};
    ///
    /// opt = nullptr;
    ///
    /// assert(!opt);
    /// ```
    constexpr option& operator=([[maybe_unused]] std::nullptr_t null) noexcept
#ifndef DOXYGEN
        requires(std::is_lvalue_reference_v<T>)
#endif
    {
        opt_.template emplace<0>();
        return *this;
    }

    /// @brief Value assignment operator
    ///
    /// @details
    /// Sets the @ref option to contain the provided value, converted to `T`.
    /// If the @ref option holds a value (is not `none`) before the assigment,
    /// the value is directly assigned the forwarded source value. If the @ref
    /// option is `none`, a new value is constructed in place from the
    /// forwarded source value.
    ///
    /// This function only participates in overload resolution if:
    /// * The source value is not an @ref option
    /// * `T` is constructible from `U`
    /// * `T` is assignable from `U`
    /// * `T` is not scalar or `U` is not equivalent to `T`
    ///
    /// ## Example
    /// ```
    /// option<long long> opt{};
    ///
    /// opt = 42;
    ///
    /// assert(opt);
    ///
    /// assert(*opt == 42);
    /// ```
    template <typename U>
#ifndef DOXYGEN
        requires(
            !detail::is_option_v<std::remove_cvref_t<U>> &&
            std::is_constructible_v<variant<void, T>, std::in_place_index_t<1>, U &&> &&
            detail::traits<T>::template is_assignable<U &&> &&
            (!std::is_scalar_v<value_type> || !std::is_same_v<T, std::decay_t<U>>))
#endif
    constexpr option& operator=(U&& value) {
        if (opt_.index() == 1) {
            opt_[index<1>] = std::forward<U>(value);
        } else {
            opt_.template emplace<1>(std::forward<U>(value));
        }
        return *this;
    }

    /// @brief Pointer assignment operator
    ///
    /// @details
    /// The destination @ref option is assigned such that it contains an lvalue
    /// reference that the source pointer points to. If the pointer is null,
    /// the destination @ref option will be `none`.
    ///
    /// This function only participates in overload resolution if `T` is an
    /// lvalue reference and the pointer `U*` can be implicitly converted to
    /// `pointer`.
    ///
    /// ## Example
    /// ```
    /// int value = 42;
    /// int* ptr = &value;
    ///
    /// option<int&> opt{};
    ///
    /// opt = ptr;
    ///
    /// assert(opt)
    ///
    /// assert(opt == ptr);
    /// ```
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

    /// @brief Converting copy assignment operator
    ///
    /// @details
    /// This function performs copy assignment just like the standard copy
    /// assignment operator, but it also performs conversion from the value
    /// contained in the source @ref option to the value type of contained
    /// in the destination @ref option.
    ///
    /// This function only participates in overload resolution if `T` is not
    /// construcitble from, convertible from, or assignable from any cvref
    /// qualified form of `option<U>`, and `T` is both constructible and
    /// assignable from `U`.
    ///
    /// ## Example
    /// ```
    /// option<float> opt1{3.14};
    /// option<int> opt2{};
    ///
    /// opt2 = opt1;
    ///
    /// assert(opt2);
    ///
    /// assert(*opt2 == 3);
    /// ```
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

    /// @brief Converting move assignment operator
    ///
    /// @details
    /// This function performs move assignment just like the standard move
    /// assignment operator, but it also performs conversion from the value
    /// contained in the source @ref option to the value type of contained
    /// in the destination @ref option.
    ///
    /// This function only participates in overload resolution if `T` is not
    /// construcitble from, convertible from, or assignable from any cvref
    /// qualified form of `option<U>`, and `T` is both constructible and
    /// assignable from `U`.
    ///
    /// ## Example
    /// ```
    /// option<float> opt1{3.14};
    /// option<int> opt2{};
    ///
    /// opt2 = std::move(opt1);
    ///
    /// assert(opt2);
    ///
    /// assert(*opt2 == 3);
    /// ```
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

    /// @brief Implicit conversion to `bool`.
    ///
    /// @details
    /// This implicit conversion allows an @ref option to be used directly in
    /// a condition to check if the @ref option contains a value.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// if (opt1) {
    ///     assert(false);
    /// } else {
    ///     assert(true);
    /// }
    ///
    /// if (opt2) {
    ///     assert(true);
    /// } else {
    ///     assert(false);
    /// }
    /// ```
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator bool() const noexcept { return opt_.index() != 0; }

    /// @brief Conversion to raw pointer.
    ///
    /// @details
    /// This conversion allows an @ref option of an lvalue to be converted into
    /// a raw pointer, where the resulting pointer points to the referenced
    /// value or is null if the @ref option is none.
    ///
    /// This conversion only participates in overload resolution if `T` is an
    /// lvalue reference and `pointer` can be cast to `U*`.
    ///
    /// This conversion is explicit if `pointer` is not implicitly convertible
    /// to `U*`.
    ///
    /// ## Example
    /// ```
    /// int value = 42;
    ///
    /// option<int&> opt{value};
    ///
    /// int* ptr = opt;
    ///
    /// assert(ptr == &value);
    /// ```
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

    /// @brief Returns true if the @ref option contains a value.
    ///
    /// @details
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// assert(opt1.has_value() == false);
    ///
    /// assert(opt2.has_value() == true);
    /// ```
    [[nodiscard]] constexpr bool has_value() const noexcept { return opt_.index() != 0; }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This operator does not check if the @ref option contains a value. Use
    /// of this operator when the @ref option is `none` results in undefined
    /// behavior.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{42};
    ///
    /// assert(*opt == 42);
    /// ```
    [[nodiscard]] constexpr reference operator*() & noexcept { return opt_[index<1>]; }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This operator does not check if the @ref option contains a value. Use
    /// of this operator when the @ref option is `none` results in undefined
    /// behavior.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt{42};
    ///
    /// assert(*opt == 42);
    /// ```
    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return opt_[index<1>];
    }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This operator does not check if the @ref option contains a value. Use
    /// of this operator when the @ref option is `none` results in undefined
    /// behavior.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{42};
    ///
    /// assert(*std::move(opt) == 42);
    /// ```
    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(opt_)[index<1>];
    }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This operator does not check if the @ref option contains a value. Use
    /// of this operator when the @ref option is `none` results in undefined
    /// behavior.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt{42};
    ///
    /// assert(*std::move(opt) == 42);
    /// ```
    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(opt_)[index<1>];
    }

    /// @brief Accesses members of the value contained in the @ref option.
    ///
    /// @details
    /// This operator does not check if the @ref option contains a value. Use
    /// of this operator when the @ref option is `none` results in undefined
    /// behavior.
    ///
    /// ## Example
    /// ```
    /// option<std::string> opt{"hello"};
    ///
    /// assert(opt->size() == 5);
    /// ```
    [[nodiscard]] constexpr pointer operator->() noexcept {
        return opt_.template get_if<1>();
    }

    /// @brief Accesses members of the value contained in the @ref option.
    ///
    /// @details
    /// This operator does not check if the @ref option contains a value. Use
    /// of this operator when the @ref option is `none` results in undefined
    /// behavior.
    ///
    /// ## Example
    /// ```
    /// const option<std::string> opt{"hello"};
    ///
    /// assert(opt->size() == 5);
    /// ```
    [[nodiscard]] constexpr const_pointer operator->() const noexcept {
        return opt_.template get_if<1>();
    }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This function first checks if the @ref option contains a value before
    /// attempt to access the value. If the @ref option is `none`, then this
    /// function throws an exception.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{42};
    ///
    /// assert(opt.value() == 42);
    /// ```
    ///
    /// @throws bad_option_access Thrown if the @ref option is `none`.
    [[nodiscard]] constexpr reference value() & {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return opt_[index<1>];
    }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This function first checks if the @ref option contains a value before
    /// attempt to access the value. If the @ref option is `none`, then this
    /// function throws an exception.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt{42};
    ///
    /// assert(opt.value() == 42);
    /// ```
    ///
    /// @throws bad_option_access Thrown if the @ref option is `none`.
    [[nodiscard]] constexpr const_reference value() const& {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return opt_[index<1>];
    }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This function first checks if the @ref option contains a value before
    /// attempt to access the value. If the @ref option is `none`, then this
    /// function throws an exception.
    ///
    /// ## Example
    /// ```
    /// option<int> opt{42};
    ///
    /// assert(std::move(opt).value() == 42);
    /// ```
    ///
    /// @throws bad_option_access Thrown if the @ref option is `none`.
    [[nodiscard]] constexpr rvalue_reference value() && {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return std::move(opt_)[index<1>];
    }

    /// @brief Accesses the value contained in the @ref option.
    ///
    /// @details
    /// This function first checks if the @ref option contains a value before
    /// attempt to access the value. If the @ref option is `none`, then this
    /// function throws an exception.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt{42};
    ///
    /// assert(std::move(opt).value() == 42);
    /// ```
    ///
    /// @throws bad_option_access Thrown if the @ref option is `none`.
    [[nodiscard]] constexpr rvalue_reference value() const&& {
        if (opt_.index() == 0) { throw bad_option_access(); }
        return std::move(opt_)[index<1>];
    }

    /// @brief Gets the @ref option value with a default used for `none`.
    ///
    /// @details
    /// If the @ref option contains a value, a copy of the value is returned.
    /// If the @ref option is `none`, the provided default value is returned
    /// as `static_cast<T>(std::forward<U>(default_value))`.
    ///
    /// # Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// assert(opt1.value_or(0) == 0);
    ///
    /// assert(opt2.value_or(0) == 42);
    /// ```
    ///
    /// @param default_value The value to use if the @ref option is `none`.
    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) const& {
        if (opt_.index() != 0) {
            return opt_[index<1>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    /// @brief Gets the @ref option value with a default used for `none`.
    ///
    /// @details
    /// If the @ref option contains a value, the moved value is returned. If
    /// the @ref option is `none`, the provided default value is returned as
    /// `static_cast<T>(std::forward<U>(default_value))`.
    ///
    /// # Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// assert(std::move(opt1).value_or(0) == 0);
    ///
    /// assert(std::move(opt2).value_or(0) == 42);
    /// ```
    ///
    /// @param default_value The value to use if the @ref option is `none`.
    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) && {
        if (opt_.index() != 0) {
            return std::move(opt_)[index<1>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    /// @brief Gets the @ref option value with a default used for `none`.
    ///
    /// @details
    /// If the @ref option contains a value, the copied value is returned. If
    /// the @ref option is `none`, a default constructed value is returned
    /// instead.
    ///
    /// # Example
    /// ```
    /// option<std::string> opt1{};
    /// option<std::string> opt2{"hello"};
    ///
    /// assert(opt1.value_or() == std::string());
    ///
    /// assert(opt2.value_or() == "hello");
    /// ```
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

    /// @brief Gets the @ref option value with a default used for `none`.
    ///
    /// @details
    /// If the @ref option contains a value, the moved value is returned. If
    /// the @ref option is `none`, a default constructed value is returned
    /// instead.
    ///
    /// # Example
    /// ```
    /// option<std::string> opt1{};
    /// option<std::string> opt2{"hello"};
    ///
    /// assert(std::move(opt1).value_or() == std::string());
    ///
    /// assert(std::move(opt2).value_or() == "hello");
    /// ```
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

    /// @brief Gets the @ref option value with a default used for `none`.
    ///
    /// @details
    /// If the @ref option contains a value, the copied value is returned. If
    /// the @ref option is `none`, the result of invoking `f` is returned. The
    /// returned value of `f` is also explicitly cast to `value_type` before
    /// return.
    ///
    /// This function might be preferred over `.value_or(default_value)` when
    /// the default value is expensive to create or move. This function allows
    /// the default value to never be instantiated if the @ref option is not
    /// `none`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// assert(opt1.value_or_else([] { return 0; }) == 0);
    ///
    /// assert(opt2.value_or_else([] { return 0; }) == 42);
    /// ```
    ///
    /// @param f A callable that creates a default value.
    template <typename F>
    [[nodiscard]] constexpr value_type value_or_else(F&& f) const& {
        if (opt_.index() != 0) {
            return opt_[index<1>];
        } else {
            if constexpr (std::is_void_v<value_type>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return static_cast<value_type>(std::invoke(std::forward<F>(f)));
            }
        }
    }

    /// @brief Gets the @ref option value with a default used for `none`.
    ///
    /// @details
    /// If the @ref option contains a value, the moved value is returned. If
    /// the @ref option is `none`, the result of invoking `f` is returned. The
    /// returned value of `f` is also explicitly cast to `value_type` before
    /// return.
    ///
    /// This function might be preferred over `.value_or(default_value)` when
    /// the default value is expensive to create or move. This function allows
    /// the default value to never be instantiated if the @ref option is not
    /// `none`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// assert(std::move(opt1).value_or_else([] { return 0; }) == 0);
    ///
    /// assert(std::move(opt2).value_or_else([] { return 0; }) == 42);
    /// ```
    ///
    /// @param f A callable that creates a default value.
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `error` value of the @ref result.
    ///
    /// The default error value is always converted to a non-reference. If a
    /// reference is desired, use the overload of this function that accepts
    /// an @ref error_t.
    ///
    /// This function only participates in overload resolution if `E` is not
    /// an @ref error_t.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = opt1.ok_or(-42);
    /// result<int, int> res2 = opt2.ok_or(-42);
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param err The default error value used if the @ref option is `none`.
    template <typename E>
#ifndef DOXYGEN
        requires(!detail::is_error_v<std::remove_cvref_t<E>>)
#endif
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `error` value of the @ref result.
    ///
    /// The default error value is always converted to a non-reference. If a
    /// reference is desired, use the overload of this function that accepts
    /// an @ref error_t.
    ///
    /// This function only participates in overload resolution if `E` is not
    /// an @ref error_t.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = std::move(opt1).ok_or(-42);
    /// result<int, int> res2 = std::move(opt2).ok_or(-42);
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param err The default error value used if the @ref option is `none`.
    template <typename E>
#ifndef DOXYGEN
        requires(!detail::is_error_v<std::remove_cvref_t<E>>)
#endif
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the provided error value
    /// is used as the error value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// const auto err = error<int>(-42);
    ///
    /// result<int, int> res1 = opt1.ok_or(err);
    /// result<int, int> res2 = opt2.ok_or(err);
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param err The default error value used if the @ref option is `none`.
    template <typename E>
    [[nodiscard]] constexpr result<T, E> ok_or(const error_t<E>& err) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return opt_[index<1>];
            }
        } else {
            return err;
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the provided error value
    /// is used as the error value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// const auto err = error<int>(-42);
    ///
    /// result<int, int> res1 = std::move(opt1).ok_or(err);
    /// result<int, int> res2 = std::move(opt2).ok_or(err);
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param err The default error value used if the @ref option is `none`.
    template <typename E>
    [[nodiscard]] constexpr result<T, E> ok_or(const error_t<E>& err) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return std::move(opt_)[index<1>];
            }
        } else {
            return err;
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the provided error value
    /// is used as the error value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = opt1.ok_or(error<int>(-42));
    /// result<int, int> res2 = opt2.ok_or(error<int>(-42));
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param err The default error value used if the @ref option is `none`.
    template <typename E>
    [[nodiscard]] constexpr result<T, E> ok_or(error_t<E>&& err) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return opt_[index<1>];
            }
        } else {
            return std::move(err);
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the provided error value
    /// is used as the error value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = std::move(opt1).ok_or(error<int>(-42));
    /// result<int, int> res2 = std::move(opt2).ok_or(error<int>(-42));
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param err The default error value used if the @ref option is `none`.
    template <typename E>
    [[nodiscard]] constexpr result<T, E> ok_or(error_t<E>&& err) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return std::move(opt_)[index<1>];
            }
        } else {
            return std::move(err);
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This fucntion constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the result of invoking
    /// the provided callable is used as the error value of the @ref result.
    ///
    /// This function might be preferred over `.ok_or(error_value)` when the
    /// error type is expense to construct, copy, or move. This function allows
    /// the error value to never be instantiated if the @ref option is not
    /// `none`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = opt1.ok_or_else([] { return -42; });
    /// result<int, int> res2 = opt2.ok_or_else([] { return -42; });
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param f A callable that creates a default error value.
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This fucntion constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `ok` value of the
    /// @ref result. If the @ref option is `none`, the result of invoking
    /// the provided callable is used as the error value of the @ref result.
    ///
    /// This function might be preferred over `.ok_or(error_value)` when the
    /// error type is expense to construct, copy, or move. This function allows
    /// the error value to never be instantiated if the @ref option is not
    /// `none`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = std::move(opt1).ok_or_else([] { return -42; });
    /// result<int, int> res2 = std::move(opt2).ok_or_else([] { return -42; });
    ///
    /// assert(!res1.has_value());
    /// assert(res1.error() == -42);
    ///
    /// assert(res2.has_value());
    /// assert(*res2 == 42);
    /// ```
    ///
    /// @param f A callable that creates a default error value.
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `ok` value of the @ref result.
    ///
    /// The default ok value is always converted to a non-reference. If a
    /// reference is desired, use the overload of this function that accepts
    /// an @ref ok_t.
    ///
    /// This function only participates in overload resolution if `U` is not
    /// an @ref ok_t.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = opt1.error_or(-42);
    /// result<int, int> res2 = opt2.error_or(-42);
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param err The default ok value used if the @ref option is `none`.
    template <typename U>
#ifndef DOXYGEN
        requires(!detail::is_ok_v<std::remove_cvref_t<U>>)
#endif
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `ok` value of the @ref result.
    ///
    /// The default ok value is always converted to a non-reference. If a
    /// reference is desired, use the overload of this function that accepts
    /// an @ref ok_t.
    ///
    /// This function only participates in overload resolution if `U` is not
    /// an @ref ok_t.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = std::move(opt1).error_or(-42);
    /// result<int, int> res2 = std::move(opt2).error_or(-42);
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param err The default ok value used if the @ref option is `none`.
    template <typename U>
#ifndef DOXYGEN
        requires(!detail::is_ok_v<std::remove_cvref_t<U>>)
#endif
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `ok` value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// const auto value = ok<int>(-42);
    ///
    /// result<int, int> res1 = opt1.error_or(value);
    /// result<int, int> res2 = opt2.error_or(value);
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param err The default ok value used if the @ref option is `none`.
    template <typename U>
    [[nodiscard]] constexpr result<U, T> error_or(const ok_t<U>& value) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<U, T>{in_place_error};
            } else {
                return result<U, T>{in_place_error, opt_[index<1>]};
            }
        } else {
            return value;
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `ok` value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// const auto value = ok<int>(-42);
    ///
    /// result<int, int> res1 = std::move(opt1).error_or(value);
    /// result<int, int> res2 = std::move(opt2).error_or(value);
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param err The default ok value used if the @ref option is `none`.
    template <typename U>
    [[nodiscard]] constexpr result<U, T> error_or(const ok_t<U>& value) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<U, T>{in_place_error};
            } else {
                return result<U, T>{in_place_error, std::move(opt_)[index<1>]};
            }
        } else {
            return value;
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `ok` value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = opt1.error_or(ok<int>(-42));
    /// result<int, int> res2 = opt2.error_or(ok<int>(-42));
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param err The default ok value used if the @ref option is `none`.
    template <typename U>
    [[nodiscard]] constexpr result<U, T> error_or(ok_t<U>&& value) const& {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<U, T>{in_place_error};
            } else {
                return result<U, T>{in_place_error, opt_[index<1>]};
            }
        } else {
            return std::move(value);
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This function constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the provided default
    /// value is used as the `ok` value of the @ref result.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = std::move(opt1).error_or(ok<int>(-42));
    /// result<int, int> res2 = std::move(opt2).error_or(ok<int>(-42));
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param err The default ok value used if the @ref option is `none`.
    template <typename U>
    [[nodiscard]] constexpr result<U, T> error_or(ok_t<U>&& value) && {
        if (opt_.index() != 0) {
            if constexpr (std::is_void_v<T>) {
                return result<U, T>{in_place_error};
            } else {
                return result<U, T>{in_place_error, std::move(opt_)[index<1>]};
            }
        } else {
            return std::move(value);
        }
    }

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This fucntion constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the result of invoking
    /// the provided callable is used as the `ok` value of the @ref result.
    ///
    /// This function might be preferred over `.error_or(ok_value)` when the
    /// value type is expensive to construct, copy, or move. This function
    /// allows the value to never be instantiated if the @ref option is not
    /// `none`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = opt1.error_or_else([] { return -42; });
    /// result<int, int> res2 = opt2.error_or_else([] { return -42; });
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param f A callable that creates a default ok value.
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

    /// @brief Converts an @ref option to a @ref result.
    ///
    /// @details
    /// This fucntion constructs and returns a @ref result such that the
    /// contained value of the @ref option becomes the `error` value of the
    /// @ref result. If the @ref option is `none`, the result of invoking
    /// the provided callable is used as the `ok` value of the @ref result.
    ///
    /// This function might be preferred over `.error_or(ok_value)` when the
    /// value type is expensive to construct, copy, or move. This function
    /// allows the value to never be instantiated if the @ref option is not
    /// `none`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// result<int, int> res1 = std::move(opt1).error_or_else([] {
    ///     return -42;
    /// });
    /// result<int, int> res2 = std::move(opt2).error_or_else([] {
    ///     return -42;
    /// });
    ///
    /// assert(res1.has_value());
    /// assert(*res1 == -42);
    ///
    /// assert(!res2.has_value());
    /// assert(res2.error() == 42);
    /// ```
    ///
    /// @param f A callable that creates a default ok value.
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

    /// @brief Converts an @ref option into an @ref option of a reference.
    ///
    /// @details
    /// This function acts as a monadic operation that gets a reference to the
    /// contained value without unwrapping the @ref option. In the case where
    /// the contained value is `void`, the returned @ref option is also `void`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// option<int&> opt1_ref = opt1.ref();
    /// option<int&> opt2_ref = opt2.ref();
    ///
    /// assert(opt1_ref == nullptr);
    /// assert(opt2_ref == &*opt2);
    /// ```
    [[nodiscard]] constexpr option<reference> ref() noexcept {
        if (opt_.index() != 0) {
            return option<reference>{std::in_place, opt_[index<1>]};
        } else {
            return option<reference>{};
        }
    }

    /// @brief Converts an @ref option into an @ref option of a reference.
    ///
    /// @details
    /// This function acts as a monadic operation that gets a reference to the
    /// contained value without unwrapping the @ref option. In the case where
    /// the contained value is `void`, the returned @ref option is also `void`.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// option<const int&> opt1_ref = opt1.ref();
    /// option<const int&> opt2_ref = opt2.ref();
    ///
    /// assert(opt1_ref == nullptr);
    /// assert(opt2_ref == &*opt2);
    /// ```
    [[nodiscard]] constexpr option<const_reference> ref() const noexcept {
        if (opt_.index() != 0) {
            return option<const_reference>{std::in_place, opt_[index<1>]};
        } else {
            return option<const_reference>{};
        }
    }

    /// @brief Converts an @ref option into an @ref option of a reference.
    ///
    /// @details
    /// This function acts as a monadic operation that gets a reference to the
    /// contained value without unwrapping the @ref option. In the case where
    /// the contained value is `void`, the returned @ref option is also `void`.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// option<const int&> opt1_ref = opt1.cref();
    /// option<const int&> opt2_ref = opt2.cref();
    ///
    /// assert(opt1_ref == nullptr);
    /// assert(opt2_ref == &*opt2);
    /// ```
    [[nodiscard]] constexpr option<const_reference> cref() const noexcept { return ref(); }

    /// @brief Applies a callable to the contents of an @ref option.
    ///
    /// @details
    /// If the @ref option contains a value, the value is passed into the
    /// callable, and the invocation result is returned from this function.
    /// If the @ref option is `none`, the callable is not invoked and `none`
    /// is returned.
    ///
    /// This function only participates in overload resolution if the
    /// invocation result of the callable is an @ref option.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    /// option<int> opt3{-42};
    ///
    /// auto callable = [](int value) -> option<unsigned int> {
    ///     if (value < 0) {
    ///         return none;
    ///     } else {
    ///         return static_cast<unsigned int>(value);
    ///     }
    /// };
    ///
    /// auto opt1_res = opt1.and_then(callable);
    /// auto opt2_res = opt2.and_then(callable);
    /// auto opt3_res = opt3.and_then(callable);
    ///
    /// assert(!opt1_res.has_value());
    ///
    /// assert(opt2_res.has_value());
    /// assert(*opt2_res == 42);
    ///
    /// assert(!opt3_res.has_value());
    /// ```
    ///
    /// @param f Callable applied to the contained option value
    template <typename F>
#ifndef DOXYGEN
        requires(
            detail::is_option_v<std::remove_cvref_t<detail::invoke_result_t<F, reference>>>)
#endif
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

    /// @brief Applies a callable to the contents of an @ref option.
    ///
    /// @details
    /// If the @ref option contains a value, the value is passed into the
    /// callable, and the invocation result is returned from this function.
    /// If the @ref option is `none`, the callable is not invoked and `none`
    /// is returned.
    ///
    /// This function only participates in overload resolution if the
    /// invocation result of the callable is an @ref option.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    /// const option<int> opt3{-42};
    ///
    /// auto callable = [](int value) -> option<unsigned int> {
    ///     if (value < 0) {
    ///         return none;
    ///     } else {
    ///         return static_cast<unsigned int>(value);
    ///     }
    /// };
    ///
    /// auto opt1_res = opt1.and_then(callable);
    /// auto opt2_res = opt2.and_then(callable);
    /// auto opt3_res = opt3.and_then(callable);
    ///
    /// assert(!opt1_res.has_value());
    ///
    /// assert(opt2_res.has_value());
    /// assert(*opt2_res == 42);
    ///
    /// assert(!opt3_res.has_value());
    /// ```
    ///
    /// @param f Callable applied to the contained option value
    template <typename F>
#ifndef DOXYGEN
        requires(detail::is_option_v<
                 std::remove_cvref_t<detail::invoke_result_t<F, const_reference>>>)
#endif
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

    /// @brief Applies a callable to the contents of an @ref option.
    ///
    /// @details
    /// If the @ref option contains a value, the value is passed into the
    /// callable, and the invocation result is returned from this function.
    /// If the @ref option is `none`, the callable is not invoked and `none`
    /// is returned.
    ///
    /// This function only participates in overload resolution if the
    /// invocation result of the callable is an @ref option.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    /// option<int> opt3{-42};
    ///
    /// auto callable = [](int value) -> option<unsigned int> {
    ///     if (value < 0) {
    ///         return none;
    ///     } else {
    ///         return static_cast<unsigned int>(value);
    ///     }
    /// };
    ///
    /// auto opt1_res = std::move(opt1).and_then(callable);
    /// auto opt2_res = std::move(opt2).and_then(callable);
    /// auto opt3_res = std::move(opt3).and_then(callable);
    ///
    /// assert(!opt1_res.has_value());
    ///
    /// assert(opt2_res.has_value());
    /// assert(*opt2_res == 42);
    ///
    /// assert(!opt3_res.has_value());
    /// ```
    ///
    /// @param f Callable applied to the contained option value
    template <typename F>
#ifndef DOXYGEN
        requires(detail::is_option_v<
                 std::remove_cvref_t<detail::invoke_result_t<F, rvalue_reference>>>)
#endif
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

    /// @brief Applies a callable to the contents of an @ref option.
    ///
    /// @details
    /// If the @ref option contains a value, the value is passed into the
    /// callable, and the invocation result is returned from this function.
    /// If the @ref option is `none`, the callable is not invoked and `none`
    /// is returned.
    ///
    /// This function only participates in overload resolution if the
    /// invocation result of the callable is an @ref option.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    /// const option<int> opt3{-42};
    ///
    /// auto callable = [](int value) -> option<unsigned int> {
    ///     if (value < 0) {
    ///         return none;
    ///     } else {
    ///         return static_cast<unsigned int>(value);
    ///     }
    /// };
    ///
    /// auto opt1_res = std::move(opt1).and_then(callable);
    /// auto opt2_res = std::move(opt2).and_then(callable);
    /// auto opt3_res = std::move(opt3).and_then(callable);
    ///
    /// assert(!opt1_res.has_value());
    ///
    /// assert(opt2_res.has_value());
    /// assert(*opt2_res == 42);
    ///
    /// assert(!opt3_res.has_value());
    /// ```
    ///
    /// @param f Callable applied to the contained option value
    template <typename F>
#ifndef DOXYGEN
        requires(detail::is_option_v<
                 std::remove_cvref_t<detail::invoke_result_t<F, const_rvalue_reference>>>)
#endif
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

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref map, but the name
    /// `transform` make @ref option able to be a drop in replacement for
    /// `std::optional`. `map` is the more typical name of this monadic
    /// operation outside of C++.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// auto opt1_mapped = opt1.transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = opt2.transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that transforms the @ref option value
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

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref map, but the name
    /// `transform` make @ref option able to be a drop in replacement for
    /// `std::optional`. `map` is the more typical name of this monadic
    /// operation outside of C++.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// auto opt1_mapped = opt1.transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = opt2.transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that transforms the @ref option value
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

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref map, but the name
    /// `transform` make @ref option able to be a drop in replacement for
    /// `std::optional`. `map` is the more typical name of this monadic
    /// operation outside of C++.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// auto opt1_mapped = std::move(opt1).transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = std::move(opt2).transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that transforms the @ref option value
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

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref map, but the name
    /// `transform` make @ref option able to be a drop in replacement for
    /// `std::optional`. `map` is the more typical name of this monadic
    /// operation outside of C++.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// auto opt1_mapped = std::move(opt1).transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = std::move(opt2).transform([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that transforms the @ref option value
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

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref transform, but the name
    /// `map` is the more typical name of this monadic operation outside of
    /// C++.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// auto opt1_mapped = opt1.map([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = opt2.map([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that maps the @ref option value
    template <typename F>
    constexpr auto map(F&& f) & {
        return transform(std::forward<F>(f));
    }

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref transform, but the name
    /// `map` is the more typical name of this monadic operation outside of
    /// C++.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// auto opt1_mapped = opt1.map([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = opt2.map([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that maps the @ref option value
    template <typename F>
    constexpr auto map(F&& f) const& {
        return transform(std::forward<F>(f));
    }

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref transform, but the name
    /// `map` is the more typical name of this monadic operation outside of
    /// C++.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// auto opt1_mapped = std::move(opt1).map([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = std::move(opt2).map([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that maps the @ref option value
    template <typename F>
    constexpr auto map(F&& f) && {
        return std::move(*this).transform(std::forward<F>(f));
    }

    /// @brief Performs a transformation on the content of an @ref option.
    ///
    /// @details
    /// This function is a monadic operation that passes the value contained
    /// in the @ref option, if any, to the callable `f`. The value returned by
    /// the call to `f` is then returned from this function wrapped in a new
    /// @ref option. If the original @ref option is `none`, the returned value
    /// is a `none` @ref option of the type that would have been returned by
    /// `f`.
    ///
    /// Note that this function is identical to @ref transform, but the name
    /// `map` is the more typical name of this monadic operation outside of
    /// C++.
    ///
    /// ## Example
    /// ```
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// auto opt1_mapped = std::move(opt1).map([](auto value) {
    ///     return std::to_string(value);
    /// });
    /// auto opt2_mapped = std::move(opt2).map([](auto value) {
    ///     return std::to_string(value);
    /// });
    ///
    /// assert(!opt1_mapped.has_value());
    ///
    /// assert(opt2_mapped.has_value());
    /// assert(*opt2_mapped = "42");
    /// ```
    ///
    /// @param f A callable that maps the @ref option value
    template <typename F>
    constexpr auto map(F&& f) const&& {
        return std::move(*this).transform(std::forward<F>(f));
    }

    /// @brief Returns the result of `f` if the @ref option is `none`.
    ///
    /// @details
    /// If the @ref option contains a value, a copy of the @ref option is
    /// returned. Otherwise, the result of invoking `f` is returned.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// auto opt1_new = opt1.or_else([] { return 0; });
    /// auto opt2_new = opt2.or_else([] { return 0; });
    ///
    /// assert(opt1_new.has_value());
    /// assert(*opt1_new == 0);
    ///
    /// assert(opt2_new.has_value());
    /// assert(*opt2_new == 42);
    /// ```
    ///
    /// @param f A callable that returns the "else" @ref option
    template <typename F>
    constexpr option or_else(F&& f) const& {
        if (opt_.index() != 0) {
            return *this;
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    /// @brief Returns the result of `f` if the @ref option is `none`.
    ///
    /// @details
    /// If the @ref option contains a value, a copy of the @ref option is
    /// returned. Otherwise, the result of invoking `f` is returned.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// auto opt1_new = std::move(opt1).or_else([] { return 0; });
    /// auto opt2_new = std::move(opt2).or_else([] { return 0; });
    ///
    /// assert(opt1_new.has_value());
    /// assert(*opt1_new == 0);
    ///
    /// assert(opt2_new.has_value());
    /// assert(*opt2_new == 42);
    /// ```
    ///
    /// @param f A callable that returns the "else" @ref option
    template <typename F>
    constexpr option or_else(F&& f) && {
        if (opt_.index() != 0) {
            return std::move(*this);
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    /// @brief Calls a visitor callable with the contained value or `void`.
    ///
    /// @details
    /// This function treats an @ref option as if it was a @ref variant of
    /// `void` and `T`. If the @ref option contains a value, that value is
    /// passed to the visitor. If the @ref option is `none`, the visitor is
    /// called with @ref void_t.
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// ## Example
    /// ```cpp
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// opt1.visit(overload([](int value) {
    ///     assert(false);
    /// }, [](void_t value) {
    ///     assert(true);
    /// });
    ///
    /// opt2.visit(overload([](int value) {
    ///     assert(value == 42);
    /// }, [](void_t value) {
    ///     assert(false);
    /// });
    /// ```
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

    /// @brief Calls a visitor callable with the contained value or `void`.
    ///
    /// @details
    /// This function treats an @ref option as if it was a @ref variant of
    /// `void` and `T`. If the @ref option contains a value, that value is
    /// passed to the visitor. If the @ref option is `none`, the visitor is
    /// called with @ref void_t.
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// ## Example
    /// ```cpp
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// opt1.visit(overload([](int value) {
    ///     assert(false);
    /// }, [](void_t value) {
    ///     assert(true);
    /// });
    ///
    /// opt2.visit(overload([](int value) {
    ///     assert(value == 42);
    /// }, [](void_t value) {
    ///     assert(false);
    /// });
    /// ```
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

    /// @brief Calls a visitor callable with the contained value or `void`.
    ///
    /// @details
    /// This function treats an @ref option as if it was a @ref variant of
    /// `void` and `T`. If the @ref option contains a value, that value is
    /// passed to the visitor. If the @ref option is `none`, the visitor is
    /// called without any arguments.
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// ## Example
    /// ```cpp
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// std::move(opt1).visit(overload([](int value) {
    ///     assert(false);
    /// }, [](void_t value) {
    ///     assert(true);
    /// });
    ///
    /// std::move(opt2).visit(overload([](int value) {
    ///     assert(value == 42);
    /// }, [](void_t value) {
    ///     assert(false);
    /// });
    /// ```
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

    /// @brief Calls a visitor callable with the contained value or `void`.
    ///
    /// @details
    /// This function treats an @ref option as if it was a @ref variant of
    /// `void` and `T`. If the @ref option contains a value, that value is
    /// passed to the visitor. If the @ref option is `none`, the visitor is
    /// called with @ref void_t.
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// ## Example
    /// ```cpp
    /// const option<int> opt1{};
    /// const option<int> opt2{42};
    ///
    /// std::move(opt1).visit(overload([](int value) {
    ///     assert(false);
    /// }, [](void_t value) {
    ///     assert(true);
    /// });
    ///
    /// std::move(opt2).visit(overload([](int value) {
    ///     assert(value == 42);
    /// }, [](void_t value) {
    ///     assert(false);
    /// });
    /// ```
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

    /// @brief Swaps two @ref option instances
    ///
    /// @details
    /// If both @ref option instances contain a value, the two contained values
    /// are swapped directly. Otherwise, if one @ref option contains a value,
    /// that value is move constructed into the other @ref option, and the old
    /// value is destroyed in place. If both @ref option instances are `none`,
    /// nothing is done.
    ///
    /// This function is `noexcept` if the contained value type is nothrow
    /// swappable.
    ///
    /// # Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// opt1.swap(opt2);
    ///
    /// assert(opt1.has_value());
    /// assert(*opt1 == 42);
    ///
    /// assert(!opt1.has_value());
    /// ```
    constexpr void swap(option& other)
#ifndef DOXYGEN
        noexcept(noexcept(opt_.swap(other.opt_)))
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        opt_.swap(other.opt_);
    }

    /// @brief Sets the @ref option to `none`.
    ///
    /// @details
    /// If the @ref option contains a value, the value is destroyed in place.
    /// Otherwise, nothing is done.
    ///
    /// ## Example
    /// ```
    /// option<int> opt1{};
    /// option<int> opt2{42};
    ///
    /// opt1.reset();
    /// opt2.reset();
    ///
    /// assert(!opt1.has_value());
    /// assert(!opt2.has_value());
    /// ```
    constexpr void reset() noexcept { opt_.template emplace<0>(); }

    /// @brief Constructs a new value in place into the @ref option.
    ///
    /// @details
    /// If the @ref option already contains a value, the old value is first
    /// destroyed in place. In any case, a new value is constructed in place
    /// in the @ref option.
    ///
    /// ## Example
    /// ```
    /// option<std::string> opt1{};
    /// option<std::string> opt2{"hello"};
    ///
    /// opt1.emplace(5, 'a');
    /// opt2.emplace(5, 'a');
    ///
    /// assert(opt1.has_value());
    /// assert(*opt1 == "aaaaa");
    ///
    /// assert(opt2.has_value());
    /// assert(*opt2 == "aaaaa");
    /// ```
    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        opt_.template emplace<1>(std::forward<Args>(args)...);
        return opt_[index<1>];
    }
};

/// @relates option
/// @brief Gets an @ref option value by index, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where index 0 gets `void` (which represents `none`)
/// and index 1 gets `T` (which is the contained value).
///
/// ## Example
/// ```
/// option<int> opt{42};
///
/// assert(get<1>(opt) == 42);
/// ```
///
/// @tparam IDX The "alternative" index
///
/// @param opt The @ref option to access
///
/// @return A reference to the accessed "alternative"
///
/// @throws bad_option_access Thrown if the @ref option state does not match
/// the index `IDX`.
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
/// @brief Gets an @ref option value by index, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where index 0 gets `void` (which represents `none`)
/// and index 1 gets `T` (which is the contained value).
///
/// ## Example
/// ```
/// const option<int> opt{42};
///
/// assert(get<1>(opt) == 42);
/// ```
///
/// @tparam IDX The "alternative" index
///
/// @param opt The @ref option to access
///
/// @return A const reference to the accessed "alternative"
///
/// @throws bad_option_access Thrown if the @ref option state does not match
/// the index `IDX`.
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
/// @brief Gets an @ref option value by index, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where index 0 gets `void` (which represents `none`)
/// and index 1 gets `T` (which is the contained value).
///
/// ## Example
/// ```
/// option<int> opt{42};
///
/// assert(get<1>(std::move(opt)) == 42);
/// ```
///
/// @tparam IDX The "alternative" index
///
/// @param opt The @ref option to access
///
/// @return An rvalue reference to the accessed "alternative"
///
/// @throws bad_option_access Thrown if the @ref option state does not match
/// the index `IDX`.
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
/// @brief Gets an @ref option value by index, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where index 0 gets `void` (which represents `none`)
/// and index 1 gets `T` (which is the contained value).
///
/// ## Example
/// ```
/// const option<int> opt{42};
///
/// assert(get<1>(std::move(opt)) == 42);
/// ```
///
/// @tparam IDX The "alternative" index
///
/// @param opt The @ref option to access
///
/// @return A const rvalue reference to the accessed "alternative"
///
/// @throws bad_option_access Thrown if the @ref option state does not match
/// the index `IDX`.
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
/// @brief Gets an @ref option value by type, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where `void` is the `none` alternative, and `T` is
/// the alternative with a value.
///
/// This function only participates in overload resolution if the contained
/// value of the @ref option is not `void`.
///
/// ## Example
/// ```
/// option<int> opt{42};
///
/// assert(get<int>(opt) == 42);
/// ```
///
/// @tparam T The type of the alternative to access
///
/// @param opt The @ref option to access
///
/// @return A reference to the accessed alternative
///
/// @throws bad_option_access Thrown if the @ref option does not contain the
/// requested type `T`.
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
/// @brief Gets an @ref option value by type, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where `void` is the `none` alternative, and `T` is
/// the alternative with a value.
///
/// This function only participates in overload resolution if the contained
/// value of the @ref option is not `void`.
///
/// ## Example
/// ```
/// const option<int> opt{42};
///
/// assert(get<int>(opt) == 42);
/// ```
///
/// @tparam T The type of the alternative to access
///
/// @param opt The @ref option to access
///
/// @return A const reference to the accessed alternative
///
/// @throws bad_option_access Thrown if the @ref option does not contain the
/// requested type `T`.
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
/// @brief Gets an @ref option value by type, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where `void` is the `none` alternative, and `T` is
/// the alternative with a value.
///
/// This function only participates in overload resolution if the contained
/// value of the @ref option is not `void`.
///
/// ## Example
/// ```
/// option<int> opt{42};
///
/// assert(get<int>(std::move(opt)) == 42);
/// ```
///
/// @tparam T The type of the alternative to access
///
/// @param opt The @ref option to access
///
/// @return An rvalue reference to the accessed alternative
///
/// @throws bad_option_access Thrown if the @ref option does not contain the
/// requested type `T`.
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
/// @brief Gets an @ref option value by type, as if it were a @ref variant.
///
/// @details
/// This function is provided to make @ref option generically compatible with
/// @ref variant. This function treats `option<T>` as if it were
/// `variant<void, T>`, where `void` is the `none` alternative, and `T` is
/// the alternative with a value.
///
/// This function only participates in overload resolution if the contained
/// value of the @ref option is not `void`.
///
/// ## Example
/// ```
/// const option<int> opt{42};
///
/// assert(get<int>(std::move(opt)) == 42);
/// ```
///
/// @tparam T The type of the alternative to access
///
/// @param opt The @ref option to access
///
/// @return A const rvalue reference to the accessed alternative
///
/// @throws bad_option_access Thrown if the @ref option does not contain the
/// requested type `T`.
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
/// @brief Tests two @ref option instances for equality.
///
/// @details
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert(opt1 == opt1);
/// assert(opt2 == opt2);
/// assert(!(opt1 == opt2));
/// assert(!(opt2 == opt3));
/// ```
template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value()) {
        return rhs.has_value() && *lhs == *rhs;
    } else {
        return !rhs.has_value();
    }
}

/// @relates option
/// @brief Tests two @ref option instances for inequality.
///
/// @details
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert(!(opt1 != opt1));
/// assert(!(opt2 != opt2));
/// assert(opt1 != opt2);
/// assert(opt2 != opt3);
/// ```
template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const option<U>& rhs) {
    if (lhs.has_value()) {
        return !rhs.has_value() || *lhs != *rhs;
    } else {
        return rhs.has_value();
    }
}

/// @relates option
/// @brief Compares two @ref option instances.
///
/// @details
/// `none` is always less than any contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert(opt1 < opt2);
/// assert(opt1 < opt3);
/// assert(opt3 < opt2);
/// ```
template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const option<U>& rhs) {
    return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

/// @relates option
/// @brief Compares two @ref option instances.
///
/// @details
/// `none` is always less than any contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert(opt2 > opt1);
/// assert(opt3 > opt1);
/// assert(opt2 > opt3);
/// ```
template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const option<U>& rhs) {
    return lhs.has_value() && (!rhs.has_value() || *lhs > *rhs);
}

/// @relates option
/// @brief Compares two @ref option instances.
///
/// @details
/// `none` is always less than any contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert(opt1 <= opt2);
/// assert(opt1 <= opt3);
/// assert(opt3 <= opt2);
/// assert(opt2 <= opt2);
/// ```
template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const option<U>& rhs) {
    return !lhs.has_value() || (rhs.has_value() && *lhs <= *rhs);
}

/// @relates option
/// @brief Compares two @ref option instances.
///
/// @details
/// `none` is always less than any contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert(opt2 >= opt1);
/// assert(opt3 >= opt1);
/// assert(opt2 >= opt3);
/// assert(opt2 >= opt2);
/// ```
template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const option<U>& rhs) {
    return !rhs.has_value() || (lhs.has_value() && *lhs >= *rhs);
}

/// @relates option
/// @brief Compares two @ref option instances.
///
/// @details
/// `none` is always less than any contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
/// option<int> opt3{24};
///
/// assert((opt1 <=> opt2) == std::strong_ordering::less);
/// assert((opt1 <=> opt3) == std::strong_ordering::less);
/// assert((opt2 <=> opt3) == std::strong_ordering::greater);
/// assert((opt2 <=> opt2) == std::strong_ordering::equal);
/// ```
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
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 == 42));
/// assert(opt2 == 42);
/// assert(!(opt2 == 24));
/// ```
template <typename T, typename U>
constexpr bool operator==(const option<T>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(42 == opt1));
/// assert(42 == opt2);
/// assert(!(24 == opt2));
/// ```
template <typename T, typename U>
constexpr bool operator==(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs == *rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 != 42);
/// assert(!(opt2 != 42));
/// assert(opt2 != 24);
/// ```
template <typename T, typename U>
constexpr bool operator!=(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs != rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(42 != opt1);
/// assert(!(42 != opt2));
/// assert(24 != opt2);
/// ```
template <typename T, typename U>
constexpr bool operator!=(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs != *rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 < 42);
/// assert(!(opt2 < 42));
/// assert(opt2 < 100);
/// ```
template <typename T, typename U>
constexpr bool operator<(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs < rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(42 < opt1));
/// assert(!(42 < opt2));
/// assert(40 < opt2);
/// ```
template <typename T, typename U>
constexpr bool operator<(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs < *rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 > 42));
/// assert(!(opt2 > 42));
/// assert(opt2 > 40);
/// ```
template <typename T, typename U>
constexpr bool operator>(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() && *lhs > rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(42 > opt);
/// assert(!(42 > opt2));
/// assert(100 > opt2);
/// ```
template <typename T, typename U>
constexpr bool operator>(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs > *lhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 <= 42);
/// assert(opt2 <= 42);
/// assert(!(opt2 <= 100));
/// ```
template <typename T, typename U>
constexpr bool operator<=(const option<T>& lhs, const U& rhs) {
    return !lhs.has_value() || *lhs <= rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(42 <= opt1));
/// assert(42 <= opt2);
/// assert(!(100 <= opt2));
/// ```
template <typename T, typename U>
constexpr bool operator<=(const U& lhs, const option<T>& rhs) {
    return rhs.has_value() && lhs <= *rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 >= 42));
/// assert(opt2 >= 42);
/// assert(!(opt2 >= 100));
/// ```
template <typename T, typename U>
constexpr bool operator>=(const option<T>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs >= rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(42 >= opt1);
/// assert(42 >= opt2);
/// assert(!(40 >= opt2));
/// ```
template <typename T, typename U>
constexpr bool operator>=(const U& lhs, const option<T>& rhs) {
    return !rhs.has_value() || lhs >= *rhs;
}

/// @relates option
/// @brief Compares an @ref option with a value.
///
/// @details
/// `none` is always less than the value. If not `none`, the contained value
/// is compared with the value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert((opt1 <=> 42) == std::strong_ordering::less);
/// assert((42 <=> opt1) == std::strong_ordering::greater);
/// assert((opt2 <=> 42) == std::strong_ordering::equal);
/// assert((40 <=> opt2) == std::strong_ordering::less);
/// ```
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
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 == none);
/// assert(!(opt2 == none));
/// ```
template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return !lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(none == opt1);
/// assert(!(none == opt2));
/// ```
template <typename T>
constexpr bool operator==([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 != none));
/// assert(opt2 != none);
/// ```
template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(none != opt1));
/// assert(none != opt2);
/// ```
template <typename T>
constexpr bool operator!=([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 < none));
/// assert(!(opt2 < none));
/// ```
template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] none_t rhs) {
    return false;
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(none < opt1));
/// assert(none < opt2);
/// ```
template <typename T>
constexpr bool operator<([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 > none));
/// assert(opt2 > none);
/// ```
template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(none > opt1));
/// assert(!(none > opt2));
/// ```
template <typename T>
constexpr bool operator>([[maybe_unused]] none_t lhs,
                         [[maybe_unused]] const option<T>& rhs) {
    return false;
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 <= none);
/// assert(!(opt2 <= none));
/// ```
template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] none_t rhs) {
    return !lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(none <= opt1);
/// assert(none <= opt2);
/// ```
template <typename T>
constexpr bool operator<=([[maybe_unused]] none_t lhs,
                          [[maybe_unused]] const option<T>& rhs) {
    return true;
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 >= none);
/// assert(opt2 >= none);
/// ```
template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] none_t rhs) {
    return true;
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(none >= opt1);
/// assert(!(none >= opt2));
/// ```
template <typename T>
constexpr bool operator>=([[maybe_unused]] none_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `none`.
///
/// @details
/// `none` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert((opt1 <=> none) == std::strong_ordering::equal);
/// assert((none <=> opt1) == std::strong_ordering::equal);
/// assert((opt2 <=> none) == std::strong_ordering::greater);
/// assert((none <=> opt2) == std::strong_ordering::less);
/// ```
template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] none_t rhs) {
    return lhs.has_value() <=> false;
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 == std::nullopt);
/// assert(!(opt2 == std::nullopt));
/// ```
template <typename T>
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return !lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(std::nullopt == opt1);
/// assert(!(std::nullopt == opt2));
/// ```
template <typename T>
constexpr bool operator==([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 != std::nullopt));
/// assert(opt2 != std::nullopt);
/// ```
template <typename T>
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(std::nullopt != opt1));
/// assert(std::nullopt != opt2);
/// ```
template <typename T>
constexpr bool operator!=([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 < std::nullopt));
/// assert(!(opt2 < std::nullopt));
/// ```
template <typename T>
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] std::nullopt_t rhs) {
    return false;
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(std::nullopt < opt1));
/// assert(std::nullopt < opt2);
/// ```
template <typename T>
constexpr bool operator<([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(opt1 > std::nullopt));
/// assert(opt2 > std::nullopt);
/// ```
template <typename T>
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(!(std::nullopt > opt1));
/// assert(!(std::nullopt > opt2));
/// ```
template <typename T>
constexpr bool operator>([[maybe_unused]] std::nullopt_t lhs,
                         [[maybe_unused]] const option<T>& rhs) {
    return false;
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 <= std::nullopt);
/// assert(!(opt2 <= std::nullopt));
/// ```
template <typename T>
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] std::nullopt_t rhs) {
    return !lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(std::nullopt <= opt1);
/// assert(std::nullopt <= opt2);
/// ```
template <typename T>
constexpr bool operator<=([[maybe_unused]] std::nullopt_t lhs,
                          [[maybe_unused]] const option<T>& rhs) {
    return true;
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(opt1 >= std::nullopt);
/// assert(opt2 >= std::nullopt);
/// ```
template <typename T>
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] std::nullopt_t rhs) {
    return true;
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert(std::nullopt >= opt1);
/// assert(!(std::nullopt >= opt2));
/// ```
template <typename T>
constexpr bool operator>=([[maybe_unused]] std::nullopt_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `std::nullopt`.
///
/// @details
/// `std::nullopt` is always less than an @ref option with a contained value.
///
/// ## Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// assert((opt1 <=> std::nullopt) == std::strong_ordering::equal);
/// assert((std::nullopt <=> opt1) == std::strong_ordering::equal);
/// assert((opt2 <=> std::nullopt) == std::strong_ordering::greater);
/// assert((std::nullopt <=> opt2) == std::strong_ordering::less);
/// ```
template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value() <=> false;
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(opt1 == nullptr);
/// assert(!(opt2 == nullptr));
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator==(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return !lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(nullptr == opt1);
/// assert(!(nullptr == opt2));
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator==([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(!(opt1 != nullptr));
/// assert(opt2 != nullptr);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator!=(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(!(nullptr != opt1));
/// assert(nullptr != opt2);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator!=([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(!(opt1 < nullptr));
/// assert(!(opt2 < nullptr));
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<([[maybe_unused]] const option<T>& lhs,
                         [[maybe_unused]] std::nullptr_t rhs) {
    return false;
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(!(nullptr < opt1));
/// assert(nullptr < opt2);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(!(opt1 > nullptr));
/// assert(opt2 > nullptr);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(!(nullptr > opt1));
/// assert(!(nullptr > opt2));
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>([[maybe_unused]] std::nullptr_t lhs,
                         [[maybe_unused]] const option<T>& rhs) {
    return false;
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(opt1 <= nullptr);
/// assert(!(opt2 <= nullptr));
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<=(const option<T>& lhs, [[maybe_unused]] std::nullptr_t rhs) {
    return !lhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(nullptr <= opt1);
/// assert(nullptr <= opt2);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<=([[maybe_unused]] std::nullptr_t lhs,
                          [[maybe_unused]] const option<T>& rhs) {
    return true;
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(opt1 >= nullptr);
/// assert(opt2 >= nullptr);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>=([[maybe_unused]] const option<T>& lhs,
                          [[maybe_unused]] std::nullptr_t rhs) {
    return true;
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert(nullptr >= opt1);
/// assert(!(nullptr >= opt2));
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>=([[maybe_unused]] std::nullptr_t lhs, const option<T>& rhs) {
    return !rhs.has_value();
}

/// @relates option
/// @brief Compares an @ref option with `nullptr`.
///
/// @details
/// `nullptr` is always less than an @ref option with a contained value.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int value = 42;
/// option<int&> opt1{};
/// option<int&> opt2{value};
///
/// assert((opt1 <=> nullptr) == std::strong_ordering::equal);
/// assert((nullptr <=> opt1) == std::strong_ordering::equal);
/// assert((opt2 <=> nullptr) == std::strong_ordering::greater);
/// assert((nullptr <=> opt2) == std::strong_ordering::less);
/// ```
template <typename T>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value() <=> false;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(!(opt1 == (vals + 2)));
/// assert(opt2 == (vals + 2));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator==(const option<T>& lhs, const U* rhs) {
    return &*lhs == rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(opt1 != (vals + 2));
/// assert(!(opt2 != (vals + 2)));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator!=(const option<T>& lhs, const U* rhs) {
    return &*lhs != rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(opt1 < (vals + 2));
/// assert(!(opt2 < (vals + 2)));
/// assert(opt2 < (vals + 3));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<(const option<T>& lhs, const U* rhs) {
    return &*lhs < rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(!(opt1 > (vals + 2)));
/// assert(!(opt2 > (vals + 2)));
/// assert(opt2 > (vals + 1));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>(const option<T>& lhs, const U* rhs) {
    return &*lhs > rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(opt1 <= (vals + 2));
/// assert(!(opt2 <= (vals + 1)));
/// assert(opt2 <= (vals + 2));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<=(const option<T>& lhs, const U* rhs) {
    return &*lhs <= rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(!(opt1 >= (vals + 2)));
/// assert(opt2 >= (vals + 2));
/// assert(!(opt2 >= (vals + 3)));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>=(const option<T>& lhs, const U* rhs) {
    return &*lhs >= rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(!((vals + 2) == opt1));
/// assert((vals + 2) == opt2);
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator==(const U* lhs, const option<T>& rhs) {
    return lhs == &*rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert((vals + 2) != opt1);
/// assert(!((vals + 2) != opt2));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator!=(const U* lhs, const option<T>& rhs) {
    return lhs != &*rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(!((vals + 2) < opt1));
/// assert((vals + 2) < opt2);
/// assert(!((vals + 3) < opt2));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<(const U* lhs, const option<T>& rhs) {
    return lhs < &*rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert((vals + 2) > opt1);
/// assert((vals + 2) > opt2);
/// assert(!((vals + 1) > opt2));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>(const U* lhs, const option<T>& rhs) {
    return lhs > &*rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert(!((vals + 2) <= opt1));
/// assert((vals + 2) <= opt2);
/// assert(!((vals + 3) <= opt2));
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator<=(const U* lhs, const option<T>& rhs) {
    return lhs <= &*rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert((vals + 2) >= opt1);
/// assert(!((vals + 1) >= opt2));
/// assert((vals + 2) >= opt2);
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr bool operator>=(const U* lhs, const option<T>& rhs) {
    return lhs >= &*rhs;
}

/// @relates option
/// @brief Compares an @ref option with a pointer.
///
/// @details
/// An @ref option of an lvalue reference is a smart pointer and is directly
/// comparable with raw pointers, where `none` is null, and a contained
/// value is a non-null address.
///
/// This function only participates in overload resolution if the contained
/// type of the @ref option is an lvalue reference.
///
/// ## Example
/// ```
/// int vals[] = {1, 2, 3, 4, 5};
///
/// option<int&> opt1{};
/// option<int&> opt2{vals[2]};
///
/// assert((opt1 <=> (vals + 2)) == std::strong_ordering::less);
/// assert(((vals + 2) <=> opt1) == std::strong_ordering::greater);
/// assert((opt2 <=> (vals + 1)) == std::strong_ordering::greater);
/// assert((opt2 <=> (vals + 2)) == std::strong_ordering::equal);
/// assert((opt2 <=> (vals + 3)) == std::strong_ordering::less);
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_lvalue_reference_v<typename option<T>::value_type>)
#endif
constexpr auto operator<=>(const option<T>& lhs, const U* rhs) {
    return &*lhs <=> rhs;
}

/// @relates option
/// @brief Constructs an @ref option that contains a value.
///
/// @details
/// This function is the counterpart to @ref none that creates an @ref option
/// that contains a value instead of being `none`. The contained value is
/// constructed in place in the @ref option, which is then RVO returned.
///
/// A call to this function must explicitly specify the type `T` that the @ref
/// option should contain, and the passed arguments are forwarded to the
/// constructor of that type.
///
/// ## Example
/// ```
/// int value = 42;
///
/// auto opt1 = some<void>();
/// auto opt2 = some<int>(value);
/// auto opt3 = some<int&>(value);
///
/// assert(opt1.has_value());
///
/// assert(opt2.has_value());
/// assert(*opt2 == 42);
///
/// assert(opt3.has_value());
/// assert(opt3 == &value);
/// ```
template <typename T, typename... Args>
constexpr option<T> some(Args&&... args) {
    return option<T>{std::in_place, std::forward<Args>(args)...};
}

/// @relates option
/// @brief Constructs an @ref option that contains a value.
///
/// @details
/// This function is the counterpart to @ref none that creates an @ref option
/// that contains a value instead of being `none`. The contained value is
/// constructed in place in the @ref option, which is then RVO returned.
///
/// A call to this function must explicitly specify the type `T` that the @ref
/// option should contain, and the passed arguments are forwarded to the
/// constructor of that type.
///
/// ## Example
/// ```
/// auto opt = some<std::vector<int>>({1, 2, 3, 4, 5});
///
/// assert(opt.has_value());
/// assert(opt->size() == 5);
/// ```
template <typename T, typename U, typename... Args>
constexpr option<T> some(std::initializer_list<U> ilist, Args&&... args) {
    return option<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

/// @relates option
/// @brief Swaps two @ref option instances
///
/// @details
/// If both @ref option instances contain a value, the two contained values
/// are swapped directly. Otherwise, if one @ref option contains a value,
/// that value is move constructed into the other @ref option, and the old
/// value is destroyed in place. If both @ref option instances are `none`,
/// nothing is done.
///
/// This function is `noexcept` if the contained value type is nothrow
/// swappable.
///
/// # Example
/// ```
/// option<int> opt1{};
/// option<int> opt2{42};
///
/// swap(opt1, opt2);
///
/// assert(opt1.has_value());
/// assert(*opt1 == 42);
///
/// assert(!opt1.has_value());
/// ```
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
