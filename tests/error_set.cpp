#include <catch2/catch_test_macros.hpp>

#include "sumty/error_set.hpp" // IWYU pragma: associated

using namespace sumty;

TEST_CASE("error_set default construct", "[error_set]") {
    const error_set<int> res1{};
    REQUIRE(res1.index() == 0);
}
