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

template <typename T, typename E>
template <typename R, typename U, typename V>
constexpr variant<U, V> result<T, E>::convert(R&& res) {
    if (res.has_value()) {
        if constexpr (std::is_void_v<decltype(*std::forward<R>(res))>) {
            return variant<U, V>{std::in_place_index<0>};
        } else {
            return variant<U, V>{std::in_place_index<0>, *std::forward<R>(res)};
        }
    } else {
        if constexpr (std::is_void_v<decltype(std::forward<R>(res).error())>) {
            return result<U, V>{std::in_place_index<1>};
        } else {
            return result<U, V>{std::in_place_index<1>, std::forward<R>(res).error()};
        }
    }
}

template <typename T, typename E>
template <typename U>
    requires(!std::is_same_v<result<T, E>, std::remove_cvref_t<U>> &&
             !detail::is_error_v<std::remove_cvref_t<U>> &&
             !detail::is_ok_v<std::remove_cvref_t<U>> &&
             detail::traits<T>::template is_constructible<U> &&
             detail::traits<T>::template is_assignable<U>)
constexpr result<T, E>& result<T, E>::operator=(U&& value) {
    if (res_.index() == 0) {
        res_[index<0>] = std::forward<U>(value);
    } else {
        res_.template emplace<0>(std::forward<U>(value));
    }
    return *this;
}

