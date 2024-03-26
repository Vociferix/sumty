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

#ifndef SUMTY_RESULT_HPP
#define SUMTY_RESULT_HPP

#include "sumty/detail/fwd.hpp"    // IWYU pragma: export
#include "sumty/detail/traits.hpp" // IWYU pragma: export
#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/option.hpp" // IWYU pragma: keep
#include "sumty/utils.hpp"  // IWYU pragma: export
#include "sumty/variant.hpp"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace sumty {

/// @class error_t result.hpp <sumty/result.hpp>
/// @brief Wrapper around a value representing an error
///
/// @details
/// @ref error_t is an intermediate type that is generally used to construct a
/// @ref result that contains an error. An @ref error_t instance implicitly
/// converts into a @ref result, as long as the contained error value is
/// implicitly convertible to the error type of the @ref result.
///
/// In most cases, @ref error_t will not be named directly. Instead, an @ref
/// error_t will be created using the @ref error function, as shown in the
/// example below.
///
/// ## Example
/// ```
/// result<int, std::string> ensure_positive(int value) {
///     if (value < 0) {
///         // returns an error_t<std::string>, which gets converted
///         // into a result<int, std::string>.
///         return error<std::string>("value is negative");
///     }
///     return value;
/// }
/// ```
///
/// @tparam E The type of the error value
template <typename E>
class error_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<E> err_;

  public:
#ifndef DOXYGEN
    using value_type = typename detail::traits<E>::value_type;
    using reference = typename detail::traits<E>::reference;
    using const_reference = typename detail::traits<E>::const_reference;
    using rvalue_reference = typename detail::traits<E>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<E>::const_rvalue_reference;
    using pointer = typename detail::traits<E>::pointer;
    using const_pointer = typename detail::traits<E>::const_pointer;
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
    /// Initializes the @ref error_t with a default constructed error value.
    ///
    /// ## Example
    /// ```
    /// error_t<int> err;
    ///
    /// assert(*err == 0);
    /// ```
    constexpr error_t()
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    /// @brief Copy constructor
    ///
    /// @details
    /// Initializes the new @ref error_t such that the contained error value is
    /// copy constructed from the value contained in the source @ref error_t.
    ///
    /// ## Example
    /// ```
    /// error_t<int> err1{42};
    /// error_t<int> err2{err1};
    ///
    /// assert(*err2 == 42);
    /// ```
    constexpr error_t(const error_t&)
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_copy_constructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Move constructor
    ///
    /// @details
    /// Initializes the new @ref error_t such that the contained error value is
    /// move constructed from the value contained in the source @ref error_t.
    ///
    /// ## Example
    /// ```
    /// error_t<int> err1{42};
    /// error_t<int> err2{std::move(err1)};
    ///
    /// assert(*err2 == 42);
    /// ```
    constexpr error_t(error_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_move_constructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Emplacement constructor
    ///
    /// @details
    /// The @ref error_t is initialized such that the contained value is
    /// constructed in place from the forwarded arguments following `inplace`.
    ///
    /// ## Example
    /// ```
    /// error_t<std::string> err{std::in_place, 5, 'a'};
    ///
    /// assert(*err == "aaaaa");
    /// ```
    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : err_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    /// @brief Emplacement constructor with initializer list
    ///
    /// @details
    /// The @ref error_t is initialized such that the contained value is
    /// constructed in place from the forwarded arguments following `inplace`.
    ///
    /// ## Example
    /// ```
    /// error_t<std::vector<int>> err{std::in_place, {1, 2, 3, 4, 5}};
    ///
    /// assert(err->size() == 5);
    /// ```
    template <typename U, typename... Args>
    constexpr error_t([[maybe_unused]] std::in_place_t inplace,
                      std::initializer_list<U> init,
                      Args&&... args)
        : err_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    /// @brief Emplacement constructor
    ///
    /// @details
    /// The @ref error_t is initialized such that the contained value is
    /// constructed in place from the forwarded arguments following `inplace`.
    ///
    /// ## Example
    /// ```
    /// error_t<std::string> err{in_place_error, 5, 'a'};
    ///
    /// assert(*err == "aaaaa");
    /// ```
    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t([[maybe_unused]] in_place_error_t inplace, Args&&... args)
        : err_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    /// @brief Emplacement constructor with initializer list
    ///
    /// @details
    /// The @ref error_t is initialized such that the contained value is
    /// constructed in place from the forwarded arguments following `inplace`.
    ///
    /// ## Example
    /// ```
    /// error_t<std::vector<int>> err{in_place_error, {1, 2, 3, 4, 5}};
    ///
    /// assert(err->size() == 5);
    /// ```
    template <typename U, typename... Args>
    constexpr error_t([[maybe_unused]] in_place_error_t inplace,
                      std::initializer_list<U> init,
                      Args&&... args)
        : err_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    /// @brief Forwarding constructor
    ///
    /// @details
    /// The @ref error_t is initialized such that it contains a value that is
    /// constructed in place with the passed value forwarded to the error
    /// type's constructor.
    ///
    /// This constructor only participates in overload resolution if the
    /// contained error type is constructible from the passed value, and the
    /// passed value is not of types `std::in_place_t`, `sumty::in_place_t`,
    /// `sumty::in_place_error_t`, `sumty::in_place_index_t<1>`, or
    /// `std::in_place_index_t<1>`.
    ///
    /// This constructor is `explicit` if the passed value is not implicitly
    /// convertible to `E`.
    ///
    /// ## Example
    /// ```
    /// float value = 3.14;
    ///
    /// error_t<int> err{value};
    ///
    /// assert(*err == 3);
    /// ```
    template <typename V>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<E>, std::in_place_index_t<0>, V &&> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<V>, std::in_place_index_t<1>>)
    explicit(detail::traits<E>::template is_convertible_from<V&&>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr error_t(V&& err)
        : err_(std::in_place_index<0>, std::forward<V>(err)) {
    }

    /// @brief Destructor
    ///
    /// @details
    /// The contained error value is destroyed in place.
    ///
    /// The destructor is `noexcept` if the contained error type is nothrow
    /// destructible.
    constexpr ~error_t()
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Copy assignment operator
    ///
    /// @details
    /// The contained error value is copy assigned from the contained error
    /// value of the source @ref error_t.
    ///
    /// ## Example
    /// ```
    /// error_t<int> err1{42};
    /// error_t<int> err2{};
    ///
    /// err2 = err1;
    ///
    /// assert(*err2 == 42);
    /// ```
    constexpr error_t& operator=(const error_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    /// @brief Move assignment operator
    ///
    /// @details
    /// The contained error value is move assigned from the contained error
    /// value of the source @ref error_t.
    ///
    /// ## Example
    /// ```
    /// error_t<int> err1{42};
    /// error_t<int> err2{};
    ///
    /// err2 = std::move(err1);
    ///
    /// assert(*err2 == 42);
    /// ```
    constexpr error_t& operator=(error_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<E>::is_nothrow_move_assignable) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Value assignment operator
    ///
    /// @details
    /// Assigns the new value directly to the contained error.
    ///
    /// This function only participates in overload resolution if:
    /// * The source value is not an @ref error_t
    /// * `E` is constructible from `V`
    /// * `E` is assignable from `V`
    ///
    /// ## Example
    /// ```
    /// error_t<long long> err{};
    ///
    /// err = 42;
    ///
    /// assert(*err == 42);
    /// ```
    template <typename V>
#ifndef DOXYGEN
        requires(!std::is_same_v<error_t, std::remove_cvref_t<V>> &&
                 detail::traits<E>::template is_constructible<V> &&
                 detail::traits<E>::template is_assignable<V>)
#endif
    constexpr error_t& operator=(V&& error) {
        err_[index<0>] = std::forward<V>(error);
        return *this;
    }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// error_t<int> err{};
    ///
    /// *err = 42;
    ///
    /// assert(*err == 42);
    /// ```
    [[nodiscard]] constexpr reference operator*() & noexcept { return err_[index<0>]; }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const error_t<int> err{42};
    ///
    /// assert(*err == 42);
    /// ```
    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return err_[index<0>];
    }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// error_t<int> err{42};
    ///
    /// assert(*std::move(err) == 42);
    /// ```
    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(err_)[index<0>];
    }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const error_t<int> err{42};
    ///
    /// assert(*std::move(err) == 42);
    /// ```
    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(err_)[index<0>];
    }

    /// @brief Accesses members of the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// error_t<std::string> err{"hello};
    ///
    /// assert(err->size() == 5);
    /// ```
    constexpr pointer operator->() noexcept { return &err_[index<0>]; }

    /// @brief Accesses members of the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const error_t<std::string> err{"hello};
    ///
    /// assert(err->size() == 5);
    /// ```
    constexpr const_pointer operator->() const noexcept { return &err_[index<0>]; }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// error_t<int> err{};
    ///
    /// err.error() = 42;
    ///
    /// assert(err.error() == 42);
    /// ```
    [[nodiscard]] constexpr reference error() & noexcept { return err_[index<0>]; }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const error_t<int> err{42};
    ///
    /// assert(err.error() == 42);
    /// ```
    [[nodiscard]] constexpr const_reference error() const& noexcept {
        return err_[index<0>];
    }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// error_t<int> err{42};
    ///
    /// assert(std::move(err).error() == 42);
    /// ```
    [[nodiscard]] constexpr rvalue_reference error() && {
        return std::move(err_)[index<0>];
    }

    /// @brief Accesses the contained error value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const error_t<int> err{42};
    ///
    /// assert(std::move(err).error() == 42);
    /// ```
    [[nodiscard]] constexpr const_rvalue_reference error() const&& {
        return std::move(err_)[index<0>];
    }

    /// @brief Swaps two @ref error_t instances
    ///
    /// @details
    /// The two contained values of the @ref error_t instances are directly
    /// swapped, as if by `std::swap`.
    ///
    /// ## Example
    /// ```
    /// error_t<int> err1{42};
    /// error_t<int> err2{24};
    ///
    /// err1.swap(err2);
    ///
    /// assert(*err1 == 24);
    /// assert(*err2 == 42);
    /// ```
    constexpr void swap(error_t& other)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_swappable_v<variant<E>>)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        err_.swap(other.err_);
    }
};

