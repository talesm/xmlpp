#include "catch.hpp"
#include "parser.hpp"
#include <cstddef>

using namespace xmlpp;
using namespace std;

TEST_CASE("Tags", "[xmlpp][parser][tags]") {
  parser s("<root/>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().size() == 0);
  REQUIRE(parser("<notroot/>").value() == "notroot");
  REQUIRE(parser("<root />").value() == "root");
  REQUIRE(parser("<root></root>").value() == "root");
}
TEST_CASE("Tag Error", "[xmlpp][parser][tags][error]") {
  REQUIRE_THROWS_AS(parser("<root"), parser_error);
  // TODO: Other invalid chars.
}

TEST_CASE("Tags with parameters", "[xmlpp][parser][tags]") {
  parser s(
      "<root param1=\"ahoy\" param2=\"test&apos;s test\" párêmçï='test'/>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().at("param1") == "ahoy");
  REQUIRE(s.params().at("párêmçï") == "test");
  REQUIRE(s.params().at("param2") == "test's test");
  REQUIRE(s.params().size() == 3);
}

TEST_CASE("Tags within tags", "[xmlpp][parser][tags]") {
  parser s("<root><branch/></root>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  s.next();
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "branch");
}
TEST_CASE("Tag closing", "[xmlpp][parser][tags]") {
  REQUIRE((++parser("<root></root>")).type() == entity_type::TAG_ENDING);
  REQUIRE((++parser("<root/>")).type() == entity_type::TAG_ENDING);
  parser s("<root><branch/><branch></branch></root>");
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE(s.type() == entity_type::TAG_ENDING);
}
TEST_CASE("Tags closing mismatch", "[xmlpp][parser][tags]") {
  REQUIRE_THROWS_AS(++++parser("<root></notroot>"), parser_error);
}

TEST_CASE("Comments", "[xmlpp][parser][comments]") {
  REQUIRE(parser("<!-- test comment -->").type() == entity_type::COMMENT);
  CHECK(parser("<!-- test comment -->").value() == " test comment ");
  REQUIRE(parser("<!--- test comment --->").type() == entity_type::COMMENT);
  CHECK(parser("<!--- test comment --->").value() == "- test comment -");
  parser s("<!-- Begin--><root><!--branch--><branch/></root><!--End -->");
  REQUIRE((s++).type() == entity_type::COMMENT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::COMMENT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::COMMENT);
}

TEST_CASE("Texts", "[xmlpp][parser][texts]") {
  REQUIRE(parser("Some text").type() == entity_type::TEXT);
  CHECK(parser("Some text").value() == "Some text");
  REQUIRE(parser("  Some text").type() == entity_type::TEXT);
  CHECK(parser("  Some text").value() == "  Some text");
  parser s("  <root>Some text<branch/>Other text</root>");
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TEXT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TEXT);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
}

TEST_CASE("Text with escaping", "[xmlpp][parser][texts]") {
  REQUIRE(
      parser("text&apos;s &lt;&quot;escaped&quot;&gt; &amp; quoted").value() ==
      "text's <\"escaped\"> & quoted");
  REQUIRE(parser("text&#32;with&#x20;spaces").value() == "text with spaces");
  REQUIRE(parser("I &lt;3 J&#xF6;rg").value() == "I <3 Jörg");
  REQUIRE(parser("<![CDATA[<\"Escaped's\">]]>").value() == "<\"Escaped's\">");
  REQUIRE(parser("between <![CDATA[<\"Escaped\">]]> text").value() ==
          "between <\"Escaped\"> text");
}

TEST_CASE("Xml declartion", "[xmlpp][parser][declaration]") {
  REQUIRE(parser("<?xml version='1.0' encoding='UTF-8'?><root/>").value() ==
          "root");
  REQUIRE(parser("<?xml version='1.0' encoding='UTF-8'?>text").value() ==
          "text");
  REQUIRE(parser("<?xml version='1.0' encoding='UTF-8'?>text").encoding() ==
          "UTF-8");
  REQUIRE(parser("<?xml version='1.0' encoding='UTF-8'?>text").version() ==
          "1.0");
  REQUIRE(parser("<?xml version='1.1' encoding='UTF-8'?>text").version() ==
          "1.1");
}

// TODO: filters
// TODO: CHECK for invalid symbols in tags.
