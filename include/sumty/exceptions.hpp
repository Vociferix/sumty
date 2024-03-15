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

#ifndef SUMTY_EXCEPTIONS_HPP
#define SUMTY_EXCEPTIONS_HPP

#include <exception>

namespace sumty {

class bad_variant_access : public std::exception {
  public:
    bad_variant_access() noexcept = default;
    bad_variant_access(const bad_variant_access&) = default;
    bad_variant_access(bad_variant_access&&) noexcept = default;
    ~bad_variant_access() override = default;
    bad_variant_access& operator=(const bad_variant_access&) = default;
    bad_variant_access& operator=(bad_variant_access&&) noexcept = default;

    [[nodiscard]] const char* what() const noexcept override {
        return "bad variant access";
    }
};

class bad_option_access : public std::exception {
  public:
    bad_option_access() noexcept = default;
    bad_option_access(const bad_option_access&) = default;
    bad_option_access(bad_option_access&&) noexcept = default;
    ~bad_option_access() override = default;
    bad_option_access& operator=(const bad_option_access&) = default;
    bad_option_access& operator=(bad_option_access&&) noexcept = default;

    [[nodiscard]] const char* what() const noexcept override { return "bad option access"; }
};

template <typename E>
class bad_result_access : public std::exception {
  private:
    E err_;

  public:
    explicit bad_result_access(E error) : err_(std::move(error)) {}

    bad_result_access(const bad_result_access&) = default;

    bad_result_access(bad_result_access&&) noexcept(
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        std::is_nothrow_move_constructible_v<E>) = default;

    ~bad_result_access() noexcept override = default;

    bad_result_access& operator=(const bad_result_access&) = default;

    bad_result_access& operator=(bad_result_access&&) noexcept(
        // NOLINTNEXTLINE(performance-noexcept-move-constructor)
        std::is_nothrow_move_assignable_v<E>) = default;

    [[nodiscard]] E& error() & noexcept { return err_; }

    [[nodiscard]] const E& error() const& noexcept { return err_; }

    [[nodiscard]] E&& error() && { return std::move(err_); }

    [[nodiscard]] const E&& error() const&& { return std::move(err_); }

    [[nodiscard]] const char* what() const noexcept override { return "bad result access"; }
};

template <>
class bad_result_access<void> : public std::exception {
  public:
    bad_result_access() noexcept = default;

    bad_result_access(const bad_result_access&) noexcept = default;

    bad_result_access(bad_result_access&&) noexcept = default;

    ~bad_result_access() noexcept override = default;

    bad_result_access& operator=(const bad_result_access&) = default;

    bad_result_access& operator=(bad_result_access&&) = default;

    void error() const noexcept {}

    [[nodiscard]] const char* what() const noexcept override { return "bad result access"; }
};

} // namespace sumty

#endif
