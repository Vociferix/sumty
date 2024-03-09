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

#ifndef SUMTY_VARIANT_HPP
#define SUMTY_VARIANT_HPP

#include "sumty/detail/fwd.hpp"    // IWYU pragma: export
#include "sumty/detail/traits.hpp" // IWYU pragma: export
#include "sumty/detail/utils.hpp"
#include "sumty/detail/variant_impl.hpp"
#include "sumty/utils.hpp"

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace sumty {

/// @class variant variant.hpp <sumty/variant.hpp>
/// @brief General discriminated union
///
/// @details
/// @ref variant is the fundamental sum type of sumty. @ref option and @ref
/// result are defined in terms of @ref variant.
///
/// @ref variant is very similar to `std::variant`, but it does have some
/// key differences. Most notably, @ref variant does not support type-based
/// alternative selection unless all alternatives have a distinct type. For
/// example, `std::variant` will allow the code shown below, but @ref variant
/// will not.
///
/// ```cpp
/// std::variant<int, int> v = 42;
/// ```
///
/// Any such functionality from `std::variant` that is similarly ambiguous to
/// the reader is disallowed by @ref variant. Otherwise, the interface provided
/// by @ref variant is identical to `std::variant`, but with some additional
/// functions available.
///
/// Another key difference from `std::variant` is that @ref variant allows
/// alternatives to have `void` and references (both lvalue and rvalue) as
/// types. When using an lvalue reference as an alternative type, copy and move
/// semantics applied to the @ref variant are the same as a raw pointer (both
/// trivial), instead of immovability of lvalue references.
///
/// A more subtle, but important, difference from `std::variant` is that @ref
/// variant makes size optimizations for several special cases. These special
/// cases revolve around alternatives with types that are lvalue references,
/// `void`, and empty types (such as `std::monostate`).
///
/// `void` and empty types need not occupy any memory at runtime, despite the
/// fact that `sizeof(<empty-type>) == 1` in C++. A @ref variant consisting
/// entirely of alternatives with types that are `void` or empty types only
/// takes up enough space to store a discriminant, which is always the smallest
/// possible integral type necessary to store the value `N - 1`, where `N` is
/// the number of alternatives. Here are some examples:
///
/// ```cpp
/// struct empty_t {};
///
/// assert(std::is_empty_v<variant<void>>);
///
/// assert(std::is_empty_v<variant<empty_t>>);
///
/// assert(sizeof(variant<void, empty_t>) == sizeof(bool));
///
/// assert(sizeof(variant<empty_t, ...>) // repeated 256 times
///         == sizeof(uint8_t));
///
/// assert(sizeof(variant<empty_t, ...>) // repeated 257 times
///         == sizeof(uint16_t));
/// ```
///
/// lvalue references have the invariant that the underlying pointer is not
/// null. This means in cases where there are only two alternatives, with one
/// being an lvalue reference and the other being `void` or an empty type, null
/// and non-null can be used as the discriminanting values instead of having a
/// separate discriminant. Here are some more examples:
///
/// ```cpp
/// struct empty_t {};
///
/// assert(sizeof(variant<int&, void>) == sizeof(int*));
///
/// assert(sizeof(variant<empty_t, int&>) == sizeof(int*));
/// ```
///
/// @tparam T @ref variant alternative types
template <typename... T>
class variant {
  private:
    SUMTY_NO_UNIQ_ADDR detail::variant_impl<void, T...> data_;

    template <size_t IDX, typename U>
    [[nodiscard]] constexpr bool holds_alt_impl() const noexcept;

  public:
    /// @brief Default constructor
    ///
    /// @details
    /// Initializes the @ref variant such that it contains a default
    /// constructed value of the first (index 0) alternative.
    ///
    /// The first alternative *must* be default constructible for this
    /// constructor to participate in overload resoltuion, but no other
    /// alternatives need be default constructible.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool, int> v;
    ///
    /// assert(holds_alternative<int>(v));
    ///
    /// assert(v.index() == 0);
    ///
    /// assert(get<0>(v) == int{});
    /// ```
    constexpr variant() noexcept(
        detail::traits<detail::first_t<T...>>::is_nothrow_default_constructible)
        requires(detail::traits<detail::first_t<T...>>::is_default_constructible)
    = default;

    /// @brief Copy constructor
    ///
    /// @details
    /// A new @ref variant is initialized such that it contains a copy
    /// constructed instance of the alternative contained in the source @ref
    /// variant. If the source @ref variant has more than one alternative of
    /// the same type, the new @ref variant will contain the alternative of the
    /// same index.
    ///
    /// All alternative types *must* be copy constructible for this constructor
    /// to participate in overload resolution.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool, int> v1{std::in_place_index<2>, 42};
    ///
    /// variant<int, bool, int> v2{v1};
    ///
    /// assert(holds_alternative<int>(v2));
    ///
    /// assert(v2.index() == 2);
    ///
    /// assert(get<2>(v2) == 42);
    /// ```
    constexpr variant(const variant&)
        requires(true && ... && detail::traits<T>::is_copy_constructible)
    = default;

    /// @brief Move constructor
    ///
    /// @details
    /// A new @ref variant is initialized such that it contains a move
    /// constructed instance of the alternative contained in the source @ref
    /// variant. If the source @ref vairant has more than one alternative of
    /// the same type, the new @ref variant will contain the alternative of the
    /// same index.
    ///
    /// All alternative types *must* be move constructible for this constructor
    /// to participate in overload resolution.
    ///
    /// The source variant will continue to contain an instance of the same
    /// alternative, but the value of the alternative after being moved depends
    /// on the move constructor of the type. In general, moved values are said
    /// to be in a valid, but unspecified, state.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool, int> v1{std::in_place_index<2>, 42};
    ///
    /// variant<int, bool, int> v2{std::move(v1)};
    ///
    /// assert(holds_alternative<int>(v2));
    ///
    /// assert(v2.index() == 2);
    ///
    /// assert(get<2>(v2) == 42);
    /// ```
    constexpr variant(variant&&) noexcept(
        (true && ... && detail::traits<T>::is_nothrow_move_constructible))
        requires(true && ... && detail::traits<T>::is_move_constructible)
    = default;

    /// @brief Index emplacement constructor
    ///
    /// @details
    /// A new @ref variant is initialized such that it contains a newly
    /// constructed instance of the alternative with the specified index.
    /// The arguments following `inplace` are forwarded directly to the
    /// constructor of the alternative type.
    ///
    /// Given that `U` is the type of the alternative at the specified index,
    /// this constructor is always valid as long as `U` is constructible with
    /// the arguments, `std::forward<Args>(args)...`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::string> v{std::in_place_index<1>, 5, 'a'};
    ///
    /// assert(holds_alternative<std::string>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v) == "aaaaa");
    /// ```
    ///
    /// @param inplace Constructor tag that specifies the alternative index.
    /// @param args Arguments used to construct the alternative.
    template <size_t IDX, typename... Args>
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr variant(std::in_place_index_t<IDX> inplace, Args&&... args);

    /// @brief Index emplacement constructor with `std::initializer_list`
    ///
    /// @details
    /// A new @ref variant is initialized such that it contains a newly
    /// constructed instance of the alternative with the specified index.
    /// The arguments following `inplace` are forwarded directly to the
    /// constructor of the alternative type.
    ///
    /// Given that `U` is the type of the alternative at the specified index,
    /// this constructor is always valid as long as `U` is constructible with
    /// the arguments, `init, std::forward<Args>(args)...`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::vector<int>> v{
    ///         std::in_place_index<1>,
    ///         {1, 2, 3, 4, 5}};
    ///
    /// assert(holds_alternative<std::vector<int>>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v).size() == 5);
    /// ```
    ///
    /// @param inplace Constructor tag that specifies the alternative index.
    /// @param init Initializer list forwarded to the alternative constructor
    /// @param args Additional arguments used to construct the alternative.
    template <size_t IDX, typename U, typename... Args>
    constexpr variant(std::in_place_index_t<IDX> inplace,
                      std::initializer_list<U> init,
                      Args&&... args);

    /// @brief Type emplacement constructor
    ///
    /// @details
    /// A new @ref variant is initialized such that it contains a newly
    /// constructed instance of the alternative with the specified type.
    /// The arguments following `inplace` are forwarded directory to the
    /// constructor of the alternative type.
    ///
    /// This constructor only participates in overload resolution if the
    /// alternative type, `U`, is unique among all alternatives of the @ref
    /// variant. That is, `variant<A, B, A>`, cannot use this constructor to
    /// initialize an alternative of type `A`, but it could use this
    /// constructor to initialize the alternative of type `B`. This constraint
    /// is imposed to prevent ambiguity to the reader. Use the index based
    /// emplacement constructor instead to initialize the alternative of type
    /// `A`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::string> v{
    ///         std::in_place_type<std::string>,
    ///         5, 'a'};
    ///
    /// assert(holds_alternative<std::string>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v) == "aaaaa");
    /// ```
    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    explicit(sizeof...(Args) == 0)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr variant(std::in_place_type_t<U> inplace, Args&&... args);

    /// @brief Type emplacement constructor with `std::initializer_list`
    ///
    /// @details
    /// A new @ref variant is initialized such that it contains a newly
    /// constructed instance of the alternative with the specified type.
    /// The arguments following `inplace` are forwarded directory to the
    /// constructor of the alternative type.
    ///
    /// This constructor only participates in overload resolution if the
    /// alternative type, `U`, is unique among all alternatives of the @ref
    /// variant. That is, `variant<A, B, A>`, cannot use this constructor to
    /// initialize an alternative of type `A`, but it could use this
    /// constructor to initialize the alternative of type `B`. This constraint
    /// is imposed to prevent ambiguity to the reader. Use the index based
    /// emplacement constructor instead to initialize the alternative of type
    /// `A`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::vector<int>> v{
    ///         std::in_place_type<std::string>,
    ///         {1, 2, 3, 4, 5}};
    ///
    /// assert(holds_alternative<std::vector<int>>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v).size() == 5);
    /// ```
    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr variant(std::in_place_type_t<U> inplace,
                      std::initializer_list<V> init,
                      Args&&... args);

    /// @brief Destructor
    ///
    /// @details
    /// The contained alternative of the @ref variant will is destroyed in
    /// place.
    constexpr ~variant() noexcept((true && ... &&
                                   detail::traits<T>::is_nothrow_destructible)) = default;

    /// @brief Copy assignment operator
    ///
    /// @details
    /// Copy assignment of a @ref variant can take one of two possible code
    /// paths.
    ///
    /// If the source and destination @ref variant hold the same alternative
    /// (same index), the alternative value is copied via copy assignment.
    ///
    /// Otherwise, if the source and destination hold different alternatives
    /// (different indices, but possibly the same type), the alternative of
    /// the destination @ref variant is destroyed in place, and the new
    /// alternative is copy constructed.
    ///
    /// All alternatives *must* be both copy assignable and copy constructible
    /// for this function to participate in overload resolution.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool> v1{std::in_place_index<0>, 42};
    ///
    /// variant<int, bool> v2{std::in_place_index<1>, true};
    ///
    /// v1 = v2;
    ///
    /// assert(holds_alternative<bool>(v1));
    ///
    /// assert(v1.index() == 1);
    ///
    /// assert(get<1>(v1) == true);
    /// ```
    constexpr variant& operator=(const variant& rhs)
        requires(true && ... &&
                 (detail::traits<T>::is_copy_assignable &&
                  detail::traits<T>::is_copy_constructible))
    = default;

    /// @brief Move assignment operator
    ///
    /// @details
    /// Move assignment of a @ref variant can take one of two possible code
    /// paths.
    ///
    /// If the source and destination @ref variant hold the same alternative
    /// (same index), the alternative value is moved from the source to the
    /// destination via move assignment.
    ///
    /// Otherwise, if the source and destination hold different alternatives
    /// (different indices, but possibly the same type), the alternative of the
    /// destination @ref variant is destroyed in place, and the new alternative
    /// is move constructed.
    ///
    /// The source @ref variant will still contain the same alternative, but
    /// the value of the alternative depends on the move assignment or move
    /// constructor of the alternative's type. In general, moved values are
    /// said to be in a valid, but unspecified, state.
    ///
    /// All alternatives *must* be both move assignable and move constructible
    /// for this function to participate in overload resolution.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool> v1{std::in_place_index<0>, 42};
    ///
    /// variant<int, bool> v2{std::in_place_index<1>, true};
    ///
    /// v1 = std::move(v2);
    ///
    /// assert(holds_alternative<bool>(v1));
    ///
    /// assert(v1.index() == 1);
    ///
    /// assert(get<1>(v1) == true);
    /// ```
    constexpr variant& operator=(variant&& rhs) noexcept(
        (true && ... &&
         (detail::traits<T>::is_nothrow_move_assignable &&
          detail::traits<T>::is_nothrow_destructible &&
          detail::traits<T>::is_nothrow_move_constructible)))
        requires(true && ... &&
                 (detail::traits<T>::is_move_assignable &&
                  detail::traits<T>::is_move_constructible))
    = default;

    /// @brief Gets the index of the contained alternative
    ///
    /// @details
    /// The set of alternatives of a @ref variant has a zero-based index based
    /// on the order in which they are specified in the @ref variant template
    /// arguments.
    ///
    /// This index is the normalized discriminant of the @ref variant. The
    /// discriminant may be represented differently internally, depending on
    /// the alternative types, so this function normalizes the discriminant by
    /// converting it to a zero-based index in order to provide a common
    /// interface for all @variant instantiations.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool, float&, std::string, void> v{
    ///         std::in_place_index<3>, "hello"};
    ///
    /// assert(v.index() == 3);
    /// ```
    ///
    /// @return The index of the contained alternative.
    [[nodiscard]] constexpr size_t index() const noexcept;

    /// @brief Constructs a new alternative in place by index
    ///
    /// @details
    /// This function destroys the alternative that the @ref variant contains
    /// before the call, and constructs a new alternative with the specified
    /// index in place.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::string> v;
    ///
    /// v.emplace<1>(5, 'a');
    ///
    /// assert(holds_alternative<std::string>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v) == "aaaaa");
    /// ```
    ///
    /// @param args Constructor arguments forwarded to the new alternative
    /// @return A reference to the new alternative, if applicable
    template <size_t I, typename... Args>
    constexpr typename detail::traits<detail::select_t<I, T...>>::reference emplace(
        Args&&... args);

    /// @brief Constructs a new alternative in place by index
    ///
    /// @details
    /// This function destroys the alternative that the @ref variant contains
    /// before the call, and constructs a new alternative with the specified
    /// index in place.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::vector<int>> v;
    ///
    /// v.emplace<1>({1, 2, 3, 4, 5});
    ///
    /// assert(holds_alternative<std::vector<int>>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v).size() == 5);
    /// ```
    ///
    /// @param ilist Initializer list forward to the new alternative
    /// @param args Constructor arguments forwarded to the new alternative
    /// @return A reference to the new alternative, if applicable
    template <size_t I, typename U, typename... Args>
    constexpr typename detail::traits<detail::select_t<I, T...>>::reference emplace(
        std::initializer_list<U> ilist,
        Args&&... args);

    /// @brief Constructs a new alternative in place by type
    ///
    /// @details
    /// This function destroy the alternative that the @ref variant contains
    /// before the call, and constructs a new alternative with the specified
    /// type in place.
    ///
    /// This function only participates in overload resolution if the type,
    /// `U`, is unique among all the alternative types of the @ref variant.
    /// That is, `variant<A, B, A>` cannot use this function to emplace an
    /// alternative of type `A`, but it can use this function to emplace an
    /// alternative of type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::string> v;
    ///
    /// v.emplace<std::string>(5, 'a');
    ///
    /// assert(holds_alternative<std::string>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v) == "aaaaa");
    /// ```
    ///
    /// @param args Constructor arguments forwarded to the new alternative
    /// @return A reference to the new alternative, if applicable
    template <typename U, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr typename detail::traits<U>::reference emplace(Args&&... args);

    /// @brief Constructs a new alternative in place by type
    ///
    /// @details
    /// This function destroy the alternative that the @ref variant contains
    /// before the call, and constructs a new alternative with the specified
    /// type in place.
    ///
    /// This function only participates in overload resolution if the type,
    /// `U`, is unique among all the alternative types of the @ref variant.
    /// That is, `variant<A, B, A>` cannot use this function to emplace an
    /// alternative of type `A`, but it can use this function to emplace an
    /// alternative of type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, std::vector<int>> v;
    ///
    /// v.emplace<std::vector<int>>({1, 2, 3, 4, 5});
    ///
    /// assert(holds_alternative<std::vector<int>>(v));
    ///
    /// assert(v.index() == 1);
    ///
    /// assert(get<1>(v).size() == 5);
    /// ```
    ///
    /// @param ilist Initializer list forward to the new alternative
    /// @param args Constructor arguments forwarded to the new alternative
    /// @return A reference to the new alternative, if applicable
    template <typename U, typename V, typename... Args>
        requires(detail::is_unique_v<U, T...>)
    constexpr typename detail::traits<U>::reference emplace(std::initializer_list<V> ilist,
                                                            Args&&... args);

    /// @brief Alternative access operator by index
    ///
    /// @details
    /// This function allows accessing alternatives by index using the square
    /// bracket operator. Because the index must be a compile time value,
    /// instead of passing the index directly, the index is provided as an
    /// instance of @ref index_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing an
    /// index that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v[index<1>] == 42);
    ///
    /// v[index<1>] = 24;
    ///
    /// assert(get<1>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return A reference to the accessed alternative, if applicable
    template <size_t I>
    [[nodiscard]] constexpr typename detail::traits<detail::select_t<I, T...>>::reference
    operator[](index_t<I> index) & noexcept;

    /// @brief Alternative access operator by index
    ///
    /// @details
    /// This function allows accessing alternatives by index using the square
    /// bracket operator. Because the index must be a compile time value,
    /// instead of passing the index directly, the index is provided as an
    /// instance of @ref index_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing an
    /// index that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v[index<1>] == 42);
    ///
    /// v[index<1>] = 24;
    ///
    /// assert(get<1>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return A reference to the accessed alternative, if applicable
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_reference
        operator[](index_t<I> index) const& noexcept;

    /// @brief Alternative access operator by index
    ///
    /// @details
    /// This function allows accessing alternatives by index using the square
    /// bracket operator. Because the index must be a compile time value,
    /// instead of passing the index directly, the index is provided as an
    /// instance of @ref index_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing an
    /// index that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v[index<1>] == 42);
    ///
    /// v[index<1>] = 24;
    ///
    /// assert(get<1>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return The accessed alternative value, if applicable
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
        operator[](index_t<I> index) &&;

    /// @brief Alternative access operator by index
    ///
    /// @details
    /// This function allows accessing alternatives by index using the square
    /// bracket operator. Because the index must be a compile time value,
    /// instead of passing the index directly, the index is provided as an
    /// instance of @ref index_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing an
    /// index that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v[index<1>] == 42);
    ///
    /// v[index<1>] = 24;
    ///
    /// assert(get<1>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return The accessed alternative value, if applicable
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
        operator[](index_t<I> index) const&&;

    /// @brief Alternative access operator by type
    ///
    /// @details
    /// This function allows accessing alternatives by type using the square
    /// bracket operator. Because the type must be a compile time value,
    /// the type is provided as an instance of @ref type_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing a
    /// type that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_type<int>, 42};
    ///
    /// assert(v[type<int>] == 42);
    ///
    /// v[type<int>] = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return A reference to the accessed alternative, if applicable
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::reference operator[](
        [[maybe_unused]] type_t<U> type) & noexcept {
        return this->operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    /// @brief Alternative access operator by type
    ///
    /// @details
    /// This function allows accessing alternatives by type using the square
    /// bracket operator. Because the type must be a compile time value,
    /// the type is provided as an instance of @ref type_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing a
    /// type that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_type<int>, 42};
    ///
    /// assert(v[type<int>] == 42);
    ///
    /// v[type<int>] = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return A reference to the accessed alternative, if applicable
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_reference operator[](
        [[maybe_unused]] type_t<U> type) const& noexcept {
        return this->operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    /// @brief Alternative access operator by type
    ///
    /// @details
    /// This function allows accessing alternatives by type using the square
    /// bracket operator. Because the type must be a compile time value,
    /// the type is provided as an instance of @ref type_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing a
    /// type that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_type<int>, 42};
    ///
    /// assert(v[type<int>] == 42);
    ///
    /// v[type<int>] = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return The accessed alternative value, if applicable
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::rvalue_reference operator[](
        [[maybe_unused]] type_t<U> type) && {
        return std::move(*this).operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    /// @brief Alternative access operator by type
    ///
    /// @details
    /// This function allows accessing alternatives by type using the square
    /// bracket operator. Because the type must be a compile time value,
    /// the type is provided as an instance of @ref type_t.
    ///
    /// This operator is unchecked and does not throw an exception. Passing a
    /// type that does not correspond to the currently contained alternative
    /// results in undefined behavior.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_type<int>, 42};
    ///
    /// assert(v[type<int>] == 42);
    ///
    /// v[type<int>] = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @param index A tag value that communicates a compile time index
    /// @return The accessed alternative value, if applicable
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_rvalue_reference operator[](
        [[maybe_unused]] type_t<U> type) const&& {
        return std::move(*this).operator[](sumty::index<detail::index_of_v<U, T...>>);
    }

    /// @brief Gets an alternative by index
    ///
    /// @details
    /// This function allows accessing alternatives by index, which is provided
    /// as a template argument.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<1>() == 42);
    ///
    /// v.get<1>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return A reference to the accessed alternative, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding index.
    template <size_t I>
    [[nodiscard]] constexpr typename detail::traits<detail::select_t<I, T...>>::reference
    get() &;

    /// @brief Gets an alternative by index
    ///
    /// @details
    /// This function allows accessing alternatives by index, which is provided
    /// as a template argument.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<1>() == 42);
    ///
    /// v.get<1>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return A reference to the accessed alternative, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding index.
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_reference
        get() const&;

    /// @brief Gets an alternative by index
    ///
    /// @details
    /// This function allows accessing alternatives by index, which is provided
    /// as a template argument.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<1>() == 42);
    ///
    /// v.get<1>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return The accessed alternative value, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding index.
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::rvalue_reference
        get() &&;

    /// @brief Gets an alternative by index
    ///
    /// @details
    /// This function allows accessing alternatives by index, which is provided
    /// as a template argument.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<1>() == 42);
    ///
    /// v.get<1>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return The accessed alternative value, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding index.
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference
        get() const&&;

    /// @brief Gets an alternative by type
    ///
    /// @details
    /// This function allows accessing alternatives by type, which is provided
    /// as a template argument.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<int>() == 42);
    ///
    /// v.get<int>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam U The type of the alternative to access.
    ///
    /// @return A reference to the accessed alternative, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding type.
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::reference get() & {
        return this->template get<detail::index_of_v<U, T...>>();
    }

    /// @brief Gets an alternative by type
    ///
    /// @details
    /// This function allows accessing alternatives by type, which is provided
    /// as a template argument.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<int>() == 42);
    ///
    /// v.get<int>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam U The type of the alternative to access.
    ///
    /// @return A reference to the accessed alternative, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding type.
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_reference get() const& {
        return this->template get<detail::index_of_v<U, T...>>();
    }

    /// @brief Gets an alternative by type
    ///
    /// @details
    /// This function allows accessing alternatives by type, which is provided
    /// as a template argument.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<int>() == 42);
    ///
    /// v.get<int>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam U The type of the alternative to access.
    ///
    /// @return The accessed alternative value, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding type.
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::rvalue_reference get() && {
        return std::move(*this).template get<detail::index_of_v<U, T...>>();
    }

    /// @brief Gets an alternative by type
    ///
    /// @details
    /// This function allows accessing alternatives by type, which is
    /// provided as a template argument.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(v.get<int>() == 42);
    ///
    /// v.get<int>() = 24;
    ///
    /// assert(get<int>(v) == 24);
    /// ```
    ///
    /// @tparam U The type of the alternative to access.
    ///
    /// @return The accessed alternative value, if applicable.
    ///
    /// @throws bad_variant_access Thrown if the @ref variant does not contain
    /// the alternative with the corresponding type.
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_rvalue_reference get()
        const&& {
        return std::move(*this).template get<detail::index_of_v<U, T...>>();
    }

    /// @brief Gets an alternative pointer by index if the @ref variant holds it
    ///
    /// @details
    /// This functions tries to access an alternative by index. If the @ref
    /// variant contains the alternative, this function returns a pointer to
    /// the alternative, if applicable. If the @ref variant does not contain
    /// the alternative, this function returns null. In the case where the
    /// alternative is of type `void`, this function does nothing.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(*v.get_if<1>() == 42);
    ///
    /// *v.get_if<1>() = 24;
    ///
    /// assert(*v.get_if<1>() == 24);
    ///
    /// assert(v.get_if<0>() == nullptr);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return A pointer to the accessed alternative, if applicable, or null
    template <size_t I>
    [[nodiscard]] constexpr typename detail::traits<detail::select_t<I, T...>>::pointer
    get_if() noexcept;

    /// @brief Gets an alternative pointer by index if the @ref variant holds it
    ///
    /// @details
    /// This functions tries to access an alternative by index. If the @ref
    /// variant contains the alternative, this function returns a pointer to
    /// the alternative, if applicable. If the @ref variant does not contain
    /// the alternative, this function returns null. In the case where the
    /// alternative is of type `void`, this function does nothing.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(*v.get_if<1>() == 42);
    ///
    /// *v.get_if<1>() = 24;
    ///
    /// assert(*v.get_if<1>() == 24);
    ///
    /// assert(v.get_if<0>() == nullptr);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return A pointer to the accessed alternative, if applicable, or null
    template <size_t I>
    [[nodiscard]] constexpr
        typename detail::traits<detail::select_t<I, T...>>::const_pointer
        get_if() const noexcept;

    /// @brief Gets an alternative pointer by type if the @ref variant holds it
    ///
    /// @details
    /// This functions tries to access an alternative by type. If the @ref
    /// variant contains the alternative, this function returns a pointer to
    /// the alternative, if applicable. If the @ref variant does not contain
    /// the alternative, this function returns null. In the case where the
    /// alternative is of type `void`, this function does nothing.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(*v.get_if<int>() == 42);
    ///
    /// *v.get_if<int>() = 24;
    ///
    /// assert(*v.get_if<int>() == 24);
    ///
    /// assert(v.get_if<bool>() == nullptr);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return A pointer to the accessed alternative, if applicable, or null
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::pointer get_if() noexcept;

    /// @brief Gets an alternative pointer by type if the @ref variant holds it
    ///
    /// @details
    /// This functions tries to access an alternative by type. If the @ref
    /// variant contains the alternative, this function returns a pointer to
    /// the alternative, if applicable. If the @ref variant does not contain
    /// the alternative, this function returns null. In the case where the
    /// alternative is of type `void`, this function does nothing.
    ///
    /// This function only participates in overload resolution if the type is
    /// unique across all alternative types of the @ref variant. That is,
    /// `variant<A, B, A>` cannot use this function to access an alternative
    /// of type `A`, but it can use this fucntion to access an alternative of
    /// type `B`.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v{std::in_place_index<1>, 42};
    ///
    /// assert(*v.get_if<int>() == 42);
    ///
    /// *v.get_if<int>() = 24;
    ///
    /// assert(*v.get_if<int>() == 24);
    ///
    /// assert(v.get_if<bool>() == nullptr);
    /// ```
    ///
    /// @tparam I The index of the alternative to access.
    ///
    /// @return A pointer to the accessed alternative, if applicable, or null
    template <typename U>
        requires detail::is_unique_v<U, T...>
    [[nodiscard]] constexpr typename detail::traits<U>::const_pointer get_if()
        const noexcept;

    /// @brief Checks if a @ref variant contains a particular alternative.
    ///
    /// @details
    /// Given a type parameter, this function checks if the @ref variant currently
    /// holds an alternative that has the exact same type.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<int, bool, int> v1{std::in_place_index<0>, 42};
    ///
    /// assert(v1.holds_alternative<int>());
    ///
    /// variant<int, bool, int> v2{std::in_place_index<2>, 42};
    ///
    /// assert(v2.holds_alternative<int>());
    /// ```
    ///
    /// @return `true` if the @ref variant holds an alternative of the given type.
    template <typename U>
    [[nodiscard]] constexpr bool holds_alternative() const noexcept;

    /// @brief Calls a visitor callable with the contained alternative
    ///
    /// @details
    /// This function calls the visitor as `std::invoke(visitor, alternative)`
    /// and returns the result of that call, if any. As such, `visitor` *must*
    /// be able to accecpt any alternative type as an argument. In the case of
    /// an alternative of type `void`, the visitor must be callable as
    /// `std::invoke(visitor)` (i.e. with no arguments).
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// Also note that this function is implemented as a compile-time-defined
    /// jump table (array of function pointers). In performance critical
    /// applications, be wary of any assumptions about how well or poorly your
    /// compiler will optimize a call to this function.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v1{std::in_place_index<1>, 42};
    ///
    /// v1.visit(overload(
    ///     [](bool bool_value) { assert(false); },
    ///     [](int int_value) { assert(int_value == 42); },
    ///     [] { assert(false); }
    /// ));
    /// ```
    ///
    /// @param visitor The callable object that will be passed an alternative.
    /// @return The return value of the visitor, if any.
    template <typename V>
    constexpr detail::
        invoke_result_t<V&&, typename detail::traits<detail::select_t<0, T...>>::reference>
        visit(V&& visitor) &;

    /// @brief Calls a visitor callable with the contained alternative
    ///
    /// @details
    /// This function calls the visitor as `std::invoke(visitor, alternative)`
    /// and returns the result of that call, if any. As such, `visitor` *must*
    /// be able to accecpt any alternative type as an argument. In the case of
    /// an alternative of type `void`, the visitor must be callable as
    /// `std::invoke(visitor)` (i.e. with no arguments).
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// Also note that this function is implemented as a compile-time-defined
    /// jump table (array of function pointers). In performance critical
    /// applications, be wary of any assumptions about how well or poorly your
    /// compiler will optimize a call to this function.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v1{std::in_place_index<1>, 42};
    ///
    /// v1.visit(overload(
    ///     [](bool bool_value) { assert(false); },
    ///     [](int int_value) { assert(int_value == 42); },
    ///     [] { assert(false); }
    /// ));
    /// ```
    ///
    /// @param visitor The callable object that will be passed an alternative.
    /// @return The return value of the visitor, if any.
    template <typename V>
    constexpr detail::invoke_result_t<
        V&&,
        typename detail::traits<detail::select_t<0, T...>>::const_reference>
    visit(V&& visitor) const&;

    /// @brief Calls a visitor callable with the contained alternative
    ///
    /// @details
    /// This function calls the visitor as `std::invoke(visitor, alternative)`
    /// and returns the result of that call, if any. As such, `visitor` *must*
    /// be able to accecpt any alternative type as an argument. In the case of
    /// an alternative of type `void`, the visitor must be callable as
    /// `std::invoke(visitor)` (i.e. with no arguments).
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// Also note that this function is implemented as a compile-time-defined
    /// jump table (array of function pointers). In performance critical
    /// applications, be wary of any assumptions about how well or poorly your
    /// compiler will optimize a call to this function.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v1{std::in_place_index<1>, 42};
    ///
    /// v1.visit(overload(
    ///     [](bool bool_value) { assert(false); },
    ///     [](int int_value) { assert(int_value == 42); },
    ///     [] { assert(false); }
    /// ));
    /// ```
    ///
    /// @param visitor The callable object that will be passed an alternative.
    /// @return The return value of the visitor, if any.
    template <typename V>
    constexpr detail::invoke_result_t<
        V&&,
        typename detail::traits<detail::select_t<0, T...>>::rvalue_reference>
    visit(V&& visitor) &&;

    /// @brief Calls a visitor callable with the contained alternative
    ///
    /// @details
    /// This function calls the visitor as `std::invoke(visitor, alternative)`
    /// and returns the result of that call, if any. As such, `visitor` *must*
    /// be able to accecpt any alternative type as an argument. In the case of
    /// an alternative of type `void`, the visitor must be callable as
    /// `std::invoke(visitor)` (i.e. with no arguments).
    ///
    /// Note that the @ref overload function can be helpful for defining a
    /// visitor inline.
    ///
    /// Also note that this function is implemented as a compile-time-defined
    /// jump table (array of function pointers). In performance critical
    /// applications, be wary of any assumptions about how well or poorly your
    /// compiler will optimize a call to this function.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v1{std::in_place_index<1>, 42};
    ///
    /// v1.visit(overload(
    ///     [](bool bool_value) { assert(false); },
    ///     [](int int_value) { assert(int_value == 42); },
    ///     [] { assert(false); }
    /// ));
    /// ```
    ///
    /// @param visitor The callable object that will be passed an alternative.
    /// @return The return value of the visitor, if any.
    template <typename V>
    constexpr detail::invoke_result_t<
        V&&,
        typename detail::traits<detail::select_t<0, T...>>::const_rvalue_reference>
    visit(V&& visitor) const&&;

    /// @brief Swaps two @ref variant instances
    ///
    /// @details
    /// If the two @ref variant instances contain the same alternative, the
    /// alternative values are swapped directly. Otherwise, the alternatives
    /// are swapped by moving out of the variants, destroying the old
    /// alternatives, and move constructed into the new alternatives.
    ///
    /// ## Example:
    /// ```cpp
    /// variant<bool, int, void> v1{std::in_place_index<0>, true};
    ///
    /// variant<bool, int, void> v2{std::in_place_index<1>, 42};
    ///
    /// v1.swap(v2);
    ///
    /// assert(v1.index() == 1);
    ///
    /// assert(get<1>(v1) == 42);
    ///
    /// assert(v2.index() == 0);
    ///
    /// assert(get<2>(v2) == true);
    /// ```
    ///
    /// @param other The "other" @ref variant to swap with this @ref variant
    constexpr void swap(variant& other) noexcept(noexcept(data_.swap(other.data_)));
};

/// @relates variant
/// @brief Checks if a @ref variant contains a particular alternative.
///
/// @details
/// Given a type parameter, this function checks if the @ref variant currently
/// holds an alternative that has the exact same type.
///
/// ## Example:
/// ```cpp
/// variant<int, bool, int> v1{std::in_place_index<0>, 42};
///
/// assert(holds_alternative<int>(v1));
///
/// variant<int, bool, int> v2{std::in_place_index<2>, 42};
///
/// assert(holds_alternative<int>(v2));
/// ```
///
/// @param v The @ref variant to check.
/// @return `true` if the @ref variant holds an alternative of the given type.
template <typename T, typename... U>
constexpr bool holds_alternative(const variant<U...>& v) noexcept;

/// @relates variant
/// @brief Gets a @ref variant alternative by index
///
/// @details
/// This function allows accessing @ref variant alternatives by index, which is
/// provided as a template argument.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<1>(v) == 42);
///
/// get<1>(v) = 24;
///
/// assert(get<1>(v) == 24);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access
///
/// @return A reference to the accessed alternative, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding index.
template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::reference get(
    variant<T...>& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by index
///
/// @details
/// This function allows accessing @ref variant alternatives by index, which is
/// provided as a template argument.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<1>(v) == 42);
///
/// get<1>(v) = 24;
///
/// assert(get<1>(v) == 24);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access
///
/// @return A reference to the accessed alternative, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding index.
template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::const_reference get(
    const variant<T...>& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by index
///
/// @details
/// This function allows accessing @ref variant alternatives by index, which is
/// provided as a template argument.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<1>(v) == 42);
///
/// get<1>(v) = 24;
///
/// assert(get<1>(v) == 24);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access
///
/// @return The accessed alternative value, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding index.
template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::rvalue_reference get(
    variant<T...>&& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by index
///
/// @details
/// This function allows accessing @ref variant alternatives by index, which is
/// provided as a template argument.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<1>(v) == 42);
///
/// get<1>(v) = 24;
///
/// assert(get<1>(v) == 24);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access
///
/// @return The accessed alternative value, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding index.
template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::const_rvalue_reference get(
    const variant<T...>&& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by type
///
/// @details
/// This function allows accessing @ref variant alternatives by type, which is
/// provided as a template argument.
///
/// This function only participates in overload resolution if the type is
/// unique across all alternative types of the @ref variant. That is,
/// `variant<A, B, A>` cannot use this function to access an alternative
/// of type `A`, but it can use this fucntion to access an alternative of
/// type `B`.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<int>(v) == 42);
///
/// get<int>(v) = 24;
///
/// assert(get<int>(v) == 24);
/// ```
///
/// @tparam U The type of the alternative to access.
///
/// @param v The @ref variant to access.
///
/// @return A reference to the accessed alternative, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding type.
template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::reference get(variant<U...>& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by type
///
/// @details
/// This function allows accessing @ref variant alternatives by type, which is
/// provided as a template argument.
///
/// This function only participates in overload resolution if the type is
/// unique across all alternative types of the @ref variant. That is,
/// `variant<A, B, A>` cannot use this function to access an alternative
/// of type `A`, but it can use this fucntion to access an alternative of
/// type `B`.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<int>(v) == 42);
///
/// get<int>(v) = 24;
///
/// assert(get<int>(v) == 24);
/// ```
///
/// @tparam U The type of the alternative to access.
///
/// @param v The @ref variant to access.
///
/// @return A reference to the accessed alternative, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding type.
template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::const_reference get(const variant<U...>& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by type
///
/// @details
/// This function allows accessing @ref variant alternatives by type, which is
/// provided as a template argument.
///
/// This function only participates in overload resolution if the type is
/// unique across all alternative types of the @ref variant. That is,
/// `variant<A, B, A>` cannot use this function to access an alternative
/// of type `A`, but it can use this fucntion to access an alternative of
/// type `B`.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<int>(v) == 42);
///
/// get<int>(v) = 24;
///
/// assert(get<int>(v) == 24);
/// ```
///
/// @tparam U The type of the alternative to access.
///
/// @param v The @ref variant to access.
///
/// @return The accessed alternative value, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding type.
template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::rvalue_reference get(variant<U...>&& v);

/// @relates variant
/// @brief Gets a @ref variant alternative by type
///
/// @details
/// This function allows accessing @ref variant alternatives by type, which is
/// provided as a template argument.
///
/// This function only participates in overload resolution if the type is
/// unique across all alternative types of the @ref variant. That is,
/// `variant<A, B, A>` cannot use this function to access an alternative
/// of type `A`, but it can use this fucntion to access an alternative of
/// type `B`.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(get<int>(v) == 42);
///
/// get<int>(v) = 24;
///
/// assert(get<int>(v) == 24);
/// ```
///
/// @tparam U The type of the alternative to access.
///
/// @param v The @ref variant to access.
///
/// @return The accessed alternative value, if applicable.
///
/// @throws bad_variant_access Thrown if the @ref variant does not contain
/// the alternative with the corresponding type.
template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::const_rvalue_reference get(const variant<U...>&& v);

/// @relates variant
/// @brief Gets a @ref variant alternative pointer by index if the @ref variant holds it
///
/// @details
/// This functions tries to access a @ref variant alternative by index. If the
/// @ref variant contains the alternative, this function returns a pointer to
/// the alternative, if applicable. If the @ref variant does not contain the
/// alternative, this function returns null. In the case where the alternative
/// is of type `void`, this function does nothing.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(*get_if<1>(v) == 42);
///
/// *get_if<1>(v) = 24;
///
/// assert(*get_if<1>(v) == 24);
///
/// assert(get_if<0>(v) == nullptr);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access
///
/// @return A pointer to the accessed alternative, if applicable, or null
template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::pointer get_if(
    variant<T...>& v) noexcept;

/// @relates variant
/// @brief Gets a @ref variant alternative pointer by index if the @ref variant holds it
///
/// @details
/// This functions tries to access a @ref variant alternative by index. If the
/// @ref variant contains the alternative, this function returns a pointer to
/// the alternative, if applicable. If the @ref variant does not contain the
/// alternative, this function returns null. In the case where the alternative
/// is of type `void`, this function does nothing.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(*get_if<1>(v) == 42);
///
/// *get_if<1>(v) = 24;
///
/// assert(*get_if<1>(v) == 24);
///
/// assert(get_if<0>(v) == nullptr);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access
///
/// @return A pointer to the accessed alternative, if applicable, or null
template <size_t I, typename... T>
constexpr typename detail::traits<detail::select_t<I, T...>>::const_pointer get_if(
    const variant<T...>& v) noexcept;

/// @relates variant
/// @brief Gets a @ref variant alternative pointer by type if the @ref variant holds it
///
/// @details
/// This functions tries to access an alternative by type. If the @ref variant
/// contains the alternative, this function returns a pointer to the
/// alternative, if applicable. If the @ref variant does not contain the
/// alternative, this function returns null. In the case where the alternative
/// is of type `void`, this function does nothing.
///
/// This function only participates in overload resolution if the type is
/// unique across all alternative types of the @ref variant. That is,
/// `variant<A, B, A>` cannot use this function to access an alternative
/// of type `A`, but it can use this fucntion to access an alternative of
/// type `B`.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(*get_if<int>(v) == 42);
///
/// *get_if<int>(v) = 24;
///
/// assert(*get_if<int>(v) == 24);
///
/// assert(get_if<bool>(v) == nullptr);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access.
///
/// @return A pointer to the accessed alternative, if applicable, or null
template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::pointer get_if(variant<U...>& v) noexcept;

/// @relates variant
/// @brief Gets a @ref variant alternative pointer by type if the @ref variant holds it
///
/// @details
/// This functions tries to access an alternative by type. If the @ref variant
/// contains the alternative, this function returns a pointer to the
/// alternative, if applicable. If the @ref variant does not contain the
/// alternative, this function returns null. In the case where the alternative
/// is of type `void`, this function does nothing.
///
/// This function only participates in overload resolution if the type is
/// unique across all alternative types of the @ref variant. That is,
/// `variant<A, B, A>` cannot use this function to access an alternative
/// of type `A`, but it can use this fucntion to access an alternative of
/// type `B`.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v{std::in_place_index<1>, 42};
///
/// assert(*get_if<int>(v) == 42);
///
/// *get_if<int>(v) = 24;
///
/// assert(*get_if<int>(v) == 24);
///
/// assert(get_if<bool>(v) == nullptr);
/// ```
///
/// @tparam I The index of the alternative to access.
///
/// @param v The @ref variant to access.
///
/// @return A pointer to the accessed alternative, if applicable, or null
template <typename T, typename... U>
    requires detail::is_unique_v<T, U...>
constexpr typename detail::traits<T>::const_pointer get_if(const variant<U...>& v) noexcept;

/// @relates variant
/// @brief Calls a visitor callable with the contained @ref variant alternatives
///
/// @details
/// This function calls the visitor as `std::invoke(visitor, alternative...)`
/// and returns the result of that call, if any. As such, `visitor` *must*
/// be able to accecpt any combination of alternative types as arguments. In
/// the case of an alternative type `void`, the visitor must accept nothing
/// for that argument. That is, the alternative type combination
/// `int, void, int`, would result in the visitor being called as
/// `std::invoke(visitor, int_value, int_value)`, essentially skipping the
/// `void` alternative.
///
/// Note that the @ref overload function can be helpful for defining a
/// visitor inline.
///
/// Also note that this function is implemented as a compile-time-defined
/// jump table (array of function pointers). In performance critical
/// applications, be wary of any assumptions about how well or poorly your
/// compiler will optimize a call to this function.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v1{std::in_place_index<1>, 42};
/// variant<float, int> v2{std::in_place_index<0>, 3.14};
///
/// visit(overload(
///     [](bool val1, float val2) { assert(false); },
///     [](int val1, float val2) { assert(val1 == 42); },
///     [](float val2) { assert(false); },
///     [](bool val1, int val2) { assert(false); },
///     [](int val1, float val2) { assert(false); },
///     [](int val2) { assert(false); }
/// ), v1, v2);
/// ```
///
/// @param visitor The callable object that will be passed an alternative.
///
/// @return The return value of the visitor, if any.
template <typename V>
constexpr std::invoke_result_t<V&&> visit(V&& visitor);

/// @relates variant
/// @brief Calls a visitor callable with the contained @ref variant alternatives
///
/// @details
/// This function calls the visitor as `std::invoke(visitor, alternative...)`
/// and returns the result of that call, if any. As such, `visitor` *must*
/// be able to accecpt any combination of alternative types as arguments. In
/// the case of an alternative type `void`, the visitor must accept nothing
/// for that argument. That is, the alternative type combination
/// `int, void, int`, would result in the visitor being called as
/// `std::invoke(visitor, int_value, int_value)`, essentially skipping the
/// `void` alternative.
///
/// Note that the @ref overload function can be helpful for defining a
/// visitor inline.
///
/// Also note that this function is implemented as a compile-time-defined
/// jump table (array of function pointers). In performance critical
/// applications, be wary of any assumptions about how well or poorly your
/// compiler will optimize a call to this function.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v1{std::in_place_index<1>, 42};
/// variant<float, int> v2{std::in_place_index<0>, 3.14};
///
/// visit(overload(
///     [](bool val1, float val2) { assert(false); },
///     [](int val1, float val2) { assert(val1 == 42); },
///     [](float val2) { assert(false); },
///     [](bool val1, int val2) { assert(false); },
///     [](int val1, float val2) { assert(false); },
///     [](int val2) { assert(false); }
/// ), v1, v2);
/// ```
///
/// @param visitor The callable object that will be passed an alternative.
///
/// @param var0 The first variant to visit
///
/// @param varn The remaining variant to visit
///
/// @return The return value of the visitor, if any.
template <typename V, typename T0, typename... TN>
constexpr detail::invoke_result_t<V&&,
                                  decltype(get<0>(std::declval<T0&&>())),
                                  decltype(get<0>(std::declval<TN&&>()))...>
visit(V&& visitor, T0&& var0, TN&&... varn);

/// @relates variant
/// @brief Swaps two @ref variant instances
///
/// @details
/// If the two @ref variant instances contain the same alternative, the
/// alternative values are swapped directly. Otherwise, the alternatives
/// are swapped by moving out of the variants, destroying the old
/// alternatives, and move constructed into the new alternatives.
///
/// ## Example:
/// ```cpp
/// variant<bool, int, void> v1{std::in_place_index<0>, true};
///
/// variant<bool, int, void> v2{std::in_place_index<1>, 42};
///
/// swap(v1, v2);
///
/// assert(v1.index() == 1);
///
/// assert(get<1>(v1) == 42);
///
/// assert(v2.index() == 0);
///
/// assert(get<2>(v2) == true);
/// ```
///
/// @param a The first variant in the swap
/// @param b The second variant in the swap
template <typename... T>
constexpr void swap(variant<T...>& a, variant<T...>& b);

namespace detail {

template <typename T>
struct variant_size_helper;

template <typename... T>
struct variant_size_helper<variant<T...>> : std::integral_constant<size_t, sizeof...(T)> {};

template <typename... T>
struct variant_size_helper<const variant<T...>>
    : std::integral_constant<size_t, sizeof...(T)> {};

template <size_t I, typename T>
struct variant_alternative_helper;

template <size_t I, typename... T>
struct variant_alternative_helper<I, variant<T...>> {
    using type = detail::select_t<I, T...>;
};

template <size_t I, typename... T>
struct variant_alternative_helper<I, const variant<T...>>
    : variant_alternative_helper<I, variant<T...>> {};

} // namespace detail

/// @relates variant
/// @class variant_size <sumty/variant.hpp>
/// @brief Utility to get the number of alternative in a @ref variant
///
/// @details
/// @ref varaint_size provides the number alternatives in the @ref variant,
/// `T`, in the static constexpr member `value`.
///
/// ## Example:
/// ```cpp
/// assert(variant_size<variant<bool, int, void>>::value == 3);
/// ```
///
/// @tparam T The @ref variant type to get the size of
template <typename T>
struct variant_size : detail::variant_size_helper<T> {};

/// @relates variant
/// @relates variant_size
/// @brief Utility to get the number of alternative in a variant
///
/// @details
/// @ref varaint_size_v provides the number alternatives in the @ref variant,
/// `T`.
///
/// ## Example:
/// ```cpp
/// assert(variant_size_v<variant<bool, int, void>> == 3);
/// ```
///
/// @tparam T The @ref variant type to get the size of
template <typename T>
static inline constexpr size_t variant_size_v = variant_size<T>::value;

/// @relates variant
/// @class variant_alternative <sumty/variant.hpp>
/// @brief Utility to get the type of a @ref variant alternative with some index
///
/// @details
/// @ref variant_alternative provides the type of a @ref alternative with the
/// index `I` in the member type, `type`.
///
/// ## Example:
/// ```cpp
/// assert(std::is_same_v<
///     typename variant_alternative<1, variant<bool, int, void>>::type,
///     int
/// >);
/// ```
///
/// @tparam I The index of the @variant alternative
///
/// @tparam T The @ref variant type containing the alternative
template <size_t I, typename T>
struct variant_alternative : detail::variant_alternative_helper<I, T> {};

/// @relates variant_alternative
/// @relates variant
/// @brief Utility to get the type of a @ref variant alternative with some index
///
/// @details
/// @ref variant_alternative provides the type of a @ref alternative with the
/// index `I` in the member type, `type`.
///
/// ## Example:
/// ```cpp
/// assert(std::is_same_v<
///         variant_alternative_t<1, variant<bool, int, void>>, int>);
/// ```
///
/// @tparam I The index of the @variant alternative
///
/// @tparam T The @ref variant type containing the alternative
template <size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

} // namespace sumty

#include "sumty/impl/variant.hpp" // IWYU pragma: export

#endif
