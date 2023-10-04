#include <catch2/catch_test_macros.hpp>
#include <type_traits>

#include "sumty/variant.hpp"

using namespace sumty;

struct empty_t {};

TEST_CASE("special variant sizes", "[variant]") {
    STATIC_CHECK(std::is_empty_v<variant<void>>);
    STATIC_CHECK(std::is_empty_v<variant<empty_t>>);
    STATIC_CHECK(sizeof(variant<int>) == sizeof(int));
    STATIC_CHECK(sizeof(variant<int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(variant<void, int&>) == sizeof(void*));
    STATIC_CHECK(sizeof(variant<int&, void>) == sizeof(void*));
    STATIC_CHECK(sizeof(variant<int, float, char, bool>) <= sizeof(int) * 2);
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

#include "sumty/detail/auto_union.hpp"   // IWYU pragma: associated
#include "sumty/detail/fwd.hpp"          // IWYU pragma: associated
#include "sumty/detail/traits.hpp"       // IWYU pragma: associated
#include "sumty/detail/utils.hpp"        // IWYU pragma: associated
#include "sumty/detail/variant_impl.hpp" // IWYU pragma: associated
#include "sumty/exceptions.hpp"          // IWYU pragma: associated
#include "sumty/impl/variant.hpp"        // IWYU pragma: associated
#include "sumty/utils.hpp"               // IWYU pragma: associated
