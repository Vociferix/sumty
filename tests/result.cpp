#include <catch2/catch_test_macros.hpp>

#include <system_error>

#include "sumty/result.hpp"

using namespace sumty;

TEST_CASE("special result sizes", "[result]") {
    STATIC_CHECK(sizeof(result<void, void>) == sizeof(bool));
    STATIC_CHECK(sizeof(result<int&, void>) == sizeof(void*));
    STATIC_CHECK(sizeof(result<int, int>) == sizeof(int) * 2);
}

TEST_CASE("result default construct", "[result]") {
    result<int> res1{};
    REQUIRE(res1.has_value() == true);
    REQUIRE(*res1 == int{});
    REQUIRE(res1.value() == int{});
    const result<void> res2{};
    REQUIRE(res2.has_value() == true);
    STATIC_CHECK(std::is_void_v<decltype(*res2)>);
    STATIC_CHECK(std::is_void_v<decltype(res2.value())>);
}

TEST_CASE("result construct from value", "[result]") {
    static constexpr int VALUE = 42;
    int i = VALUE;
    result<int> res1{VALUE};
    REQUIRE(res1.has_value() == true);
    REQUIRE(*res1 == VALUE);
    REQUIRE(res1.value() == VALUE);
    result<int&> res2{i};
    REQUIRE(res2.has_value() == true);
    REQUIRE(*res2 == VALUE);
    REQUIRE(&*res2 == &i);
    REQUIRE(res2.value() == VALUE);
    REQUIRE(&res2.value() == &i);
    result<int> res3{ok<int>(VALUE)};
    REQUIRE(res3.has_value() == true);
    REQUIRE(*res3 == VALUE);
    result<int&> res4{ok<int&>(i)};
    REQUIRE(res4.has_value() == true);
    REQUIRE(*res4 == VALUE);
    REQUIRE(&*res4 == &i);
    REQUIRE(res4.value() == VALUE);
    REQUIRE(&res4.value() == &i);
    result<int> res5{std::in_place, VALUE};
    REQUIRE(res5.has_value() == true);
    REQUIRE(*res5 == VALUE);
    result<int&> res6{std::in_place, i};
    REQUIRE(res6.has_value() == true);
    REQUIRE(*res6 == VALUE);
    REQUIRE(&*res6 == &i);
    REQUIRE(res6.value() == VALUE);
    REQUIRE(&res6.value() == &i);
}

TEST_CASE("result construct from error value", "[result]") {
    static constexpr int VALUE = 42;
    int i = VALUE;
    result<void, int> res1{error<int>(VALUE)};
    REQUIRE(res1.has_value() == false);
    REQUIRE(res1.error() == VALUE);
    result<void, int&> res2{error<int&>(i)};
    REQUIRE(res2.has_value() == false);
    REQUIRE(res2.error() == VALUE);
    REQUIRE(&res2.error() == &i);
    result<void, int> res3{in_place_error, VALUE};
    REQUIRE(res3.has_value() == false);
    REQUIRE(res3.error() == VALUE);
    result<void, int&> res4{in_place_error, i};
    REQUIRE(res4.has_value() == false);
    REQUIRE(res4.error() == VALUE);
    REQUIRE(&res4.error() == &i);
}

TEST_CASE("result assign from value", "[result]") {
    static constexpr int VALUE = 42;
    int i = VALUE;
    result<int> res1{};
    res1 = VALUE;
    REQUIRE(res1.has_value() == true);
    REQUIRE(*res1 == VALUE);
    REQUIRE(res1.value() == VALUE);
    result<int&> res2{error<std::error_code>()};
    res2 = i;
    REQUIRE(res2.has_value() == true);
    REQUIRE(*res2 == VALUE);
    REQUIRE(&*res2 == &i);
    REQUIRE(res2.value() == VALUE);
    REQUIRE(&*res2 == &i);
    result<int> res3{};
    res3 = ok<int>(VALUE);
    REQUIRE(res3.has_value() == true);
    REQUIRE(*res3 == VALUE);
    REQUIRE(res3.value() == VALUE);
    result<int&> res4{error<std::error_code>()};
    res4 = ok<int&>(i);
    REQUIRE(res4.has_value() == true);
    REQUIRE(*res4 == VALUE);
    REQUIRE(&*res4 == &i);
    REQUIRE(res4.value() == VALUE);
    REQUIRE(&*res4 == &i);
}

