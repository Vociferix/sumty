#include <catch2/catch_test_macros.hpp>
#include <compare>
#include <optional>
#include <string>
#include <type_traits>

#include "sumty/option.hpp" // IWYU pragma: associated
#include "sumty/result.hpp"

using namespace sumty;

TEST_CASE("special option sizes", "[option]") {
    STATIC_CHECK(sizeof(option<void>) == sizeof(bool));
    STATIC_CHECK(sizeof(option<int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(option<float>) == sizeof(float) * 2);
}

TEST_CASE("option default construct", "[option]") {
    const option<int> opt1;
    REQUIRE(opt1.has_value() == false);
    const option<void> opt2;
    REQUIRE(opt2.has_value() == false);
    const option<int&> opt3;
    REQUIRE(opt3.has_value() == false);
}

TEST_CASE("option construct from value", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt{VALUE};
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == VALUE);
    REQUIRE(opt.value() == VALUE);
}

TEST_CASE("option assign from value", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt;
    REQUIRE(opt.has_value() == false);
    opt = VALUE;
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == VALUE);
    REQUIRE(opt.value() == VALUE);
    opt = none;
    REQUIRE(opt.has_value() == false);
}

TEST_CASE("option construct from ptr", "[option]") {
    static constexpr int VALUE = 42;
    int i = VALUE;
    int* i_ptr = &i;
    int* null_ptr = nullptr;
    option<int&> opt1{i_ptr};
    const option<int&> opt2{null_ptr};
    REQUIRE(opt1.has_value() == true);
    REQUIRE(*opt1 == VALUE);
    REQUIRE(&*opt1 == &i);
    REQUIRE(opt2.has_value() == false);
    int* opt1_ptr = opt1;
    int* opt2_ptr = opt2;
    REQUIRE(opt1_ptr == &i);
    REQUIRE(opt2_ptr == nullptr);
}

TEST_CASE("option assign from ptr", "[option]") {
    static constexpr int VALUE = 42;
    int i = VALUE;
    int* null_ptr = nullptr;
    option<int&> opt{};
    opt = &i;
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == VALUE);
    REQUIRE(&*opt == &i);
    opt = null_ptr;
    REQUIRE(opt.has_value() == false);
}

TEST_CASE("option converting construct", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{VALUE};
    option<long long> opt2{opt1};
    REQUIRE(opt2.has_value() == true);
    REQUIRE(*opt2 == static_cast<long long>(VALUE));
    const option<int> opt3{};
    const option<long long> opt4{opt3};
    REQUIRE(opt4.has_value() == false);
}

TEST_CASE("option converting assignment", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{VALUE};
    option<long long> opt2{};
    opt2 = opt1;
    REQUIRE(opt2.has_value() == true);
    REQUIRE(*opt2 == static_cast<long long>(VALUE));
    const option<int> opt3{};
    option<long long> opt4{};
    opt4 = opt3;
    REQUIRE(opt4.has_value() == false);
}

TEST_CASE("option converting construct from value", "[option]") {
    static constexpr int VALUE = 42;
    option<long long> opt{VALUE};
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == static_cast<long long>(VALUE));
}

TEST_CASE("option assignment from value", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt{};
    opt = VALUE;
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == VALUE);
}

TEST_CASE("option converting assigment from value", "[option]") {
    static constexpr int VALUE = 42;
    option<long long> opt{};
    opt = VALUE;
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == static_cast<long long>(VALUE));
}

TEST_CASE("option in condition", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt{};
    if (opt) {
        REQUIRE(false);
    } else {
        REQUIRE(true);
    }
    opt = VALUE;
    if (opt) {
        REQUIRE(true);
    } else {
        REQUIRE(false);
    }
}

TEST_CASE("option value_or", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{};
    const option<int> opt2{VALUE};
    REQUIRE(opt1.value_or(0) == 0);
    REQUIRE(opt1.value_or(VALUE) == VALUE);
    REQUIRE(opt1.value_or() == int{});
    REQUIRE(opt2.value_or(0) == VALUE);
    REQUIRE(opt2.value_or(VALUE) == VALUE);
    REQUIRE(opt2.value_or() == VALUE);
}

TEST_CASE("option and_then", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt1{};
    auto opt2 = opt1.and_then([](int val) -> option<int> { return val + 1; });
    REQUIRE(opt2.has_value() == false);
    option<int> opt3{VALUE};
    auto opt4 = opt3.and_then([](int val) -> option<int> { return val + 1; });
    REQUIRE(opt4.has_value() == true);
    REQUIRE(*opt4 == VALUE + 1);
}