/// @relates error_t
/// @brief Compares the contained values of two @ref error_t instances
///
/// @details
/// ## Example
/// ```
/// error_t<int> err1{42};
/// error_t<int> err2{24};
/// error_t<int> err3{42};
///
/// assert(err1 != err2);
/// assert(err1 == err3);
/// ```
template <typename E, typename V>
#ifndef DOXYGEN
    requires(std::is_void_v<E> == std::is_void_v<V>)
#endif
constexpr bool operator==(const error_t<E>& lhs, const error_t<V>& rhs) {
    if constexpr (std::is_void_v<E>) {
        return true;
    } else {
        return *lhs == *rhs;
    }
}

/// @relates error_t
/// @brief Compares the contained value of an @ref error_t with another value.
///
/// @details
/// ## Example
/// ```
/// error_t<int> err{42};
///
/// assert(err == 42);
/// assert(err != 24);
/// ```
template <typename E, typename V>
#ifndef DOXYGEN
    requires(!std::is_void_v<E>)
#endif
constexpr bool operator==(const error_t<E>& lhs, const V& rhs) {
    return *lhs == rhs;
}

/// @relates error_t
/// @brief Compares the contained value of an @ref error_t with another value.
///
/// @details
/// ## Example
/// ```
/// error_t<int> err{42};
///
/// assert(42 == err);
/// assert(24 != err);
/// ```
template <typename E, typename V>
#ifndef DOXYGEN
    requires(!std::is_void_v<V>)
#endif
constexpr bool operator==(const E& lhs, const error_t<V>& rhs) {
    return lhs == *rhs;
}

/// @relates error_t
/// @brief Swaps two @ref error_t instances
///
/// @details
/// The two contained values of the @ref error_t instances are directly
/// swapped, as if by `std::swap`.
///
/// ## Example
/// ```
/// error_t<int> err1{42};
/// error_t<int> err2{24};
///
/// swap(err1, err2);
///
/// assert(*err1 == 24);
/// assert(*err2 == 42);
/// ```
template <typename E>
constexpr void swap(error_t<E>& a, error_t<E>& b)
#ifndef DOXYGEN
    noexcept(std::is_nothrow_swappable_v<variant<E>>)
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