TEST_CASE("result assign from error", "[result]") {
    static constexpr int VALUE = 42;
    int i = VALUE;
    result<void, int> res1{};
    res1 = error<int>(VALUE);
    REQUIRE(res1.has_value() == false);
    REQUIRE(res1.error() == VALUE);
    result<void, int&> res2{};
    res2 = error<int&>(i);
    REQUIRE(res2.has_value() == false);
    REQUIRE(res2.error() == VALUE);
    REQUIRE(&res2.error() == &i);
}

TEST_CASE("result converting construct from value", "[result]") {
    static constexpr float FLT_VAL = 3.14F;
    static constexpr int INT_VAL = static_cast<int>(FLT_VAL);
    result<int> res1{FLT_VAL};
    REQUIRE(res1.has_value() == true);
    REQUIRE(*res1 == INT_VAL);
    REQUIRE(res1.value() == INT_VAL);
    result<int> res2{ok<float>(FLT_VAL)};
    REQUIRE(res2.has_value() == true);
    REQUIRE(*res2 == INT_VAL);
    REQUIRE(res2.value() == INT_VAL);
}

TEST_CASE("result converting construct from error", "[result]") {
    static constexpr float FLT_VAL = 3.14F;
    static constexpr int INT_VAL = static_cast<int>(FLT_VAL);
    result<void, int> res1{error<float>(FLT_VAL)};
    REQUIRE(res1.has_value() == false);
    REQUIRE(res1.error() == INT_VAL);
}

TEST_CASE("result and_then", "[result]") {
    static constexpr int VALUE = 42;
    const result<int> res1{VALUE};
    const auto res2 = res1.and_then([value = VALUE](auto val) -> result<unsigned> {
        REQUIRE(val == value);
        return static_cast<unsigned>(val) * 2;
    });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<unsigned>>);
    REQUIRE(res2.has_value());
    REQUIRE(*res2 == static_cast<unsigned>(VALUE) * 2);

    const result<void> res3{};
    const auto res4 = res3.and_then([value = VALUE]() -> result<int> { return value; });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res4)>, result<int>>);
    REQUIRE(res4.has_value());

    const result<void, int> res5{error<int>(VALUE)};
    const auto res6 = res5.and_then([]() -> result<void, int> {
        REQUIRE(false);
        return {};
    });
    REQUIRE(!res6.has_value());

    const result<void, void> res7{error<void>()};
    const auto res8 = res7.and_then([]() -> result<void, void> {
        REQUIRE(false);
        return {};
    });
    REQUIRE(!res8.has_value());
}

TEST_CASE("result transform", "[result]") {
    static constexpr int VALUE = 42;
    const result<int> res1{VALUE};
    const auto res2 = res1.transform([value = VALUE](auto val) {
        REQUIRE(val == value);
        return static_cast<unsigned>(val) * 2;
    });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<unsigned>>);
    REQUIRE(res2.has_value());
    REQUIRE(*res2 == static_cast<unsigned>(VALUE) * 2);

    const result<void> res3{};
    const auto res4 = res3.transform([value = VALUE]() { return value; });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res4)>, result<int>>);
    REQUIRE(res4.has_value());

    const result<void, int> res5{error<int>(VALUE)};
    const auto res6 = res5.transform([]() { REQUIRE(false); });
    REQUIRE(!res6.has_value());

    const result<void, void> res7{error<void>()};
    const auto res8 = res7.transform([]() { REQUIRE(false); });
    REQUIRE(!res8.has_value());
}

TEST_CASE("result or_else") {
    static constexpr int VALUE = 42;
    const result<int, void> res1{error<void>()};
    auto res2 = res1.or_else([]() -> result<int, void> { return 0; });
    REQUIRE(res2.has_value() == true);
    REQUIRE(*res2 == 0);

    const result<int, void> res3{VALUE};
    auto res4 = res3.or_else([]() -> result<int, void> {
        REQUIRE(false);
        return 0;
    });
    REQUIRE(res4.has_value() == true);
    REQUIRE(*res4 == VALUE);

    const result<void, int> res5{};
    auto res6 = res5.or_else([]([[maybe_unused]] auto err) -> result<void, int> {
        REQUIRE(false);
        return {};
    });
    REQUIRE(res6.has_value() == true);

    const result<int, int> res7{error<int>(VALUE)};
    const auto res8 = res7.or_else([value = VALUE](auto err) -> result<int, int> {
        REQUIRE(err == value);
        return value;
    });
    REQUIRE(res8.has_value() == true);
    REQUIRE(*res8 == VALUE);
}