TEST_CASE("option or_else", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{};
    auto opt2 = opt1.or_else([]() -> option<int> { return 0; });
    REQUIRE(opt2.has_value() == true);
    REQUIRE(*opt2 == 0);
    const option<int> opt3{VALUE};
    auto opt4 = opt3.or_else([]() -> option<int> { return 0; });
    REQUIRE(opt4.has_value() == true);
    REQUIRE(*opt4 == VALUE);
}

TEST_CASE("option transform", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt1{};
    auto opt2 = opt1.transform(
        [](int val) -> long long { return static_cast<long long>(val) + 1; });
    REQUIRE(opt2.has_value() == false);
    option<int> opt3{VALUE};
    auto opt4 = opt3.transform(
        [](int val) -> long long { return static_cast<long long>(val) + 1; });
    REQUIRE(opt4.has_value() == true);
    REQUIRE(*opt4 == static_cast<long long>(VALUE) + 1);
}

TEST_CASE("option value_or_else", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{};
    const option<int> opt2{VALUE};
    REQUIRE(opt1.value_or_else([] { return 0; }) == 0);
    REQUIRE(opt1.value_or_else([] { return VALUE; }) == VALUE);
    REQUIRE(opt2.value_or_else([] { return 0; }) == VALUE);
    REQUIRE(opt2.value_or_else([] { return VALUE; }) == VALUE);
}

TEST_CASE("option ok_or", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{};
    const option<int> opt2{VALUE};
    const auto res1 = opt1.ok_or(VALUE * 2);
    const auto res2 = opt2.ok_or(VALUE * 2);
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res1)>, result<int, int>>);
    REQUIRE(!res1.has_value());
    REQUIRE(res1.error() == VALUE * 2);
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<int, int>>);
    REQUIRE(res2.has_value());
    REQUIRE(*res2 == VALUE);
}

TEST_CASE("option ok_or_else", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{};
    const option<int> opt2{VALUE};
    const auto res1 = opt1.ok_or_else([] { return VALUE * 2; });
    const auto res2 = opt2.ok_or_else([] { return VALUE * 2; });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res1)>, result<int, int>>);
    REQUIRE(!res1.has_value());
    REQUIRE(res1.error() == VALUE * 2);
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<int, int>>);
    REQUIRE(res2.has_value());
    REQUIRE(*res2 == VALUE);
}

TEST_CASE("option error_or", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{VALUE};
    const option<int> opt2{};
    const auto res1 = opt1.error_or(VALUE * 2);
    const auto res2 = opt2.error_or(VALUE * 2);
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res1)>, result<int, int>>);
    REQUIRE(!res1.has_value());
    REQUIRE(res1.error() == VALUE);
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<int, int>>);
    REQUIRE(res2.has_value());
    REQUIRE(*res2 == VALUE * 2);
}

TEST_CASE("option error_or_else", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{VALUE};
    const option<int> opt2{};
    const auto res1 = opt1.error_or_else([] { return VALUE * 2; });
    const auto res2 = opt2.error_or_else([] { return VALUE * 2; });
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res1)>, result<int, int>>);
    REQUIRE(!res1.has_value());
    REQUIRE(res1.error() == VALUE);
    STATIC_REQUIRE(std::is_same_v<std::remove_cvref_t<decltype(res2)>, result<int, int>>);
    REQUIRE(res2.has_value());
    REQUIRE(*res2 == VALUE * 2);
}

TEST_CASE("option ref", "[option]") {
    static constexpr int VALUE = 42;
    const option<int> opt1{};
    const option<int> opt2{VALUE};
    REQUIRE(opt1.ref() == nullptr);
    REQUIRE(&*opt2.ref() == &*opt2);
}

struct Test {
  private:
    bool* alive_;

  public:
    constexpr explicit Test(bool& alive_flag) : alive_(&alive_flag) { alive_flag = true; }

    Test(const Test&) = delete;
    Test(Test&&) = delete;

    constexpr ~Test() { *alive_ = false; }

    Test& operator=(const Test&) = delete;
    Test& operator=(Test&&) = delete;
};

TEST_CASE("option emplace and reset", "[option]") {
    bool alive = false;
    option<Test> opt{};
    REQUIRE(opt.has_value() == false);
    opt.emplace(alive);
    REQUIRE(opt.has_value() == true);
    REQUIRE(alive == true);
    opt.reset();
    REQUIRE(opt.has_value() == false);
    REQUIRE(alive == false);
}