template <typename T, typename E>
template <typename U>
    requires((detail::traits<T>::template is_constructible<typename detail::traits<U>::const_reference> &&
             detail::traits<T>::template is_assignable<typename detail::traits<U>::const_reference>) ||
             (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
             std::is_void_v<T>)
constexpr result<T, E>& result<T, E>::operator=(const ok_t<U>& value) {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<U>) {
            res_.template emplace<0>();
        } else {
            res_.template emplace<0>(*value);
        }
    } else {
        if constexpr (std::is_void_v<U>) {
            if constexpr (!std::is_void_v<T>) {
                res_[index<0>] = T{};
            }
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

template <typename T, typename E>
template <typename U>
    requires((detail::traits<T>::template is_constructible<typename detail::traits<U>::rvalue_reference> &&
             detail::traits<T>::template is_assignable<typename detail::traits<U>::rvalue_reference>) ||
             (std::is_void_v<U> && detail::traits<T>::is_default_constructible) ||
             std::is_void_v<T>)
constexpr result<T, E>& result<T, E>::operator=(ok_t<U>&& value) {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<U>) {
            res_.template emplace<0>();
        } else {
            res_.template emplace<0>(*std::move(value));
        }
    } else {
        if constexpr (std::is_void_v<U>) {
            if constexpr (!std::is_void_v<T>) {
                res_[index<0>] = T{};
            }
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

template <typename T, typename E>
template <typename V>
    requires((detail::traits<E>::template is_constructible<typename detail::traits<V>::const_reference> &&
             detail::traits<E>::template is_assignable<typename detail::traits<V>::const_reference>) ||
             (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
             std::is_void_v<E>)
constexpr result<T, E>& result<T, E>::operator=(const error_t<V>& error) {
    if (res_.index() == 0) {
        if constexpr (std::is_void_v<V>) {
            res_.template emplace<1>();
        } else {
            res_.template emplace<1>(*error);
        }
    } else {
        if constexpr (std::is_void_v<V>) {
            if constexpr (!std::is_void_v<E>) {
                res_[index<1>] = E{};
            }
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

template <typename T, typename E>
template <typename V>
    requires((detail::traits<E>::template is_constructible<typename detail::traits<V>::rvalue_reference> &&
             detail::traits<E>::template is_assignable<typename detail::traits<V>::rvalue_reference>) ||
             (std::is_void_v<V> && detail::traits<E>::is_default_constructible) ||
             std::is_void_v<E>)
constexpr result<T, E>& result<T, E>::operator=(error_t<V>&& value) {
    if (res_.index() == 0) {
        if constexpr (std::is_void_v<V>) {
            res_.template emplace<1>();
        } else {
            res_.template emplace<1>(*std::move(error));
        }
    } else {
        if constexpr (std::is_void_v<V>) {
            if constexpr (!std::is_void_v<E>) {
                res_[index<1>] = E{};
            }
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

template <typename T, typename E>
constexpr result<T, E>::operator bool() const noexcept {
    return res_.index() == 0;
}

template <typename T, typename E>
constexpr bool result<T, E>::has_value() const noexcept {
    return res_.index() == 0;
}

template <typename T, typename E>
constexpr typename result<T, E>::reference result<T, E>::operator*() & noexcept {
    return res_[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::const_reference result<T, E>::operator*() const& noexcept {
    return res_[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::rvalue_reference result<T, E>::operator*() && {
    return std::move(res_)[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::const_rvalue_reference result<T, E>::operator*() const&& {
    return std::move(res_)[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::pointer result<T, E>::operator->() noexcept {
    return &res_[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::const_pointer result<T, E>::operator->() const noexcept {
    return &res_[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::reference result<T, E>::value() & {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<E>) {
            throw bad_result_access<void>();
        } else {
            throw bad_result_access<std::remove_cvref_t<E>>(res_[index<1>]);
        }
    }
    return res_[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::const_reference result<T, E>::value() const& {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<E>) {
            throw bad_result_access<void>();
        } else {
            throw bad_result_access<std::remove_cvref_t<E>>(res_[index<1>]);
        }
    }
    return res_[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::rvalue_reference result<T, E>::value() && {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<E>) {
            throw bad_result_access<void>();
        } else {
            throw bad_result_access<std::remove_cvref_t<E>>(std::move(res_)[index<1>]);
        }
    }
    return std::move(res_)[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::rvalue_reference result<T, E>::value() const&& {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<E>) {
            throw bad_result_access<void>();
        } else {
            throw bad_result_access<std::remove_cvref_t<E>>(std::move(res_)[index<1>]);
        }
    }
    return std::move(res_)[index<0>];
}

template <typename T, typename E>
constexpr typename result<T, E>::error_reference result<T, E>::error() & noexcept {
    return res_[index<1>];
}

template <typename T, typename E>
constexpr typename result<T, E>::error_const_reference result<T, E>::error() const& noexcept {
    return res_[index<1>];
}

template <typename T, typename E>
constexpr typename result<T, E>::error_rvalue_reference result<T, E>::error() && {
    return std::move(res_)[index<1>];
}

template <typename T, typename E>
constexpr typename result<T, E>::error_const_rvalue_reference result<T, E>::error() const&& {
    return std::move(res_)[index<1>];
}

template <typename T, typename E>
template <typename U>
constexpr typename result<T, E>::value_type result<T, E>::value_or(U&& default_value) const& {
    if (res_.index() == 0) {
        return res_[index<0>];
    } else {
        return static_cast<value_type>(std::forward<U>(default_value));
    }
}

template <typename T, typename E>
template <typename U>
constexpr typename result<T, E>::value_type result<T, E>::value_or(U&& default_value) && {
    if (res_.index() == 0) {
        return std::move(res_)[index<0>];
    } else {
        return static_cast<value_type>(std::forward<U>(default_value));
    }
}

template <typename T, typename E>
constexpr typename result<T, E>::value_type result<T, E>::value_or() const& {
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

template <typename T, typename E>
constexpr typename result<T, E>::value_type result<T, E>::value_or() && {
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

template <typename T, typename E>
template <typename F>
constexpr auto result<T, E>::and_then(F&& f) & {
    if constexpr (std::is_void_v<T>) {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f));
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F>>{};
        }
    } else {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f), res_[index<0>]);
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, reference>>{};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr auto result<T, E>::and_then(F&& f) const& {
    if constexpr (std::is_void_v<T>) {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f));
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F>>{};
        }
    } else {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f), res_[index<0>]);
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>{};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr auto result<T, E>::and_then(F&& f) && {
    if constexpr (std::is_void_v<T>) {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f));
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F>>{};
        }
    } else {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>{};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr auto result<T, E>::and_then(F&& f) const&& {
    if constexpr (std::is_void_v<T>) {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f));
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F>>{};
        }
    } else {
        if (res_.index() == 0) {
            return std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, const_rvalue_reference>>{};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform(F&& f) & {
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
            return result<res_t, E>{in_place_error, res_[index<1>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, reference>;
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), res_[index<0>]);
                return result<res_t, E>{std::in_place};
            } else {
                return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f), res_[index<0>])};
            }
        } else {
            return result<res_t, E>{in_place_error, res_[index<1>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform(F&& f) const& {
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
            return result<res_t, E>{in_place_error, res_[index<1>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, const_reference>;
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), res_[index<0>]);
                return result<res_t, E>{std::in_place};
            } else {
                return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f), res_[index<0>])};
            }
        } else {
            return result<res_t, E>{in_place_error, res_[index<1>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform(F&& f) && {
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
            return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, rvalue_reference>;
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
                return result<res_t, E>{std::in_place};
            } else {
                return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f), std::move(res_)[index<0>])};
            }
        } else {
            return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform(F&& f) const&& {
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
            return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, const_rvalue_reference>;
        if (res_.index() == 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), std::move(res_)[index<0>]);
                return result<res_t, E>{std::in_place};
            } else {
                return result<res_t, E>{std::in_place, std::invoke(std::forward<F>(f), std::move(res_)[index<0>])};
            }
        } else {
            return result<res_t, E>{in_place_error, std::move(res_)[index<1>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr auto result<T, E>::or_else(F&& f) const& {
    if (std::is_void_v<E>) {
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
        using res_t = std::remove_cvref_t<std::invoke_result_t<F, error_const_reference>>;
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

template <typename T, typename E>
template <typename F>
constexpr auto result<T, E>::or_else(F&& f) && {
    if (std::is_void_v<E>) {
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
        using res_t = std::remove_cvref_t<std::invoke_result_t<F, error_rvalue_reference>>;
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

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform_error(F&& f) & {
    if constexpr (std::is_void_v<E>) {
        using res_t = std::invoke_result_t<F>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f));
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f))};
            }
        } else {
            return result<T, res_t>{std::in_place, res_[index<0>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, error_reference>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), res_[index<1>]);
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f), res_[index<1>])};
            }
        } else {
            return result<T, res_t>{std::in_place, res_[index<0>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform_error(F&& f) const& {
    if constexpr (std::is_void_v<E>) {
        using res_t = std::invoke_result_t<F>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f));
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f))};
            }
        } else {
            return result<T, res_t>{std::in_place, res_[index<0>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, error_const_reference>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), res_[index<1>]);
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f), res_[index<1>])};
            }
        } else {
            return result<T, res_t>{std::in_place, res_[index<0>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform_error(F&& f) && {
    if constexpr (std::is_void_v<E>) {
        using res_t = std::invoke_result_t<F>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f));
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f))};
            }
        } else {
            return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, error_rvalue_reference>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f), std::move(res_)[index<1>])};
            }
        } else {
            return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
        }
    }
}