/// @relates error_t
/// @brief Creates an @ref error_t constructed in place
///
/// @details
/// This function is how most @ref error_t instances should be instantiated.
/// Similar to STL functions like `std::make_unique` and `std::make_optional`,
/// this function is provided a template argument to specify what contained
/// error type should be constructed, and then is passed arguments that are
/// forwarded to the constructor of the contained error type.
///
/// ## Example
/// ```
/// result<void, std::string> ensure_positive(int value) {
///   if (value < 0) {
///       return error<std::string>("value is negative");
///   }
///   return value;
/// }
/// ```
template <typename E, typename... Args>
constexpr error_t<E> error(Args&&... args) {
    return error_t<E>(std::in_place, std::forward<Args>(args)...);
}

/// @relates error_t
/// @brief Creates an @ref error_t constructed in place
///
/// @details
/// This function is how most @ref error_t instances should be instantiated.
/// Similar to STL functions like `std::make_unique` and `std::make_optional`,
/// this function is provided a template argument to specify what contained
/// error type should be constructed, and then is passed arguments that are
/// forwarded to the constructor of the contained error type.
///
/// ## Example
/// ```
/// result<void, std::string> ensure_positive(int value) {
///   if (value < 0) {
///       return error<std::string>("value is negative");
///   }
///   return value;
/// }
/// ```
template <typename E, typename U, typename... Args>
constexpr error_t<E> error(std::initializer_list<U> ilist, Args&&... args) {
    return error_t<E>(std::in_place, ilist, std::forward<Args>(args)...);
}

/// @class ok_t result.hpp <sumty/result.hpp>
/// @brief Wrapper around a non-error value
///
/// @details
/// @ref ok_t is an intermediate type that is generally used to construct a
/// @ref result that contains a non-error value. An @ref ok_t instance
/// implicitly converts into a @ref result, as long as the contained value is
/// implicitly convertible to the value type of the @ref result.
///
/// In most cases, @ref ok_t will not be named directly. Instead, an @ref ok_t
/// will be created using the @ref ok function, as shown in the example below.
/// Also, note that using @ref ok_t or the @ref ok function is often
/// unnecessary, unlike with @ref error_t and the @ref error function. A
/// non-error value does not need to be wrapped to be able to convert to a
/// @ref result.
///
/// ## Example
/// ```
/// result<std::string, void> make_string() {
///     // returns an ok_t<std::string> which gets converted to
///     // a result<std::string, void>
///     return ok<std::string>("hello");
/// }
/// ```
///
/// @tparam T The type of the contained value
template <typename T>
class ok_t {
  private:
    SUMTY_NO_UNIQ_ADDR variant<T> ok_;

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
    /// Initializes the @ref ok_t with a default constructed contained value.
    ///
    /// ## Example
    /// ```
    /// ok_t<int> val;
    ///
    /// assert(*val == 0);
    /// ```
    constexpr ok_t()
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    /// @brief Copy constructor
    ///
    /// @details
    /// Initializes the new @ref ok_t such that the contained value is
    /// copy constructed from the value contained in the source @ref ok_t.
    ///
    /// ## Example
    /// ```
    /// ok_t<int> val1{42};
    /// ok_t<int> val2{val1};
    ///
    /// assert(*val2 == 42);
    /// ```
    constexpr ok_t(const ok_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    /// @brief Move constructor
    ///
    /// @details
    /// Initializes the new @ref ok_t such that the contained value is
    /// move constructed from the value contained in the source @ref ok_t.
    ///
    /// ## Example
    /// ```
    /// ok_t<int> val1{42};
    /// ok_t<int> val2{std::move(val1)};
    ///
    /// assert(*val2 == 42);
    /// ```
    constexpr ok_t(ok_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_constructible) = default;
#else
        ;
#endif

    /// @brief Emplacement constructor
    ///
    /// @details
    /// The @ref ok_t is initialized such that the contained value is
    /// constructed in place from the forwarded arguments following `inplace`.
    ///
    /// ## Example
    /// ```
    /// ok_t<std::string> val{std::in_place, 5, 'a'};
    ///
    /// assert(*val == "aaaaa");
    /// ```
    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr ok_t([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : ok_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    /// @brief Emplacement constructor with initializer list
    ///
    /// @details
    /// The @ref ok_t is initialized such that the contained value is
    /// constructed in place from the forwarded arguments following `inplace`.
    ///
    /// ## Example
    /// ```
    /// ok_t<std::vector<int>> val{std::in_place, {1, 2, 3, 4, 5}};
    ///
    /// assert(val->size() == 5);
    /// ```
    template <typename U, typename... Args>
    constexpr ok_t([[maybe_unused]] std::in_place_t inplace,
                   std::initializer_list<U> init,
                   Args&&... args)
        : ok_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    /// @brief Forwarding constructor
    ///
    /// @details
    /// The @ref ok_t is initialized such that it contains a value that is
    /// constructed in place with the passed value forwarded to the contained
    /// type's constructor.
    ///
    /// This constructor only participates in overload resolution if the
    /// contained type is constructible from the passed value, and the
    /// passed value is not of types `std::in_place_t` or `sumty::in_place_t`.
    ///
    /// This constructor is `explicit` if the passed value is not implicitly
    /// convertible to `T`.
    ///
    /// ## Example
    /// ```
    /// float value = 3.14;
    ///
    /// ok_t<int> val{value};
    ///
    /// assert(*val == 3);
    /// ```
    template <typename U>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<T>, std::in_place_index_t<0>, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(detail::traits<T>::template is_convertible_from<U&&>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr ok_t(U&& value)
        : ok_(std::in_place_index<0>, std::forward<U>(value)) {
    }

    /// @brief Destructor
    ///
    /// @details
    /// The contained value is destroyed in place.
    ///
    /// The destructor is `noexcept` if the contained type is nothrow
    /// destructible.
    constexpr ~ok_t()
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_destructible) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Copy assignment operator
    ///
    /// @details
    /// The contained value is copy assigned from the contained
    /// value of the source @ref ok_t.
    ///
    /// ## Example
    /// ```
    /// ok_t<int> val1{42};
    /// ok_t<int> val2{};
    ///
    /// val2 = val1;
    ///
    /// assert(*val2 == 42);
    /// ```
    constexpr ok_t& operator=(const ok_t&)
#ifndef DOXYGEN
        = default;
#else
        ;
#endif

    /// @brief Move assignment operator
    ///
    /// @details
    /// The contained value is move assigned from the contained
    /// value of the source @ref ok_t.
    ///
    /// ## Example
    /// ```
    /// ok_t<int> val1{42};
    /// ok_t<int> val2{};
    ///
    /// val2 = std::move(val1);
    ///
    /// assert(*val2 == 42);
    /// ```
    constexpr ok_t& operator=(ok_t&&)
#ifndef DOXYGEN
        noexcept(detail::traits<T>::is_nothrow_move_assignable) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    /// @brief Value assignment operator
    ///
    /// @details
    /// Assigns the new value directly to the contained value.
    ///
    /// This function only participates in overload resolution if:
    /// * The source value is not an @ref ok_t
    /// * `T` is constructible from `U`
    /// * `T` is assignable from `U`
    ///
    /// ## Example
    /// ```
    /// ok_t<long long> val{};
    ///
    /// val = 42;
    ///
    /// assert(*val == 42);
    /// ```
    template <typename U>
#ifndef DOXYGEN
        requires(!std::is_same_v<ok_t, std::remove_cvref_t<U>> &&
                 detail::traits<T>::template is_constructible<U> &&
                 detail::traits<T>::template is_assignable<U>)
#endif
    constexpr ok_t& operator=(U&& value) {
        ok_[index<0>] = std::forward<U>(value);
        return *this;
    }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// ok_t<int> val{};
    ///
    /// *val = 42;
    ///
    /// assert(*val == 42);
    /// ```
    [[nodiscard]] constexpr reference operator*() & noexcept { return ok_[index<0>]; }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const ok_t<int> val{42};
    ///
    /// assert(*val == 42);
    /// ```
    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return ok_[index<0>];
    }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// ok_t<int> val{42};
    ///
    /// assert(*std::move(val) == 42);
    /// ```
    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(ok_)[index<0>];
    }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const ok_t<int> val{42};
    ///
    /// assert(*std::move(val) == 42);
    /// ```
    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(ok_)[index<0>];
    }

