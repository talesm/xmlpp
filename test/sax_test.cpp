#include "catch.hpp"
#include "sax.hpp"

using namespace xmlpp;

TEST_CASE("Placeholder", "[xmlpp][sax]"){
    REQUIRE(1==1);
    sax s("<root/>");
}