template <typename T, typename E>
template <typename F>
constexpr decltype(auto) result<T, E>::transform_error(F&& f) const&& {
    if constexpr (std::is_void_v<E>) {
        using res_t = std::invoke_result_t<F>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f));
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f))};
            }
        } else {
            return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
        }
    } else {
        using res_t = std::invoke_result_t<F, error_const_rvalue_reference>;
        if (res_.index() != 0) {
            if constexpr (std::is_void_v<res_t>) {
                std::invoke(std::forward<F>(f), std::move(res_)[index<1>]);
                return result<T, res_t>{in_place_error};
            } else {
                return result<T, res_t>{in_place_error, std::invoke(std::forward<F>(f), std::move(res_)[index<1>])};
            }
        } else {
            return result<T, res_t>{std::in_place, std::move(res_)[index<0>]};
        }
    }
}

template <typename T, typename E>
constexpr result<typename result<T, E>::reference, typename result<T, E>::error_reference> result<T, E>::ref() noexcept {
    if (res_.index() == 0) {
        return result<reference, error_reference>{std::in_place, **this};
    } else {
        return result<reference, error_reference>{in_place_error, this.error()};
    }
}

template <typename T, typename E>
constexpr result<typename result<T, E>::const_reference, typename result<T, E>::error_const_reference> result<T, E>::ref() const noexcept {
    if (res_.index() == 0) {
        return result<const_reference, error_const_reference>{std::in_place, **this};
    } else {
        return result<const_reference, error_const_reference>{in_place_error, this.error()};
    }
}