    /// @brief Accesses members of the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// ok_t<std::string> val{"hello};
    ///
    /// assert(val->size() == 5);
    /// ```
    constexpr pointer operator->() noexcept { return &ok_[index<0>]; }

    /// @brief Accesses members of the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const ok_t<std::string> val{"hello};
    ///
    /// assert(val->size() == 5);
    /// ```
    constexpr const_pointer operator->() const noexcept { return &ok_[index<0>]; }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// ok_t<int> val{};
    ///
    /// val.value() = 42;
    ///
    /// assert(val.value() == 42);
    /// ```
    [[nodiscard]] constexpr reference value() & noexcept { return ok_[index<0>]; }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const ok_t<int> val{42};
    ///
    /// assert(val.value() == 42);
    /// ```
    [[nodiscard]] constexpr const_reference value() const& noexcept {
        return ok_[index<0>];
    }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// ok_t<int> val{42};
    ///
    /// assert(std::move(val).value() == 42);
    /// ```
    [[nodiscard]] constexpr rvalue_reference value() && { return std::move(ok_)[index<0>]; }

    /// @brief Accesses the contained value
    ///
    /// @details
    /// ## Example
    /// ```
    /// const ok_t<int> val{42};
    ///
    /// assert(std::move(val).value() == 42);
    /// ```
    [[nodiscard]] constexpr const_rvalue_reference value() const&& {
        return std::move(ok_)[index<0>];
    }

    /// @brief Swaps two @ref ok_t instances
    ///
    /// @details
    /// The two contained values of the @ref ok_t instances are directly
    /// swapped, as if by `std::swap`.
    ///
    /// ## Example
    /// ```
    /// ok_t<int> val1{42};
    /// ok_t<int> val2{24};
    ///
    /// val1.swap(val2);
    ///
    /// assert(*val1 == 24);
    /// assert(*val2 == 42);
    /// ```
    constexpr void swap(ok_t& other)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_swappable_v<variant<T>>)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        ok_.swap(other.ok_);
    }
};

/// @relates ok_t
/// @brief Compares the contained values of two @ref ok_t instances
///
/// @details
/// ## Example
/// ```
/// ok_t<int> val1{42};
/// ok_t<int> val2{24};
/// ok_t<int> val3{42};
///
/// assert(val1 != val2);
/// assert(val1 == val3);
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(std::is_void_v<T> == std::is_void_v<U>)
#endif
constexpr bool operator==(const ok_t<T>& lhs, const ok_t<U>& rhs) {
    if constexpr (std::is_void_v<T>) {
        return true;
    } else {
        return *lhs == *rhs;
    }
}

/// @relates ok_t
/// @brief Compares the contained value of an @ref ok_t with another value.
///
/// @details
/// ## Example
/// ```
/// ok_t<int> val{42};
///
/// assert(val == 42);
/// assert(val != 24);
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<T>)
#endif
constexpr bool operator==(const ok_t<T>& lhs, const U& rhs) {
    return *lhs == rhs;
}

/// @relates ok_t
/// @brief Compares the contained value of an @ref ok_t with another value.
///
/// @details
/// ## Example
/// ```
/// ok_t<int> val{42};
///
/// assert(42 == val);
/// assert(24 != val);
/// ```
template <typename T, typename U>
#ifndef DOXYGEN
    requires(!std::is_void_v<U>)
#endif
constexpr bool operator==(const T& lhs, const ok_t<U>& rhs) {
    return lhs == *rhs;
}