TEST_CASE("option swap", "[option]") {
    static constexpr int VALUE1 = 42;
    static constexpr int VALUE2 = 24;
    option<int> opt1{};
    option<int> opt2{VALUE1};
    swap(opt1, opt2);
    REQUIRE(opt1.has_value() == true);
    REQUIRE(*opt1 == VALUE1);
    REQUIRE(opt2.has_value() == false);
    opt2 = VALUE2;
    swap(opt1, opt2);
    REQUIRE(opt1.has_value() == true);
    REQUIRE(*opt1 == VALUE2);
    REQUIRE(opt2.has_value() == true);
    REQUIRE(*opt2 == VALUE1);
    opt1.reset();
    opt2.reset();
    swap(opt1, opt2);
    REQUIRE(opt1.has_value() == false);
    REQUIRE(opt2.has_value() == false);
}

TEST_CASE("option some", "[option]") {
    auto opt = some<std::string>("hello");
    STATIC_CHECK(std::is_same_v<option<std::string>, std::remove_cvref_t<decltype(opt)>>);
    REQUIRE(opt.has_value() == true);
    REQUIRE(*opt == "hello");
}

TEST_CASE("option visit method", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt1{};
    option<int> opt2{VALUE};
    auto r1 = opt1.visit([](auto val) { return static_cast<int>(val) + 1; });
    REQUIRE(r1 == 1);
    auto r2 = opt2.visit([](auto val) { return static_cast<int>(val) + 1; });
    REQUIRE(r2 == VALUE + 1);
}

TEST_CASE("option visit func", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt1{};
    option<int> opt2{VALUE};
    auto r1 = visit([](auto val) { return static_cast<int>(val) + 1; }, opt1);
    REQUIRE(r1 == 1);
    auto r2 = visit([](auto val) { return static_cast<int>(val) + 1; }, opt2);
    REQUIRE(r2 == VALUE + 1);
}

TEST_CASE("multi option visit func", "[option]") {
    static constexpr int INT_VAL = 42;
    static constexpr float FLT_VAL = 3.14F;
    option<int> opt1{};
    option<float> opt2{};

    auto visitor = [](auto ival, auto fval) {
        return static_cast<int>(ival) + static_cast<int>(fval);
    };

    auto r1 = visit(visitor, opt1, opt2);
    REQUIRE(r1 == 0);
    opt1.emplace(INT_VAL);
    auto r2 = visit(visitor, opt1, opt2);
    REQUIRE(r2 == INT_VAL);
    opt2.emplace(FLT_VAL);
    auto r3 = visit(visitor, opt1, opt2);
    REQUIRE(r3 == INT_VAL + static_cast<int>(FLT_VAL));
    opt1.reset();
    auto r4 = visit(visitor, opt1, opt2);
    REQUIRE(r4 == static_cast<int>(FLT_VAL));
}

TEST_CASE("option compare", "[option]") {
    static constexpr int VALUE = 42;
    option<int> opt1{};
    option<int> opt2{};
    auto cmp = opt1 <=> opt2;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(opt1 == opt2);
    REQUIRE(opt1 <= opt2);
    REQUIRE(opt1 >= opt2);
    REQUIRE(!(opt1 != opt2));
    REQUIRE(!(opt1 < opt2));
    REQUIRE(!(opt1 > opt2));
    opt1.emplace(VALUE);
    cmp = opt1 <=> opt2;
    REQUIRE(cmp == std::strong_ordering::greater);
    REQUIRE(opt1 != opt2);
    REQUIRE(!(opt1 == opt2));
    REQUIRE(!(opt1 < opt2));
    REQUIRE(opt1 > opt2);
    REQUIRE(!(opt1 <= opt2));
    REQUIRE(opt1 >= opt2);
    cmp = opt2 <=> opt1;
    REQUIRE(cmp == std::strong_ordering::less);
    REQUIRE(opt2 != opt1);
    REQUIRE(!(opt2 == opt1));
    REQUIRE(opt2 < opt1);
    REQUIRE(!(opt2 > opt1));
    REQUIRE(opt2 <= opt1);
    REQUIRE(!(opt2 >= opt1));
    opt2.emplace(VALUE);
    cmp = opt1 <=> opt2;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(opt1 == opt2);
    REQUIRE(!(opt1 != opt2));
    REQUIRE(!(opt1 < opt2));
    REQUIRE(!(opt1 > opt2));
    REQUIRE(opt1 >= opt2);
    REQUIRE(opt1 <= opt2);
    opt2.emplace(-VALUE);
    cmp = opt1 <=> opt2;
    REQUIRE(cmp == std::strong_ordering::greater);
    REQUIRE(!(opt1 == opt2));
    REQUIRE(opt1 != opt2);
    REQUIRE(!(opt1 < opt2));
    REQUIRE(opt1 > opt2);
    REQUIRE(!(opt1 <= opt2));
    REQUIRE(opt1 >= opt2);
    cmp = opt2 <=> opt1;
    REQUIRE(cmp == std::strong_ordering::less);
    REQUIRE(!(opt2 == opt1));
    REQUIRE(opt2 != opt1);
    REQUIRE(opt2 < opt1);
    REQUIRE(!(opt2 > opt1));
    REQUIRE(opt2 <= opt1);
    REQUIRE(!(opt2 >= opt1));
}

