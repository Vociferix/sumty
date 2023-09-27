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

#include "sumty/variant.hpp"

#include "sumty/detail/auto_union.hpp"
#include "sumty/detail/utils.hpp"
#include "sumty/exceptions.hpp"
#include "sumty/utils.hpp"

namespace sumty {

namespace detail {

template <typename Enable, typename... T>
class variant_impl {
  private:
    using discrim_t = discriminant_t<static_cast<uint64_t>(sizeof...(T))>;

    SUMTY_NO_UNIQ_ADDR auto_union<T...> data_;
    discrim_t discrim_{};

    template <size_t I>
    constexpr void copy_construct(const auto_union<T...>& data) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template construct<I>(data.template get<I>());
            } else {
                copy_construct<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void move_construct(auto_union<T...>& data) noexcept(
        (true && ... && traits<T>::is_nothrow_move_constructible)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template construct<I>(std::move(data.template get<I>()));
            } else {
                move_construct<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void destroy() noexcept((true && ... &&
                                       traits<T>::is_nothrow_destructible)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template destroy<I>();
            } else {
                destroy<I + 1>();
            }
        }
    }

    template <size_t I>
    constexpr void copy_assign(const auto_union<T...>& data) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template get<I>() = data.template get<I>();
            } else {
                copy_assign<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void move_assign(auto_union<T...>& data) noexcept(
        (true && ... && traits<T>::is_nothrow_move_assignable)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                data_.template get<I>() = std::move(data.template get<I>());
            } else {
                move_assign<I + 1>(data);
            }
        }
    }

    template <size_t I>
    constexpr void same_swap(auto_union<T...>& data) noexcept(
        (true && ... && traits<T>::is_nothrow_swappable)) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                if constexpr (!std::is_void_v<select_t<I, T...>>) {
                    using std::swap;
                    swap(data_.template get<I>(), data.template get<I>());
                }
            } else {
                same_swap<I + 1>(data);
            }
        }
    }

    template <size_t I, size_t J>
    constexpr void diff_swap_impl(variant_impl<T...>& other) noexcept(
        (true && ... &&
         (traits<T>::is_nothrow_move_constructible &&
          traits<T>::is_nothrow_destructible))) {
        std::swap(discrim_, other.discrim_);
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            if constexpr (std::is_void_v<select_t<J, T...>>) {
                return;
            } else if constexpr (std::is_lvalue_reference_v<
                                     select_t<J, T...>>) {
                data_.template construct<J>(other.data_.template get<J>());
                other.data_.template destroy<J>();
                other.data_.template construct<I>();
            } else {
                data_.template construct<J>(
                    std::move(other.data_.template get<J>()));
                other.data_.template destroy<J>();
                other.data_.template construct<I>();
            }
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            if constexpr (std::is_void_v<select_t<J, T...>>) {
                other.data_.template construct<I>(data_.template get<I>());
                data_.template destroy<I>();
                data_.template construct<J>();
            } else if constexpr (std::is_lvalue_reference_v<
                                     select_t<J, T...>>) {
                auto& tmp = other.data_.template get<J>();
                other.data_.template construct<I>(data_.template get<I>());
                data_.template construct<J>(tmp);
            } else {
                auto& tmp = data_.template get<I>();
                data_.template construct<J>(
                    std::move(other.data_.template get<J>()));
                other.data_.template destroy<J>();
                other.data_.template construct<I>(tmp);
            }
        } else {
            if constexpr (std::is_void_v<select_t<J, T...>>) {
                other.data_.template construct<I>(
                    std::move(data_.template get<I>()));
                data_.template destroy<I>();
                data_.template construct<J>();
            } else if constexpr (std::is_lvalue_reference_v<
                                     select_t<J, T...>>) {
                auto& tmp = other.data_.template get<J>();
                other.data_.template construct<I>(
                    std::move(data_.template get<I>()));
                data_.template destroy<I>();
                data_.template construct<J>(tmp);
            } else {
                auto&& tmp = std::move(other.data_.template get<J>());
                other.data_.template destroy<J>();
                other.data_.template construct<I>(
                    std::move(data_.template get<I>()));
                data_.template destroy<I>();
                data_.template construct<J>(std::move(tmp));
            }
        }
    }

    template <size_t I, size_t J>
    constexpr void diff_swap_nested(variant_impl<T...>& other) noexcept(
        (true && ... &&
         (traits<T>::is_nothrow_move_constructible &&
          traits<T>::is_nothrow_destructible))) {
        if constexpr (J < sizeof...(T)) {
            if (other.discrim_ == static_cast<discrim_t>(J)) {
                diff_swap_impl<I, J>(other);
            } else {
                diff_swap_nested<I, J + 1>(other);
            }
        }
    }

    template <size_t I>
    constexpr void diff_swap(variant<T...>& other) noexcept(
        (true && ... &&
         (traits<T>::is_nothrow_move_constructible &&
          traits<T>::is_nothrow_destructible))) {
        if constexpr (I < sizeof...(T)) {
            if (discrim_ == static_cast<discrim_t>(I)) {
                diff_swap_nested<I, 0>(data_.template get<I>(), other);
            } else {
                diff_swap<I + 1>(other);
            }
        }
    }

  public:
    constexpr variant_impl() noexcept(
        traits<first_t<T...>>::is_nothrow_default_constructible)
        : variant_impl(std::in_place_index<0>) {}

    constexpr variant_impl(const variant_impl& other)
        : discrim_(other.discrim_) {
        copy_construct<0>(other.data_);
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        (true && ... && traits<T>::is_nothrow_move_constructible))
        : discrim_(other.discrim_) {
        move_construct<0>(other.data_);
    }

    template <size_t I, typename... Args>
    constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<I> in_place,
        Args&&... args) noexcept(traits<select_t<I, T...>>::
                                     template is_nothrow_constructible<
                                         Args...>) {
        data_.template construct<I>(std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept((true && ... &&
                                        traits<T>::is_nothrow_destructible)) {
        destroy<0>();
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (discrim_ == rhs.discrim_) {
                copy_assign<0>(rhs.data_);
            } else {
                destroy<0>();
                copy_construct<0>(rhs.data_);
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        (true && ... &&
         (traits<T>::is_nothrow_move_assignable &&
          traits<T>::is_nothrow_move_constructible &&
          traits<T>::is_nothrow_destructible))) {
        if (discrim_ == rhs.discrim_) {
            move_assign<0>(rhs.data_);
        } else {
            destroy<0>();
            move_construct<0>(rhs.data_);
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(discrim_);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::reference get() & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::const_reference get()
        const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::rvalue_reference get() && {
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::const_rvalue_reference get()
        const&& {
        if constexpr (std::is_void_v<select_t<I, T...>>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<select_t<I, T...>>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::pointer ptr() noexcept {
        if constexpr (!std::is_void_v<select_t<I, T...>>) {
            return &data_.template get<I>();
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T...>>::const_pointer ptr()
        const noexcept {
        if constexpr (!std::is_void_v<select_t<I, T...>>) {
            return &data_.template get<I>();
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) {
        destroy<0>();
        data_.template construct<I>(std::forward<Args>(args)...);
        discrim_ = static_cast<discrim_t>(I);
    }

    constexpr void swap(variant_impl& other) noexcept(
        (true && ... &&
         (traits<T>::is_nothrow_swappable &&
          traits<T>::is_nothrow_move_constructible &&
          traits<T>::is_nothrow_destructible))) {
        if (discrim_ == other.discrim_) {
            same_swap<0>(other.data_);
        } else {
            diff_swap<0>(other);
        }
    }
};

template <>
class variant_impl<void> {};

template <typename T>
class variant_impl<void, T> {
  private:
    SUMTY_NO_UNIQ_ADDR auto_union<T> data_;

  public:
    constexpr variant_impl() noexcept(
        traits<T>::is_nothrow_default_constructible) {
        data_.template construct<0>();
    }

    constexpr variant_impl(const variant_impl& other) {
        data_.template construct<0>(other.data_.template get<0>());
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        traits<T>::is_nothrow_move_constructible) {
        data_.template construct<0>(other.data_.template get<0>());
    }

    template <typename... Args>
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place,
                           Args&&... args) {
        data_.template construct<0>(std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept(traits<T>::is_nothrow_destructible) {
        data_.template destroy<0>();
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if constexpr (std::is_lvalue_reference_v<T>) {
                data_.template construct<0>(rhs.data_.template get<0>());
            } else {
                data_.template get<0>() = rhs.data_.template get<0>();
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        traits<T>::is_nothrow_move_assignable) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            data_.template construct<0>(rhs.data_.template get<0>());
        } else {
            data_.template get<0>() = std::move(rhs.data_.template get<0>());
        }
        return *this;
    }

    constexpr size_t index() const noexcept { return 0; }

    template <size_t I>
    constexpr typename traits<T>::reference get() & noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<T>::const_reference get() const& noexcept {
        return data_.template get<I>();
    }

    template <size_t I>
    constexpr typename traits<T>::rvalue_reference get() && {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<T>::const_rvalue_reference get() const&& {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return data_.template get<I>();
        } else {
            return std::move(data_.template get<I>());
        }
    }

    template <size_t I>
    constexpr typename traits<T>::pointer ptr() noexcept {
        if constexpr (!std::is_void_v<T>) { return &data_.template get<I>(); }
    }

    template <size_t I>
    constexpr typename traits<T>::const_pointer ptr() const noexcept {
        if constexpr (!std::is_void_v<T>) { return &data_.template get<I>(); }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) {
        data_.template destroy<0>();
        data_.template construct<0>(std::forward<Args>(args)...);
    }

    constexpr void swap(variant_impl& other) noexcept(
        traits<T>::is_nothrow_swappable) {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            auto& tmp = other.data_.template get<0>();
            other.data_.template construct<0>(data_.template get<0>());
            data_.template construct<0>(tmp);
        } else {
            using std::swap;
            swap(data_.template get<0>(), other.data_.template get<0>());
        }
    }
};

template <>
class variant_impl<void, void> {
  public:
    constexpr variant_impl() noexcept = default;

    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<0> in_place) noexcept {}

    template <typename T>
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place,
                           [[maybe_unused]] T&& value) noexcept {}

    constexpr size_t index() const noexcept { return 0; }

    template <size_t I>
    constexpr void get() const noexcept {}

    template <size_t I>
    constexpr void ptr() const noexcept {}

    constexpr void swap([[maybe_unused]] variant_impl& other) noexcept {}
};

template <typename T>
class variant_impl<void, T&, void> {
  private:
    T* data_;

  public:
    variant_impl() = delete;

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place,
                           U& value) noexcept
        : data_(&value) {}

    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<1> in_place) noexcept
        : data_(nullptr) {}

    template <typename U>
    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<1> in_place,
        [[maybe_unused]] U&& value) noexcept
        : data_(nullptr) {}

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(data_ == nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, void>>::const_reference get()
        const noexcept {
        if constexpr (I == 0) { return *data_; }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, void>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 0) { return data_; }
    }

    template <size_t I>
    constexpr void emplace() noexcept {
        static_assert(I == 1, "no matching constructor for reference");
        data_ = nullptr;
    }

    template <size_t I, typename U>
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        if constexpr (I == 0) {
            static_assert(std::is_lvalue_reference_v<U>,
                          "no matching constructor for reference");
            data_ = &value;
        } else {
            data_ = nullptr;
        }
    }

    constexpr void swap(variant_impl& other) noexcept {
        std::swap(data_, other.data_);
    }
};

template <typename T>
class variant_impl<void, void, T&> {
  private:
    T* data_{nullptr};

  public:
    constexpr variant_impl() noexcept = default;

    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<0> in_place) noexcept
        : data_(nullptr) {}

    template <typename U>
    explicit constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<0> in_place,
        [[maybe_unused]] U&& value) noexcept
        : data_(nullptr) {}

    template <typename U>
        requires(std::is_convertible_v<U*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place,
                           U& value) noexcept
        : data_(&value) {}

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(data_ != nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, void, T&>>::const_reference get()
        const noexcept {
        if constexpr (I == 1) { return *data_; }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, void, T&>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 1) { return data_; }
    }

    template <size_t I>
    constexpr void emplace() noexcept {
        static_assert(I == 0, "no matching constructor for reference");
        data_ = nullptr;
    }

    template <size_t I, typename U>
    constexpr void emplace([[maybe_unused]] U&& value) noexcept {
        if constexpr (I == 1) {
            static_assert(std::is_lvalue_reference_v<U>,
                          "no matching constructor for reference");
            data_ = &value;
        } else {
            data_ = nullptr;
        }
    }

    constexpr void swap(variant_impl& other) noexcept {
        std::swap(data_, other.data_);
    }
};

template <typename T, typename U>
class variant_impl<std::enable_if_t<(sizeof(U) <= sizeof(bool))>, T&, U> {
  private:
    T* head_;
    union {
        SUMTY_NO_UNIQ_ADDR U tail_;
    };

  public:
    variant_impl() = delete;

    constexpr variant_impl(const variant_impl& other) : head_(other.head_) {
        if (head_ == nullptr) { std::construct_at(&tail_, other.tail_); }
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        std::is_nothrow_move_constructible_v<U>)
        : head_(other.head_) {
        if (head_ == nullptr) {
            std::construct_at(&tail_, std::move(other.tail_));
        }
    }

    template <typename V>
        requires(std::is_convertible_v<V*, T*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<0> in_place,
                           V& value) noexcept
        : head_(&value) {}

    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<1> in_place,
        Args&&... args)
        : head_(nullptr) {
        std::construct_at(&tail_, std::forward<Args>(args)...);
    }

    constexpr ~variant_impl() noexcept(std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) { std::destroy_at(&tail_); }
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (head_ == nullptr) {
                head_ = rhs.head_;
                if (head_ == nullptr) {
                    tail_ = rhs.tail_;
                } else {
                    std::destroy_at(&tail_);
                }
            } else {
                head_ == rhs.head_;
                if (head_ == nullptr) { std::construct_at(&tail_, rhs.tail_); }
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        std::is_nothrow_move_assignable_v<U>&&
            std::is_nothrow_move_constructible_v<U>&&
                std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            head_ = rhs.head_;
            if (head_ == nullptr) {
                tail_ = std::move(rhs.tail_);
            } else {
                std::destroy_at(&tail_);
            }
        } else {
            head_ == rhs.head_;
            if (head_ == nullptr) {
                std::construct_at(&tail_, std::move(rhs.tail_));
            }
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(head_ == nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::reference get() & noexcept {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::const_reference get()
        const& noexcept {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::rvalue_reference get() && {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return std::move(tail_);
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::const_rvalue_reference get()
        const&& {
        if constexpr (I == 0) {
            return *head_;
        } else {
            return std::move(tail_);
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::pointer ptr() noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return &tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T&, U>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return &tail_;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        if constexpr (I == 0) {
            static_assert(
                (true && ... &&
                 std::is_lvalue_reference_v<Args>)&&sizeof...(Args) == 1,
                "no matching constructor for reference");
            if (head_ != nullptr) { std::destroy_at(&tail_); }
            head_ = std::addressof(std::forward<Args>(args)...);
        } else {
            if (head_ == nullptr) {
                std::destroy_at(&tail_);
            } else {
                head_ = nullptr;
            }
            std::construct_at(&tail_, std::forward<Args>(args)...);
        }
    }

    constexpr void swap(variant_impl& other) noexcept(
        std::is_nothrow_swappable_v<U>&& std::is_nothrow_move_constructible_v<
            U>&& std::is_nothrow_destructible_v<U>) {
        if (head_ == nullptr) {
            if (other.head_ == nullptr) {
                using std::swap;
                swap(tail_, other.tail_);
            } else {
                head_ = other.head_;
                other.head_ = nullptr;
                std::construct_at(&other.tail_, std::move(tail_));
                std::destroy_at(&tail_);
            }
        } else {
            if (other.head_ == nullptr) {
                other.head_ = head_;
                head_ = nullptr;
                std::construct_at(&tail_, std::move(other.tail_));
                std::destroy_at(&other.tail_);
            } else {
                std::swap(head_, other.head_);
            }
        }
    }
};

template <typename T, typename U>
class variant_impl<std::enable_if_t<(sizeof(T) <= sizeof(bool))>, T, U&> {
  private:
    U* tail_{nullptr};
    union {
        SUMTY_NO_UNIQ_ADDR T head_;
    };

  public:
    constexpr variant_impl() noexcept(
        std::is_nothrow_default_constructible_v<T>) {
        std::construct_at(&head_);
    }

    constexpr variant_impl(const variant_impl& other) : tail_(other.tail_) {
        if (tail_ == nullptr) { std::construct_at(&head_, other.head_); }
    }

    constexpr variant_impl(variant_impl&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : tail_(other.tail_) {
        if (tail_ == nullptr) {
            std::construct_at(&head_, std::move(other.head_));
        }
    }

    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr variant_impl(
        [[maybe_unused]] std::in_place_index_t<0> in_place,
        Args&&... args) {
        std::construct_at(&head_, std::forward<Args>(args)...);
    }

    template <typename V>
        requires(std::is_convertible_v<V*, U*>)
    constexpr variant_impl([[maybe_unused]] std::in_place_index_t<1> in_place,
                           V& value) noexcept
        : tail_(&value) {}

    constexpr ~variant_impl() noexcept(std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) { std::destroy_at(&head_); }
    }

    constexpr variant_impl& operator=(const variant_impl& rhs) {
        if (this != &rhs) {
            if (tail_ == nullptr) {
                tail_ = rhs.tail_;
                if (tail_ == nullptr) {
                    head_ = rhs.head_;
                } else {
                    std::destroy_at(&head_);
                }
            } else {
                tail_ == rhs.tail_;
                if (head_ == nullptr) { std::construct_at(&head_, rhs.head_); }
            }
        }
        return *this;
    }

    constexpr variant_impl& operator=(variant_impl&& rhs) noexcept(
        std::is_nothrow_move_assignable_v<T>&&
            std::is_nothrow_move_constructible_v<T>&&
                std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            tail_ = rhs.tail_;
            if (tail_ == nullptr) {
                head_ = std::move(rhs.head_);
            } else {
                std::destroy_at(&head_);
            }
        } else {
            tail_ == rhs.tail_;
            if (head_ == nullptr) {
                std::construct_at(&head_, std::move(rhs.head_));
            }
        }
        return *this;
    }

    constexpr size_t index() const noexcept {
        return static_cast<size_t>(tail_ != nullptr);
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::reference get() & noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::const_reference get()
        const& noexcept {
        if constexpr (I == 0) {
            return head_;
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::rvalue_reference get() && {
        if constexpr (I == 0) {
            return std::move(head_);
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::const_rvalue_reference get()
        const&& {
        if constexpr (I == 0) {
            return std::move(head_);
        } else {
            return *tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::pointer ptr() noexcept {
        if constexpr (I == 0) {
            return &head_;
        } else {
            return tail_;
        }
    }

    template <size_t I>
    constexpr typename traits<select_t<I, T, U&>>::const_pointer ptr()
        const noexcept {
        if constexpr (I == 0) {
            return &head_;
        } else {
            return tail_;
        }
    }

    template <size_t I, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        if constexpr (I == 1) {
            static_assert(
                (true && ... &&
                 std::is_lvalue_reference_v<Args>)&&sizeof...(Args) == 1,
                "no matching constructor for reference");
            if (tail_ != nullptr) { std::destroy_at(&head_); }
            tail_ = std::addressof(std::forward<Args>(args)...);
        } else {
            if (tail_ == nullptr) {
                std::destroy_at(&head_);
            } else {
                tail_ = nullptr;
            }
            std::construct_at(&head_, std::forward<Args>(args)...);
        }
    }

    constexpr void swap(variant_impl& other) noexcept(
        std::is_nothrow_swappable_v<T>&& std::is_nothrow_move_constructible_v<
            T>&& std::is_nothrow_destructible_v<T>) {
        if (tail_ == nullptr) {
            if (other.tail_ == nullptr) {
                using std::swap;
                swap(head_, other.head_);
            } else {
                tail_ = other.tail_;
                other.tail_ = nullptr;
                std::construct_at(&other.head_, std::move(head_));
                std::destroy_at(&head_);
            }
        } else {
            if (other.tail_ == nullptr) {
                other.tail_ = tail_;
                tail_ = nullptr;
                std::construct_at(&head_, std::move(other.head_));
                std::destroy_at(&other.head_);
            } else {
                std::swap(tail_, other.tail_);
            }
        }
    }
};

} // namespace detail

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
        return visit_impl<IDX + 1>(std::forward<V>(visitor),
                                   std::forward<U>(var));
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
constexpr variant<T...>::variant(std::in_place_index_t<IDX> in_place,
                                 Args&&... args)
    : data_(in_place, std::forward<Args>(args)...) {}

template <typename... T>
template <size_t IDX, typename U, typename... Args>
constexpr variant<T...>::variant(std::in_place_index_t<IDX> in_place,
                                 std::initializer_list<U> init,
                                 Args&&... args)
    : data_(in_place, init, std::forward<Args>(args)...) {}

template <typename... T>
template <typename U, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr variant<T...>::variant(
    [[maybe_unused]] std::in_place_type_t<U> in_place,
    Args&&... args)
    : data_(std::in_place_index<detail::index_of_v<U, T...>>,
            std::forward<Args>(args)...) {}

template <typename... T>
template <typename U, typename V, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr variant<T...>::variant(
    [[maybe_unused]] std::in_place_type_t<U> in_place,
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
    data_.template emplace<detail::index_of_v<U, T...>>(
        std::forward<Args>(args)...);
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U, typename V, typename... Args>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::emplace(std::initializer_list<V> ilist,
                                                Args&&... args) {
    data_.template emplace<detail::index_of_v<U, T...>>(
        ilist, std::forward<Args>(args)...);
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](
    index_t<I> index) & noexcept {
    return data_.template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](
    index_t<I> index) const& noexcept {
    return data_.template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](index_t<I> index) && {
    return std::move(data_).template get<I>();
}

template <typename... T>
template <size_t I>
constexpr decltype(auto) variant<T...>::operator[](index_t<I> index) const&& {
    return std::move(data_).template get<I>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](type_t<U> type) & noexcept {
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](
    type_t<U> type) const& noexcept {
    return data_.template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](type_t<U> type) && {
    return std::move(data_).template get<detail::index_of_v<U, T...>>();
}

template <typename... T>
template <typename U>
    requires(detail::is_unique_v<U, T...>)
constexpr decltype(auto) variant<T...>::operator[](type_t<U> type) const&& {
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
