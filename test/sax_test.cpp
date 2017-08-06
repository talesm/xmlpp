#include "catch.hpp"
#include "sax.hpp"
#include <stddef.h>

using namespace xmlpp;

TEST_CASE("Tags", "[xmlpp][sax][tags]") {
  sax s("<root/>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().size() == 0);
  REQUIRE(sax("<notroot/>").value() == "notroot");
  REQUIRE(sax("<root />").value() == "root");
  REQUIRE(sax("<root></root>").value() == "root");
}
TEST_CASE("Tags with parameters", "[xmlpp][sax][tags]") {
  sax s("<root param1=\"ahoy\" param2=\"test\" párêmçï='test'/>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().at("param1") == "ahoy");
  REQUIRE(s.params().size() == 3);
}

TEST_CASE("Tags with tags", "[xmlpp][sax][tags]") {
  sax s("<root><branch/></root>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  s.next();
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "branch");
}
TEST_CASE("Tag closing", "[xmlpp][sax][tags]") {
  REQUIRE((++sax("<root></root>")).type() == entity_type::TAG_ENDING);
  REQUIRE((++sax("<root/>")).type() == entity_type::TAG_ENDING);
  sax s("<root><branch/><branch></branch></root>");
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE(s.type() == entity_type::TAG_ENDING);
}

TEST_CASE("Comments", "[xmlpp][sax][comments]") {
  REQUIRE(sax("<!-- test comment -->").type() == entity_type::COMMENT);
  CHECK(sax("<!-- test comment -->").value() == " test comment ");
  REQUIRE(sax("<!--- test comment --->").type() == entity_type::COMMENT);
  CHECK(sax("<!--- test comment --->").value() == "- test comment -");
  sax s("<!-- Begin--><root><!--branch--><branch/></root><!--End -->");
  REQUIRE((s++).type() == entity_type::COMMENT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::COMMENT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::COMMENT);
}