template <typename T, typename E>
constexpr result<typename result<T, E>::const_reference, typename result<T, E>::error_const_reference> result<T, E>::cref() const noexcept {
    return ref();
}

template <typename T, typename E>
constexpr option<T> result<T, E>::or_none() const& noexcept {
    if (res_.index() == 0) {
        if constexpr (std::is_void_v<T>) {
            return option<void>{std::in_place};
        } else {
            return option<T>{std::in_place, res_[index<0>]};
        }
    } else {
        return option<T>{};
    }
}

template <typename T, typename E>
constexpr option<T> result<T, E>::or_none() && {
    if (res_.index() == 0) {
        if constexpr (std::is_void_v<T>) {
            return option<void>{std::in_place};
        } else {
            return option<T>{std::in_place, std::move(res_)[index<0>]};
        }
    } else {
        return option<T>{};
    }
}

template <typename T, typename E>
constexpr option<E> result<T, E>::error_or_none() const& noexcept {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<E>) {
            return option<void>{std::in_place};
        } else {
            return option<E>{std::in_place, res_[index<1>]};
        }
    } else {
        return option<E>{};
    }
}

template <typename T, typename E>
constexpr option<E> result<T, E>::error_or_none() && {
    if (res_.index() != 0) {
        if constexpr (std::is_void_v<E>) {
            return option<void>{std::in_place};
        } else {
            return option<E>{std::in_place, std::move(res_)[index<1>]};
        }
    } else {
        return option<E>{};
    }
}

template <typename T, typename E>
template <typename... Args>
constexpr typename result<T, E>::reference result<T, E>::emplace(Args&&... args) {
    return res_.template emplace<0>(std::forward<Args>(args)...);
}

template <typename T, typename E>
template <typename U, typename... Args>
constexpr typename result<T, E>::reference result<T, E>::emplace(std::initializer_list<U> ilist, Args&&... args) {
    return res_.template emplace<0>(std::forward<Args>(args)...);
}

template <typename T, typename E>
constexpr void result<T, E>::swap(result<T, E>& other) noexcept(std::is_nothrow_swappable_v<variant<T, E>>) {
    res_.swap(other.res_);
}

template <typename T, typename E, typename U, typename V>
    requires(std::is_void_v<T> == std::is_void_v<U> && std::is_void_v<E> == std::is_void_v<V>)
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

template <typename T, typename E, typename U>
constexpr bool operator==(const result<T, E>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

template <typename T, typename E, typename V>
constexpr bool operator==(const result<T, E>& lhs, const error_t<V>& rhs) {
    return !lhs.has_value() && lhs.error() == rhs;
}

template <typename T, typename E>
constexpr void swap(result<T, E>& a, result<T, E>& b) noexcept(std::is_nothrow_swappable_v<variant<T, E>>) {
    a.swap(b);
}

template <typename E, typename... Args>
constexpr error_t<E> error(Args&&... args) {
    return error_t<E>(std::in_place, std::forward<Args>(args)...);
}

template <typename E, typename U, typename... Args>
constexpr error_t<E> error(std::initializer_list<U> ilist, Args&&... args) {
    return error_t<E>(std::in_place, ilist, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr ok_t<T> ok(Args&&... args) {
    return ok_t<T>{std::in_place, std::forward<Args>(args)...};
}

template <typename T, typename U, typename... Args>
constexpr ok_t<T> ok(std::initializer_list<U> ilist, Args&&... args) {
    return ok_t<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

}
