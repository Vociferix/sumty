#include <catch2/catch_test_macros.hpp>
#include <type_traits>

#include "sumty/variant.hpp"

using namespace sumty;

struct empty_t {};

template <typename T>
constexpr T max(T value) {
    return value;
}

template <typename T0, typename... TN>
constexpr T0 max(T0 value0, T0 value1, TN... valuen) {
    if (value0 < value1) {
        // NOLINTNEXTLINE(readability-suspicious-call-argument)
        return max(value1, valuen...);
    } else {
        return max(value0, valuen...);
    }
}

TEST_CASE("special variant sizes", "[variant]") {
    STATIC_CHECK(std::is_empty_v<variant<void>>);
    STATIC_CHECK(std::is_empty_v<variant<empty_t>>);
    STATIC_CHECK(sizeof(variant<int>) == sizeof(int));
    STATIC_CHECK(sizeof(variant<int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(variant<void, int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(variant<int&, void>) == sizeof(void*));
    STATIC_CHECK(sizeof(variant<int, float, char, bool>) <=
                 max(sizeof(int), sizeof(float), sizeof(char), sizeof(bool)) * 2);
}

TEST_CASE("variant default construct", "[variant]") {
    variant<int, float&, void> v;
    REQUIRE(v.index() == 0);
    REQUIRE(get<0>(v) == 0);
    REQUIRE(get<int>(v) == 0);
    REQUIRE(v[sumty::index<0>] == 0);
    REQUIRE(v[sumty::type<int>] == 0);
    REQUIRE(holds_alternative<int>(v) == true);
}

TEST_CASE("variant hold optional ref", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr int NEW_VAL = 24;
    int i = INIT_VAL;
    variant<void, int&> v{};
    REQUIRE(v.index() == 0);
    REQUIRE(holds_alternative<void>(v) == true);
    v.template emplace<1>(i);
    REQUIRE(v.index() == 1);
    REQUIRE(holds_alternative<int&>(v) == true);
    REQUIRE(get<1>(v) == INIT_VAL);
    REQUIRE(&get<1>(v) == &i);
    get<1>(v) = NEW_VAL;
    REQUIRE(i == NEW_VAL);
}

TEST_CASE("variant hold general ref", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr int NEW_VAL = 24;
    int i = INIT_VAL;
    variant<void, int&, float> v{};
    REQUIRE(v.index() == 0);
    REQUIRE(holds_alternative<void>(v) == true);
    v.template emplace<1>(i);
    REQUIRE(v.index() == 1);
    REQUIRE(holds_alternative<int&>(v) == true);
    REQUIRE(get<1>(v) == INIT_VAL);
    REQUIRE(&get<1>(v) == &i);
    get<1>(v) = NEW_VAL;
    REQUIRE(i == NEW_VAL);
}

TEST_CASE("variant move", "[variant]") {
    static constexpr int INIT_VAL = 42;
    int i = INIT_VAL;
    variant<void, int&> v1{};
    variant<void, int&, float> v2{};
    v1.template emplace<1>(i);
    v2.template emplace<1>(i);
    // NOLINTNEXTLINE(performance-move-const-arg)
    variant<void, int&> v3{std::move(v1)};
    variant<void, int&, float> v4{std::move(v2)};
    REQUIRE(v3.index() == 1);
    REQUIRE(v4.index() == 1);
    REQUIRE(get<1>(v3) == INIT_VAL);
    REQUIRE(get<1>(v4) == INIT_VAL);
    REQUIRE(&get<1>(v3) == &i);
    REQUIRE(&get<1>(v4) == &i);
    variant<void, int&> v5{};
    variant<void, int&, float> v6{};
    // NOLINTNEXTLINE(performance-move-const-arg)
    v5 = std::move(v3);
    v6 = std::move(v4);
    REQUIRE(v5.index() == 1);
    REQUIRE(v6.index() == 1);
    REQUIRE(get<1>(v5) == INIT_VAL);
    REQUIRE(get<1>(v6) == INIT_VAL);
    REQUIRE(&get<1>(v5) == &i);
    REQUIRE(&get<1>(v6) == &i);
}

TEST_CASE("variant copy", "[variant]") {
    static constexpr int INIT_VAL = 42;
    int i = INIT_VAL;
    variant<void, int&> v1{};
    variant<void, int&, float> v2{};
    v1.template emplace<1>(i);
    v2.template emplace<1>(i);
    variant<void, int&> v3{v1};
    variant<void, int&, float> v4{v2};
    REQUIRE(v3.index() == 1);
    REQUIRE(v4.index() == 1);
    REQUIRE(get<1>(v3) == INIT_VAL);
    REQUIRE(get<1>(v4) == INIT_VAL);
    REQUIRE(&get<1>(v3) == &i);
    REQUIRE(&get<1>(v4) == &i);
    variant<void, int&> v5{};
    variant<void, int&, float> v6{};
    v5 = v3;
    v6 = v4;
    REQUIRE(v5.index() == 1);
    REQUIRE(v6.index() == 1);
    REQUIRE(get<1>(v5) == INIT_VAL);
    REQUIRE(get<1>(v6) == INIT_VAL);
    REQUIRE(&get<1>(v5) == &i);
    REQUIRE(&get<1>(v6) == &i);
}

TEST_CASE("variant visit method", "[variant]") {
    static constexpr int INIT_VAL = 42;
    int i = INIT_VAL;
    variant<void, int&, float> v1{};
    v1.template emplace<1>(i);
    REQUIRE(v1.visit(overload([]() -> int { return 24; },
                              [](auto val) { return static_cast<int>(val * 2); })) ==
            INIT_VAL * 2);
}

TEST_CASE("variant visit function", "[variant]") {
    static constexpr int INIT_VAL = 42;
    int i = INIT_VAL;
    variant<void, int&, float> v1{};
    v1.template emplace<1>(i);
    REQUIRE(visit(overload([]() -> int { return 24; },
                           [](auto val) { return static_cast<int>(val * 2); }),
                  v1) == INIT_VAL * 2);
}

TEST_CASE("multi variant visit", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    int i = INIT_VAL;
    variant<void, int&, float> v1{};
    variant<float, bool, int&> v2{};
    v1.template emplace<1>(i);
    v2.template emplace<0>(INIT_FLT);
    REQUIRE(visit(overload([](auto&& rhs) { return static_cast<int>(rhs); },
                           [](auto&& lhs, auto&& rhs) {
                               return static_cast<int>(lhs) + static_cast<int>(rhs);
                           }),
                  v1, v2) == INIT_VAL + static_cast<int>(INIT_FLT));
}

TEST_CASE("variant get_if", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    int i = INIT_VAL;
    variant<void, int&, float, empty_t> v1{};
    STATIC_CHECK(std::is_void_v<decltype(get_if<0>(v1))>);
    REQUIRE(get_if<1>(v1) == nullptr);
    REQUIRE(get_if<2>(v1) == nullptr);
    REQUIRE(get_if<3>(v1) == nullptr);
    v1.template emplace<1>(i);
    REQUIRE(get_if<1>(v1) == &i);
    REQUIRE(get_if<2>(v1) == nullptr);
    REQUIRE(get_if<3>(v1) == nullptr);
    v1.template emplace<2>(INIT_FLT);
    REQUIRE(get_if<1>(v1) == nullptr);
    REQUIRE(get_if<2>(v1) != nullptr);
    REQUIRE(*get_if<2>(v1) == INIT_FLT);
    REQUIRE(get_if<3>(v1) == nullptr);
    v1.template emplace<3>();
    REQUIRE(get_if<1>(v1) == nullptr);
    REQUIRE(get_if<2>(v1) == nullptr);
    REQUIRE(get_if<3>(v1) != nullptr);
    STATIC_CHECK(std::is_same_v<empty_t*, decltype(get_if<3>(v1))>);
}

TEST_CASE("variant get by type", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    int i = INIT_VAL;
    variant<void, int&, float, empty_t> v1{};
    REQUIRE(holds_alternative<void>(v1) == true);
    REQUIRE(holds_alternative<int&>(v1) == false);
    REQUIRE(holds_alternative<float>(v1) == false);
    REQUIRE(holds_alternative<empty_t>(v1) == false);
    STATIC_CHECK(std::is_void_v<decltype(get<void>(v1))>);
    v1.template emplace<int&>(i);
    REQUIRE(holds_alternative<void>(v1) == false);
    REQUIRE(holds_alternative<int&>(v1) == true);
    REQUIRE(holds_alternative<float>(v1) == false);
    REQUIRE(holds_alternative<empty_t>(v1) == false);
    REQUIRE(get<int&>(v1) == INIT_VAL);
    REQUIRE(&get<int&>(v1) == &i);
    v1.template emplace<float>(INIT_FLT);
    REQUIRE(holds_alternative<void>(v1) == false);
    REQUIRE(holds_alternative<int&>(v1) == false);
    REQUIRE(holds_alternative<float>(v1) == true);
    REQUIRE(holds_alternative<empty_t>(v1) == false);
    REQUIRE(get<float>(v1) == INIT_FLT);
    v1.template emplace<empty_t>();
    REQUIRE(holds_alternative<void>(v1) == false);
    REQUIRE(holds_alternative<int&>(v1) == false);
    REQUIRE(holds_alternative<float>(v1) == false);
    REQUIRE(holds_alternative<empty_t>(v1) == true);
    STATIC_CHECK(std::is_same_v<empty_t&, decltype(get<empty_t>(v1))>);
}

TEST_CASE("variant get_if by type", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    int i = INIT_VAL;
    variant<void, int&, float, empty_t> v1{};
    STATIC_CHECK(std::is_void_v<decltype(get_if<void>(v1))>);
    REQUIRE(get_if<int&>(v1) == nullptr);
    REQUIRE(get_if<float>(v1) == nullptr);
    REQUIRE(get_if<empty_t>(v1) == nullptr);
    v1.template emplace<1>(i);
    REQUIRE(get_if<int&>(v1) == &i);
    REQUIRE(get_if<float>(v1) == nullptr);
    REQUIRE(get_if<empty_t>(v1) == nullptr);
    v1.template emplace<2>(INIT_FLT);
    REQUIRE(get_if<int&>(v1) == nullptr);
    REQUIRE(get_if<float>(v1) != nullptr);
    REQUIRE(*get_if<float>(v1) == INIT_FLT);
    REQUIRE(get_if<empty_t>(v1) == nullptr);
    v1.template emplace<3>();
    REQUIRE(get_if<int&>(v1) == nullptr);
    REQUIRE(get_if<float>(v1) == nullptr);
    REQUIRE(get_if<empty_t>(v1) != nullptr);
    STATIC_CHECK(std::is_same_v<empty_t*, decltype(get_if<empty_t>(v1))>);
}

TEST_CASE("variant square bracket index", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    int i = INIT_VAL;
    variant<void, int&, float, empty_t> v1{};
    STATIC_CHECK(std::is_void_v<decltype(v1[index_v<0>])>);
    STATIC_CHECK(std::is_void_v<decltype(index_v<0>(v1))>);
    v1.template emplace<1>(i);
    REQUIRE(v1[index_v<1>] == INIT_VAL);
    REQUIRE(&v1[index_v<1>] == &i);
    REQUIRE(index_v<1>(v1) == INIT_VAL);
    REQUIRE(&index_v<1>(v1) == &i);
    v1.template emplace<2>(INIT_FLT);
    REQUIRE(v1[index_v<2>] == INIT_FLT);
    REQUIRE(index_v<2>(v1) == INIT_FLT);
    v1.template emplace<3>();
    STATIC_CHECK(std::is_same_v<empty_t&, decltype(v1[index_v<3>])>);
    STATIC_CHECK(std::is_same_v<empty_t&, decltype(index_v<3>(v1))>);
}

TEST_CASE("variant square bracket type", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    int i = INIT_VAL;
    variant<void, int&, float, empty_t> v1{};
    STATIC_CHECK(std::is_void_v<decltype(v1[type<void>])>);
    STATIC_CHECK(std::is_void_v<decltype(type<void>(v1))>);
    v1.template emplace<1>(i);
    REQUIRE(v1[type<int&>] == INIT_VAL);
    REQUIRE(&v1[type<int&>] == &i);
    REQUIRE(type<int&>(v1) == INIT_VAL);
    REQUIRE(&type<int&>(v1) == &i);
    v1.template emplace<2>(INIT_FLT);
    REQUIRE(v1[type<float>] == INIT_FLT);
    REQUIRE(type<float>(v1) == INIT_FLT);
    v1.template emplace<3>();
    STATIC_CHECK(std::is_same_v<empty_t&, decltype(v1[type<empty_t>])>);
    STATIC_CHECK(std::is_same_v<empty_t&, decltype(type<empty_t>(v1))>);
}

TEST_CASE("variant swap", "[variant]") {
    static constexpr int INIT_VAL = 42;
    static constexpr float INIT_FLT = 3.14F;
    static constexpr float INIT_FLT_2 = 1.23F;
    int i = INIT_VAL;
    variant<void, int&, float, empty_t> v1{};
    variant<void, int&, float, empty_t> v2{};
    swap(v1, v2);
    REQUIRE(v1.index() == 0);
    REQUIRE(v2.index() == 0);
    v1.template emplace<1>(i);
    swap(v1, v2);
    REQUIRE(v1.index() == 0);
    REQUIRE(v2.index() == 1);
    REQUIRE(get<1>(v2) == INIT_VAL);
    REQUIRE(&get<1>(v2) == &i);
    v1.template emplace<2>(INIT_FLT);
    swap(v1, v2);
    REQUIRE(v1.index() == 1);
    REQUIRE(v2.index() == 2);
    REQUIRE(get<1>(v1) == INIT_VAL);
    REQUIRE(&get<1>(v1) == &i);
    REQUIRE(get<2>(v2) == INIT_FLT);
    v1.template emplace<2>(INIT_FLT_2);
    swap(v1, v2);
    REQUIRE(v1.index() == 2);
    REQUIRE(v2.index() == 2);
    REQUIRE(get<2>(v1) == INIT_FLT);
    REQUIRE(get<2>(v2) == INIT_FLT_2);
}

// XXX: The below headers are included to make sure they get checked
//      by include-what-you-use.

#include "sumty/detail/auto_union.hpp"   // IWYU pragma: associated
#include "sumty/detail/fwd.hpp"          // IWYU pragma: associated
#include "sumty/detail/traits.hpp"       // IWYU pragma: associated
#include "sumty/detail/utils.hpp"        // IWYU pragma: associated
#include "sumty/detail/variant_impl.hpp" // IWYU pragma: associated
#include "sumty/exceptions.hpp"          // IWYU pragma: associated
#include "sumty/impl/variant.hpp"        // IWYU pragma: associated
#include "sumty/utils.hpp"               // IWYU pragma: associated