/// @relates ok_t
/// @brief Swaps two @ref ok_t instances
///
/// @details
/// The two contained values of the @ref ok_t instances are directly
/// swapped, as if by `std::swap`.
///
/// ## Example
/// ```
/// ok_t<int> val1{42};
/// ok_t<int> val2{24};
///
/// swap(val1, val2);
///
/// assert(*val1 == 24);
/// assert(*val2 == 42);
/// ```
template <typename T>
constexpr void swap(ok_t<T>& a, ok_t<T>& b)
#ifndef DOXYGEN
    noexcept(std::is_nothrow_swappable_v<variant<T>>)
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

/// @relates ok_t
/// @brief Creates an @ref ok_t constructed in place
///
/// @details
/// This function is how most @ref ok_t instances should be instantiated.
/// Similar to STL functions like `std::make_unique` and `std::make_optional`,
/// this function is provided a template argument to specify what contained
/// type should be constructed, and then is passed arguments that are
/// forwarded to the constructor of the contained type.
///
/// ## Example
/// ```
/// result<std::string, void> make_string() {
///     // returns an ok_t<std::string> which gets converted to
///     // a result<std::string, void>
///     return ok<std::string>("hello");
/// }
/// ```
template <typename T, typename... Args>
constexpr ok_t<T> ok(Args&&... args) {
    return ok_t<T>{std::in_place, std::forward<Args>(args)...};
}