TEST_CASE("option compare with none", "[option]") {
    option<int> opt{};
    auto cmp = opt <=> none;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(opt == none);
    REQUIRE(!(opt != none));
    REQUIRE(!(opt < none));
    REQUIRE(!(opt > none));
    REQUIRE(opt <= none);
    REQUIRE(opt >= none);
    cmp = none <=> opt;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(none == opt);
    REQUIRE(!(none != opt));
    REQUIRE(!(none < opt));
    REQUIRE(!(none > opt));
    REQUIRE(none <= opt);
    REQUIRE(none >= opt);

    opt.emplace(0);
    cmp = opt <=> none;
    REQUIRE(cmp == std::strong_ordering::greater);
    REQUIRE(!(opt == none));
    REQUIRE(opt != none);
    REQUIRE(!(opt < none));
    REQUIRE(opt > none);
    REQUIRE(!(opt <= none));
    REQUIRE(opt >= none);
    cmp = none <=> opt;
    REQUIRE(cmp == std::strong_ordering::less);
    REQUIRE(!(none == opt));
    REQUIRE(none != opt);
    REQUIRE(none < opt);
    REQUIRE(!(none > opt));
    REQUIRE(none <= opt);
    REQUIRE(!(none >= opt));
}

TEST_CASE("option compare with nullopt", "[option]") {
    option<int> opt{};
    auto cmp = opt <=> std::nullopt;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(opt == std::nullopt);
    REQUIRE(!(opt != std::nullopt));
    REQUIRE(!(opt < std::nullopt));
    REQUIRE(!(opt > std::nullopt));
    REQUIRE(opt <= std::nullopt);
    REQUIRE(opt >= std::nullopt);
    cmp = std::nullopt <=> opt;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(std::nullopt == opt);
    REQUIRE(!(std::nullopt != opt));
    REQUIRE(!(std::nullopt < opt));
    REQUIRE(!(std::nullopt > opt));
    REQUIRE(std::nullopt <= opt);
    REQUIRE(std::nullopt >= opt);

    opt.emplace(0);
    cmp = opt <=> std::nullopt;
    REQUIRE(cmp == std::strong_ordering::greater);
    REQUIRE(!(opt == std::nullopt));
    REQUIRE(opt != std::nullopt);
    REQUIRE(!(opt < std::nullopt));
    REQUIRE(opt > std::nullopt);
    REQUIRE(!(opt <= std::nullopt));
    REQUIRE(opt >= std::nullopt);
    cmp = std::nullopt <=> opt;
    REQUIRE(cmp == std::strong_ordering::less);
    REQUIRE(!(std::nullopt == opt));
    REQUIRE(std::nullopt != opt);
    REQUIRE(std::nullopt < opt);
    REQUIRE(!(std::nullopt > opt));
    REQUIRE(std::nullopt <= opt);
    REQUIRE(!(std::nullopt >= opt));
}

TEST_CASE("option compare with nullptr", "[option]") {
    int i = 0;
    option<int&> opt{};
    auto cmp = opt <=> nullptr;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(opt == nullptr);
    REQUIRE(!(opt != nullptr));
    REQUIRE(!(opt < nullptr));
    REQUIRE(!(opt > nullptr));
    REQUIRE(opt <= nullptr);
    REQUIRE(opt >= nullptr);
    cmp = nullptr <=> opt;
    REQUIRE(cmp == std::strong_ordering::equal);
    REQUIRE(nullptr == opt);
    REQUIRE(!(nullptr != opt));
    REQUIRE(!(nullptr < opt));
    REQUIRE(!(nullptr > opt));
    REQUIRE(nullptr <= opt);
    REQUIRE(nullptr >= opt);

    opt.emplace(i);
    cmp = opt <=> nullptr;
    REQUIRE(cmp == std::strong_ordering::greater);
    REQUIRE(!(opt == nullptr));
    REQUIRE(opt != nullptr);
    REQUIRE(!(opt < nullptr));
    REQUIRE(opt > nullptr);
    REQUIRE(!(opt <= nullptr));
    REQUIRE(opt >= nullptr);
    cmp = nullptr <=> opt;
    REQUIRE(cmp == std::strong_ordering::less);
    REQUIRE(!(nullptr == opt));
    REQUIRE(nullptr != opt);
    REQUIRE(nullptr < opt);
    REQUIRE(!(nullptr > opt));
    REQUIRE(nullptr <= opt);
    REQUIRE(!(nullptr >= opt));
}
