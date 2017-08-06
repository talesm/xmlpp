#include "catch.hpp"
#include "sax.hpp"
#include <stddef.h>

using namespace xmlpp;

TEST_CASE("Tags", "[xmlpp][sax][tags]") {
  sax s("<root/>");
  REQUIRE(s.type() == entity_type::tag);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().size() == 0);
  REQUIRE(sax("<notroot/>").value() == "notroot");
  REQUIRE(sax("<root />").value() == "root");
  REQUIRE(sax("<root></root>").value() == "root");
}
TEST_CASE("Tags with parameters", "[xmlpp][sax][tags]") {
  sax s("<root param1=\"ahoy\" param2=\"test\" párêmçï='test'/>");
  REQUIRE(s.type() == entity_type::tag);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().at("param1") == "ahoy");
  REQUIRE(s.params().size() == 3);
}