/// @relates ok_t
/// @brief Creates an @ref ok_t constructed in place
///
/// @details
/// This function is how most @ref ok_t instances should be instantiated.
/// Similar to STL functions like `std::make_unique` and `std::make_optional`,
/// this function is provided a template argument to specify what contained
/// type should be constructed, and then is passed arguments that are
/// forwarded to the constructor of the contained type.
///
/// ## Example
/// ```
/// result<std::string, void> make_string() {
///     // returns an ok_t<std::string> which gets converted to
///     // a result<std::string, void>
///     return ok<std::string>("hello");
/// }
/// ```
template <typename T, typename U, typename... Args>
constexpr ok_t<T> ok(std::initializer_list<U> ilist, Args&&... args) {
    return ok_t<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

template <typename T, typename E>
class result {
  private:
    variant<T, E> res_;

    template <typename, typename>
    friend class result;

  public:
#ifndef DOXYGEN
    using value_type = typename detail::traits<T>::value_type;
    using reference = typename detail::traits<T>::reference;
    using const_reference = typename detail::traits<T>::const_reference;
    using rvalue_reference = typename detail::traits<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::traits<T>::const_rvalue_reference;
    using pointer = typename detail::traits<T>::pointer;
    using const_pointer = typename detail::traits<T>::const_pointer;

    using error_type = typename detail::traits<E>::value_type;
    using error_reference = typename detail::traits<E>::reference;
    using error_const_reference = typename detail::traits<E>::const_reference;
    using error_rvalue_reference = typename detail::traits<E>::rvalue_reference;
    using error_const_rvalue_reference = typename detail::traits<E>::const_rvalue_reference;
    using error_pointer = typename detail::traits<E>::pointer;
    using error_const_pointer = typename detail::traits<E>::const_pointer;
#else
    using value_type = ...;
    using reference = ...;
    using const_reference = ...;
    using rvalue_reference = ...;
    using const_rvalue_reference = ...;
    using pointer = ...;
    using const_pointer = ...;

    using error_type = ...;
    using error_reference = ...;
    using error_const_reference = ...;
    using error_rvalue_reference = ...;
    using error_const_rvalue_reference = ...;
    using error_pointer = ...;
    using error_const_pointer = ...;
#endif

    template <typename U>
    using rebind = result<U, E>;

    template <typename V>
    using rebind_error = result<T, V>;

    // For compatibility with std::expected
    using unexpected_type = error_t<E>;

    constexpr result()
#ifndef DOXYGEN
        noexcept(std::is_nothrow_default_constructible_v<variant<T, E>>)
        requires(std::is_default_constructible_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result(const result&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_copy_constructible_v<variant<T, E>>)
        requires(std::is_copy_constructible_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result(result&&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_move_constructible_v<variant<T, E>>)
        requires(std::is_move_constructible_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result([[maybe_unused]] std::in_place_t inplace, Args&&... args)
        : res_(std::in_place_index<0>, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr result([[maybe_unused]] std::in_place_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : res_(std::in_place_index<0>, init, std::forward<Args>(args)...) {}

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(std::in_place_index_t<0> inplace, Args&&... args)
        : res_(inplace, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr result(std::in_place_index_t<0> inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : res_(inplace, init, std::forward<Args>(args)...) {}

    template <typename... Args>
#ifndef DOXYGEN
    explicit(sizeof...(Args) == 0)
#else
    CONDITONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(in_place_error_t inplace, Args&&... args)
        : res_(inplace, std::forward<Args>(args)...) {
    }

    template <typename U, typename... Args>
    constexpr result(in_place_error_t inplace,
                     std::initializer_list<U> init,
                     Args&&... args)
        : res_(inplace, init, std::forward<Args>(args)...) {}

    template <typename U>
#ifndef DOXYGEN
        requires(std::is_constructible_v<variant<T, E>, std::in_place_index_t<0>, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<0>> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_index_t<1>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 (!std::is_same_v<std::remove_cvref_t<T>, bool> ||
                  !detail::is_result_v<std::remove_cvref_t<U>>))
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(U&& value)
        : res_(std::in_place_index<0>, std::forward<U>(value)) {
    }

    template <typename U>
#ifndef DOXYGEN
    explicit(!detail::traits<T>::template is_convertible_from<U>)
#else
    CONDITONALLY_NOEXCEPT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(ok_t<U> ok)
        : res_(std::in_place_index<0>, *std::move(ok)) {
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr result([[maybe_unused]] ok_t<void> ok) : res_(std::in_place_index<0>) {}

    template <typename U>
#ifndef DOXYGEN
    explicit(!detail::traits<E>::template is_convertible_from<U>)
#else
    CONDITONALLY_NOEXCEPT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(error_t<U> err)
        : res_(std::in_place_index<1>, *std::move(err)) {
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr result([[maybe_unused]] error_t<void> err) : res_(std::in_place_index<1>) {}

    template <typename U, typename V>
#ifndef DOXYGEN
        requires(((std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<0>,
                                          typename detail::traits<U>::const_reference>) &&
                 ((std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<1>,
                                          typename detail::traits<E>::const_reference>))
    explicit((!std::is_void_v<U> && !detail::traits<T>::template is_convertible_from<U>) ||
             (!std::is_void_v<V> && !detail::traits<E>::template is_convertible_from<V>))
#else
    CONDITIONALLY_EXPLICIT
#endif
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr result(const result<U, V>& other)
        : res_(detail::uninit) {
        other.res_.visit_informed([this](auto&& value, auto info) {
            res_.template uninit_emplace<info.index>(value);
        });
    }

    template <typename U, typename V>
#ifndef DOXYGEN
        requires(((std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<0>,
                                          typename detail::traits<U>::rvalue_reference>) &&
                 ((std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                  std::is_constructible_v<variant<T, E>,
                                          std::in_place_index_t<1>,
                                          typename detail::traits<E>::rvalue_reference>))
    explicit((!std::is_void_v<U> && !detail::traits<T>::template is_convertible_from<U>) ||
             (!std::is_void_v<V> && !detail::traits<E>::template is_convertible_from<V>))
#else
    CONDITIONALLY_EXPLICIT
#endif
        // clang-format off
        // NOLINTNEXTLINE(hicpp-explicit-conversions,cppcoreguidelines-rvalue-reference-param-not-moved)
        constexpr result(result<U, V>&& other)
        // clang-format on
        : res_(detail::uninit) {
        other.res_.visit_informed([this](auto&& value, auto info) {
            res_.template uninit_emplace<info.index>(info.forward(value));
        });
    }

    constexpr ~result()
#ifndef DOXYGEN
        noexcept(std::is_nothrow_destructible_v<variant<T, E>>) = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result& operator=(const result&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_copy_assignable_v<variant<T, E>>)
        requires(std::is_copy_assignable_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    constexpr result& operator=(result&&)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_move_assignable_v<variant<T, E>>)
        requires(std::is_move_assignable_v<variant<T, E>>)
    = default;
#else
        CONDITIONALLY_NOEXCEPT;
#endif

    template <typename U>
#ifndef DOXYGEN
        requires(!std::is_same_v<result, std::remove_cvref_t<U>> &&
                 !detail::is_error_v<std::remove_cvref_t<U>> &&
                 !detail::is_ok_v<std::remove_cvref_t<U>> &&
                 detail::traits<T>::template is_constructible<U> &&
                 detail::traits<T>::template is_assignable<U>)
#endif
    constexpr result<T, E>& operator=(U&& value) {
        if (res_.index() == 0) {
            res_[index<0>] = std::forward<U>(value);
        } else {
            res_.template emplace<0>(std::forward<U>(value));
        }
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires((detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::const_reference> &&
                  detail::traits<T>::template is_assignable<
                      typename detail::traits<U>::const_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
#endif
    constexpr result& operator=(const ok_t<U>& value) {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<U>) {
                res_.template emplace<0>();
            } else {
                res_.template emplace<0>(*value);
            }
        } else {
            if constexpr (std::is_void_v<U>) {
                if constexpr (!std::is_void_v<T>) { res_[index<0>] = T{}; }
            } else {
                if constexpr (!std::is_void_v<T>) {
                    if constexpr (std::is_lvalue_reference_v<T>) {
                        res_.template emplace<0>(*value);
                    } else {
                        res_[index<0>] = *value;
                    }
                }
            }
        }
        return *this;
    }

    template <typename U>
#ifndef DOXYGEN
        requires((detail::traits<T>::template is_constructible<
                      typename detail::traits<U>::rvalue_reference> &&
                  detail::traits<T>::template is_assignable<
                      typename detail::traits<U>::rvalue_reference>) ||
                 (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
                 std::is_void_v<T>)
#endif
    constexpr result& operator=(ok_t<U>&& value) {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<U>) {
                res_.template emplace<0>();
            } else {
                res_.template emplace<0>(*std::move(value));
            }
        } else {
            if constexpr (std::is_void_v<U>) {
                if constexpr (!std::is_void_v<T>) { res_[index<0>] = T{}; }
            } else {
                if constexpr (!std::is_void_v<T>) {
                    if constexpr (std::is_lvalue_reference_v<T>) {
                        res_.template emplace<0>(*std::move(value));
                    } else {
                        res_[index<0>] = *std::move(value);
                    }
                }
            }
        }
        return *this;
    }

    template <typename V>
#ifndef DOXYGEN
        requires((detail::traits<E>::template is_constructible<
                      typename detail::traits<V>::const_reference> &&
                  detail::traits<E>::template is_assignable<
                      typename detail::traits<V>::const_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
#endif
    constexpr result& operator=(const error_t<V>& error) {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<V>) {
                res_.template emplace<1>();
            } else {
                res_.template emplace<1>(*error);
            }
        } else {
            if constexpr (std::is_void_v<V>) {
                if constexpr (!std::is_void_v<E>) { res_[index<1>] = E{}; }
            } else {
                if constexpr (!std::is_void_v<E>) {
                    if constexpr (std::is_lvalue_reference_v<E>) {
                        res_.template emplace<1>(*error);
                    } else {
                        res_[index<1>] = *error;
                    }
                }
            }
        }
        return *this;
    }

    template <typename V>
#ifndef DOXYGEN
        requires((detail::traits<E>::template is_constructible<
                      typename detail::traits<V>::rvalue_reference> &&
                  detail::traits<E>::template is_assignable<
                      typename detail::traits<V>::rvalue_reference>) ||
                 (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
                 std::is_void_v<E>)
#endif
    constexpr result& operator=(error_t<V>&& error) {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<V>) {
                res_.template emplace<1>();
            } else {
                res_.template emplace<1>(*std::move(error));
            }
        } else {
            if constexpr (std::is_void_v<V>) {
                if constexpr (!std::is_void_v<E>) { res_[index<1>] = E{}; }
            } else {
                if constexpr (!std::is_void_v<E>) {
                    if constexpr (std::is_lvalue_reference_v<E>) {
                        res_.template emplace<1>(*std::move(error));
                    } else {
                        res_[index<1>] = *std::move(error);
                    }
                }
            }
        }
        return *this;
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator bool() const noexcept { return res_.index() == 0; }

    [[nodiscard]] constexpr bool has_value() const noexcept { return res_.index() == 0; }

    [[nodiscard]] constexpr reference operator*() & noexcept { return res_[index<0>]; }

    [[nodiscard]] constexpr const_reference operator*() const& noexcept {
        return res_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference operator*() && {
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr const_rvalue_reference operator*() const&& {
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr pointer operator->() noexcept { return &res_[index<0>]; }

    [[nodiscard]] constexpr const_pointer operator->() const noexcept {
        return &res_[index<0>];
    }

    [[nodiscard]] constexpr reference value() & {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(res_[index<1>]);
            }
        }
        return res_[index<0>];
    }

    [[nodiscard]] constexpr const_reference value() const& {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(res_[index<1>]);
            }
        }
        return res_[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference value() && {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(std::move(res_)[index<1>]);
            }
        }
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr rvalue_reference value() const&& {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                throw bad_result_access<void>();
            } else {
                throw bad_result_access<std::remove_cvref_t<E>>(std::move(res_)[index<1>]);
            }
        }
        return std::move(res_)[index<0>];
    }

    [[nodiscard]] constexpr error_reference error() & noexcept { return res_[index<1>]; }

    [[nodiscard]] constexpr error_const_reference error() const& noexcept {
        return res_[index<1>];
    }

    [[nodiscard]] constexpr error_rvalue_reference error() && {
        return std::move(res_)[index<1>];
    }

    [[nodiscard]] constexpr error_const_rvalue_reference error() const&& {
        return std::move(res_)[index<1>];
    }

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) const& {
        if (res_.index() == 0) {
            return res_[index<0>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    [[nodiscard]] constexpr value_type value_or(U&& default_value) && {
        if (res_.index() == 0) {
            return std::move(res_)[index<0>];
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    [[nodiscard]] constexpr value_type value_or() const& {
        if (res_.index() == 0) {
            return res_[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    [[nodiscard]] constexpr value_type value_or() && {
        if (res_.index() == 0) {
            return std::move(res_)[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                return;
            } else {
                return value_type{};
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr value_type value_or_else(F&& f) const& {
        if (res_.index() == 0) {
            return res_[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return std::invoke(std::forward<F>(f));
            }
        }
    }

    template <typename F>
    [[nodiscard]] constexpr value_type value_or_else(F&& f) && {
        if (res_.index() == 0) {
            return std::move(res_)[index<0>];
        } else {
            if constexpr (std::is_void_v<T>) {
                std::invoke(std::forward<F>(f));
                return;
            } else {
                return std::invoke(std::forward<F>(f));
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) & {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error,
                                                                        res_[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), res_[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F, reference>>{
                        in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F, reference>>{
                        in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const& {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error,
                                                                        res_[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), res_[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{
                        in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{
                        in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) && {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{
                        in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const&& {
        if constexpr (std::is_void_v<T>) {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f));
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{in_place_error};
                } else {
                    return std::remove_cvref_t<std::invoke_result_t<F>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            if (res_.index() == 0) {
                return std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
            } else {
                if constexpr (std::is_void_v<E>) {
                    return std::remove_cvref_t<
                        std::invoke_result_t<F, const_rvalue_reference>>{in_place_error};
                } else {
                    return std::remove_cvref_t<
                        std::invoke_result_t<F, const_rvalue_reference>>{
                        in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place, std::invoke(std::forward<F>(f), res_[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, const_reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place, std::invoke(std::forward<F>(f), res_[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, res_[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, rvalue_reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
        if constexpr (std::is_void_v<T>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, const_rvalue_reference>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
                    return result<res_t, E>{std::in_place};
                } else {
                    return result<res_t, E>{
                        std::in_place,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<0>])};
                }
            } else {
                if constexpr (std::is_void_v<E>) {
                    return result<res_t, E>{in_place_error};
                } else {
                    return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
                }
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
    constexpr auto or_else(F&& f) const& {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::remove_cvref_t<std::invoke_result_t<F>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, res_[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f));
            }
        } else {
            using res_t =
                std::remove_cvref_t<std::invoke_result_t<F, error_const_reference>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, res_[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f), res_[index<1>]);
            }
        }
    }

    template <typename F>
    constexpr auto or_else(F&& f) && {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::remove_cvref_t<std::invoke_result_t<F>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, std::move(res_)[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f));
            }
        } else {
            using res_t =
                std::remove_cvref_t<std::invoke_result_t<F, error_rvalue_reference>>;
            if (res_.index() == 0) {
                if constexpr (std::is_void_v<T>) {
                    return res_t{};
                } else {
                    return res_t{std::in_place, std::move(res_)[index<0>]};
                }
            } else {
                return std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) & {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error, std::invoke(std::forward<F>(f), res_[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) const& {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_const_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), res_[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error, std::invoke(std::forward<F>(f), res_[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, res_[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) && {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_rvalue_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) const&& {
        if constexpr (std::is_void_v<E>) {
            using res_t = std::invoke_result_t<F>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f));
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{in_place_error,
                                            std::invoke(std::forward<F>(f))};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        } else {
            using res_t = std::invoke_result_t<F, error_const_rvalue_reference>;
            if (res_.index() != 0) {
                if constexpr (std::is_void_v<res_t>) {
                    std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
                    return result<T, res_t>{in_place_error};
                } else {
                    return result<T, res_t>{
                        in_place_error,
                        std::invoke(std::forward<F>(f), std::move(res_)[index<1>])};
                }
            } else {
                if constexpr (std::is_void_v<T>) {
                    return result<T, res_t>{std::in_place};
                } else {
                    return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
                }
            }
        }
    }

    template <typename F>
    constexpr auto map_error(F&& f) & {
        return transform_error(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map_error(F&& f) const& {
        return transform_error(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map_error(F&& f) && {
        return std::move(*this).transform_error(std::forward<F>(f));
    }

    template <typename F>
    constexpr auto map_error(F&& f) const&& {
        return std::move(*this).transform_error(std::forward<F>(f));
    }

    [[nodiscard]] constexpr result<reference, error_reference> ref() noexcept {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return result<reference, error_reference>{std::in_place};
            } else {
                return result<reference, error_reference>{std::in_place, **this};
            }
        } else {
            if constexpr (std::is_void_v<E>) {
                return result<reference, error_reference>{in_place_error};
            } else {
                return result<reference, error_reference>{in_place_error, error()};
            }
        }
    }

    [[nodiscard]] constexpr result<const_reference, error_const_reference> ref()
        const noexcept {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return result<const_reference, error_const_reference>{std::in_place};
            } else {
                return result<const_reference, error_const_reference>{std::in_place,
                                                                      **this};
            }
        } else {
            if constexpr (std::is_void_v<E>) {
                return result<const_reference, error_const_reference>{in_place_error};
            } else {
                return result<const_reference, error_const_reference>{in_place_error,
                                                                      error()};
            }
        }
    }

    [[nodiscard]] constexpr result<const_reference, error_const_reference> cref()
        const noexcept {
        return ref();
    }

    [[nodiscard]] constexpr option<T> or_none() const& noexcept {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return option<T>{std::in_place};
            } else {
                return option<T>{std::in_place, res_[index<0>]};
            }
        } else {
            return option<T>{};
        }
    }

    [[nodiscard]] constexpr option<T> or_none() && {
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<T>) {
                return option<T>{std::in_place};
            } else {
                return option<T>{std::in_place, std::move(res_)[index<0>]};
            }
        } else {
            return option<T>{};
        }
    }

    [[nodiscard]] constexpr option<E> error_or_none() const& noexcept {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                return option<E>{std::in_place};
            } else {
                return option<E>{std::in_place, res_[index<1>]};
            }
        } else {
            return option<E>{};
        }
    }

    [[nodiscard]] constexpr option<E> error_or_none() && {
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<E>) {
                return option<E>{std::in_place};
            } else {
                return option<E>{std::in_place, std::move(res_)[index<1>]};
            }
        } else {
            return option<E>{};
        }
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        return res_.template emplace<0>(std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr reference emplace(std::initializer_list<U> ilist, Args&&... args) {
        return res_.template emplace<0>(ilist, std::forward<Args>(args)...);
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) & {
        return res_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) const& {
        return res_.visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) && {
        return std::move(res_).visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit(V&& visitor) const&& {
        return std::move(res_).visit(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) & {
        return res_.visit_informed(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) const& {
        return res_.visit_informed(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) && {
        return std::move(res_).visit_informed(std::forward<V>(visitor));
    }

    template <typename V>
    constexpr
#ifndef DOXYGEN
        decltype(auto)
#else
        DEDUCED
#endif
        visit_informed(V&& visitor) const&& {
        return std::move(res_).visit_informed(std::forward<V>(visitor));
    }

    constexpr void swap(result& other)
#ifndef DOXYGEN
        noexcept(std::is_nothrow_swappable_v<variant<T, E>>)
#else
        CONDITIONALLY_NOEXCEPT
#endif
    {
        res_.swap(other.res_);
    }
};

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::reference
#else
    REFERENCE
#endif
    get(result<T, E>& res) {
    return get<IDX>(res.res_);
}

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const result<T, E>& res) {
    return get<IDX>(res.res_);
}

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    get(result<T, E>&& res) {
    return get<IDX>(std::move(res.res_));
}

/// @relates result
template <size_t IDX, typename T, typename E>
constexpr
#ifndef DOXYGEN
    typename detail::traits<detail::select_t<IDX, T, E>>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const result<T, E>&& res) {
    return get<IDX>(std::move(res.res_));
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::reference
#else
    REFERENCE
#endif
    get(result<T, E>& res) {
    return get<U>(res.res_);
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::const_reference
#else
    CONST_REFERENCE
#endif
    get(const result<T, E>& res) {
    return get<U>(res.res_);
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::rvalue_reference
#else
    RVALUE_REFERENCE
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    get(result<T, E>&& res) {
    return get<U>(std::move(res.res_));
}

/// @relates result
template <typename U, typename T, typename E>
#ifndef DOXYGEN
    requires(std::is_same_v<U, T> || std::is_same_v<U, E>)
#endif
constexpr
#ifndef DOXYGEN
    typename detail::traits<U>::const_rvalue_reference
#else
    CONST_RVALUE_REFERENCE
#endif
    get(const result<T, E>&& res) {
    return get<U>(std::move(res.res_));
}

/// @relates result
template <typename T, typename E, typename U, typename V>
#ifndef DOXYGEN
    requires(std::is_void_v<T> == std::is_void_v<U> &&
             std::is_void_v<E> == std::is_void_v<V>)
#endif
constexpr bool operator==(const result<T, E>& lhs, const result<U, V>& rhs) {
    if (lhs.has_value()) {
        if constexpr (std::is_void_v<T>) {
            return rhs.has_value();
        } else {
            if (rhs.has_value()) {
                return *lhs == *rhs;
            } else {
                return false;
            }
        }
    } else {
        if constexpr (std::is_void_v<E>) {
            return !rhs.has_value();
        } else {
            if (rhs.has_value()) {
                return false;
            } else {
                return lhs.error() == rhs.error();
            }
        }
    }
}

/// @relates result
template <typename T, typename E, typename U>
constexpr bool operator==(const result<T, E>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

/// @relates result
template <typename T, typename E, typename V>
constexpr bool operator==(const result<T, E>& lhs, const error_t<V>& rhs) {
    return !lhs.has_value() && lhs.error() == rhs;
}

/// @relates result
template <typename T, typename E>
constexpr void swap(result<T, E>& a, result<T, E>& b)
#ifndef DOXYGEN
    noexcept(std::is_nothrow_swappable_v<variant<T, E>>)
#else
    CONDITIONALLY_NOEXCEPT
#endif
{
    a.swap(b);
}

} // namespace sumty

#endif