TEST_CASE("result transform_error") {
    static constexpr int VALUE = 42;
    const result<void, int> res1{error<int>(VALUE)};
    const auto res2 = res1.transform_error([value = VALUE](auto val) {
        REQUIRE(val == value);
        return static_cast<unsigned>(val) * 2;
    });
    STATIC_REQUIRE(
        std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<void, unsigned>>);
    REQUIRE(!res2.has_value());
    REQUIRE(res2.error() == static_cast<unsigned>(VALUE) * 2);

    const result<int, void> res3{error<void>()};
    const auto res4 = res3.transform_error([value = VALUE]() { return value; });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res4)>, result<int, int>>);
    REQUIRE(!res4.has_value());

    const result<int, void> res5{VALUE};
    const auto res6 = res5.transform_error([]() { REQUIRE(false); });
    REQUIRE(res6.has_value());
    REQUIRE(*res6 == VALUE);

    const result<void, void> res7{};
    const auto res8 = res7.transform_error([]() { REQUIRE(false); });
    REQUIRE(res8.has_value());
}

TEST_CASE("result value_or", "[result]") {
    static constexpr int VALUE = 42;
    const result<int, void> res1{VALUE};
    REQUIRE(res1.value_or(VALUE * 2) == VALUE);
    const result<int, void> res2{error<void>()};
    REQUIRE(res2.value_or(VALUE) == VALUE);
    const result<void, void> res3{error<void>()};
    res3.value_or();
}

TEST_CASE("result or_none", "[result]") {
    static constexpr int VALUE = 42;
    const result<int, void> res1{VALUE};
    const auto opt1 = res1.or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt1)>, option<int>>);
    REQUIRE(opt1.has_value());
    REQUIRE(*opt1 == VALUE);

    const result<int, void> res2{error<void>()};
    const auto opt2 = res2.or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt2)>, option<int>>);
    REQUIRE(!opt2.has_value());

    const result<void, void> res3{};
    const auto opt3 = res3.or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt3)>, option<void>>);
    REQUIRE(opt3.has_value());

    const result<void, void> res4{error<void>()};
    const auto opt4 = res4.or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt4)>, option<void>>);
    REQUIRE(!opt4.has_value());
}

TEST_CASE("result error_or_none", "[result]") {
    static constexpr int VALUE = 42;
    const result<void, int> res1{error<int>(VALUE)};
    const auto opt1 = res1.error_or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt1)>, option<int>>);
    REQUIRE(opt1.has_value());
    REQUIRE(*opt1 == VALUE);

    const result<void, int> res2{};
    const auto opt2 = res2.error_or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt2)>, option<int>>);
    REQUIRE(!opt2.has_value());

    const result<void, void> res3{error<void>()};
    const auto opt3 = res3.error_or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt3)>, option<void>>);
    REQUIRE(opt3.has_value());

    const result<void, void> res4{};
    const auto opt4 = res4.error_or_none();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(opt4)>, option<void>>);
    REQUIRE(!opt4.has_value());
}

TEST_CASE("result ref", "[result]") {
    static constexpr int VALUE = 42;
    const result<int, int> res1{VALUE};
    const auto res2 = res1.ref();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>,
                                  result<const int&, const int&>>);
    REQUIRE(res2.has_value());
    REQUIRE(&*res2 == &*res1);

    const result<int, int> res3{error<int>(VALUE)};
    const auto res4 = res3.ref();
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res4)>,
                                  result<const int&, const int&>>);
    REQUIRE(!res4.has_value());
    REQUIRE(&res4.error() == &res3.error());
}

TEST_CASE("result visit", "[result]") {
    static constexpr int VALUE = 42;
    const result<int, bool> res1{VALUE};
    auto val1 = res1.visit([value = VALUE](auto val) -> int {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, int>) {
            REQUIRE(val == value);
            return val;
        } else {
            REQUIRE(false);
            return 0;
        }
    });
    REQUIRE(val1 == VALUE);

    const result<bool, int> res2{error<int>(VALUE)};
    auto val2 = res2.visit([value = VALUE](auto val) -> int {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(val)>, int>) {
            REQUIRE(val == value);
            return val;
        } else {
            REQUIRE(false);
            return 0;
        }
    });
    REQUIRE(val2 == VALUE);
}

#include "sumty/impl/result.hpp" // IWYU pragma: associated
