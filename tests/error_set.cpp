#include <catch2/catch_test_macros.hpp>

#include "sumty/error_set.hpp" // IWYU pragma: associated

using namespace sumty;

template <size_t ID>
struct myerr {
    int value{};
};

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

TEST_CASE("special error_set sizes", "[error_set]") {
    STATIC_CHECK(std::is_empty_v<error_set<void>>);
    STATIC_CHECK(std::is_empty_v<error_set<empty_t>>);
    STATIC_CHECK(sizeof(error_set<empty_t, int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(error_set<int&, empty_t>) == sizeof(void*));
    STATIC_CHECK(sizeof(error_set<int>) == sizeof(int));
    STATIC_CHECK(sizeof(error_set<int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(error_set<void, int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(error_set<int&, void>) == sizeof(void*));
    STATIC_CHECK(sizeof(error_set<int, float, char, bool>) <=
                 max(sizeof(int), sizeof(float), sizeof(char), sizeof(bool)) * 2);
}

TEST_CASE("error_set default construct", "[error_set]") {
    const error_set<myerr<0>> res1{};
    REQUIRE(res1.index() == 0);
    REQUIRE(get<0>(res1).value == 0);
    const error_set<myerr<0>, myerr<1>> res2{};
    REQUIRE(res2.index() == 0);
    REQUIRE(get<0>(res2).value == 0);
}

TEST_CASE("error_set construct in place", "[error_set]") {
    error_set<myerr<0>, myerr<1>, myerr<2>> e{in_place_index<1>, 42};
    REQUIRE(e.index() == 1);
    REQUIRE(holds_alternative<myerr<1>>(e));
    REQUIRE(get<1>(e).value == 42);
}

TEST_CASE("error_set emplace construct", "[error_set]") {
    error_set<myerr<0>, myerr<1>, myerr<2>> e1 = myerr<1>{42};
    REQUIRE(e1.index() == 1);
    REQUIRE(holds_alternative<myerr<1>>(e1));
    REQUIRE(get<1>(e1).value == 42);
}

TEST_CASE("error_set value assignment", "[error_set]") {
    error_set<myerr<0>, myerr<1>, myerr<2>> e1{};
    e1 = myerr<1>{42};
    REQUIRE(e1.index() == 1);
    REQUIRE(holds_alternative<myerr<1>>(e1) == true);
    REQUIRE(get<1>(e1).value == 42);
}

TEST_CASE("error_set visit method", "[error_set]") {
    error_set<myerr<0>, myerr<1>, myerr<2>> e1 = myerr<1>{42};
    e1.visit(overload([]([[maybe_unused]] myerr<0> err) {
        REQUIRE(false);
    }, [](myerr<1> err) {
        REQUIRE(err.value == 42);
    }, []([[maybe_unused]] myerr<2> err) {
        REQUIRE(false);
    }));
}

TEST_CASE("error_set visit function", "[error_set]") {
    error_set<myerr<0>, myerr<1>, myerr<2>> e1 = myerr<1>{42};
    visit(overload([]([[maybe_unused]] myerr<0> err) {
        REQUIRE(false);
    }, [](myerr<1> err) {
        REQUIRE(err.value == 42);
    }, []([[maybe_unused]] myerr<2> err) {
        REQUIRE(false);
    }), e1);
}

TEST_CASE("error_set construct from subset", "[error_set]") {
    error_set<myerr<2>, myerr<0>> e1 = myerr<2>{42};
    error_set<myerr<0>, myerr<1>, myerr<2>> e2 = e1;
    REQUIRE(e2.index() == 2);
    REQUIRE(holds_alternative<myerr<2>>(e2));
    REQUIRE(get<2>(e2).value == 42);

    error_set<myerr<1>, myerr<2>, myerr<0>> e3 = e2;
    REQUIRE(e3.index() == 1);
    REQUIRE(holds_alternative<myerr<2>>(e3));
    REQUIRE(get<1>(e3).value == 42);
}

TEST_CASE("error_set assign from subset", "[error_set]") {
    error_set<myerr<2>, myerr<0>> e1 = myerr<2>{42};
    error_set<myerr<0>, myerr<1>, myerr<2>> e2{};
    e2 = e1;
    REQUIRE(e2.index() == 2);
    REQUIRE(holds_alternative<myerr<2>>(e2));
    REQUIRE(get<2>(e2).value == 42);

    error_set<myerr<1>, myerr<2>, myerr<0>> e3{};
    e3 = e2;
    REQUIRE(e3.index() == 1);
    REQUIRE(holds_alternative<myerr<2>>(e3));
    REQUIRE(get<1>(e3).value == 42);
}
