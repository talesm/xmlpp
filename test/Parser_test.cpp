#include "Parser.hpp"
#include <cstddef>
#include "catch.hpp"

using namespace xmlpp;
using namespace std;

TEST_CASE("Tags", "[xmlpp][parser][tags]")
{
  Parser s("<root/>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().size() == 0);
  REQUIRE(Parser("<notroot/>").value() == "notroot");
  REQUIRE(Parser("<root />").value() == "root");
  REQUIRE(Parser("<root></root>").value() == "root");
}
TEST_CASE("Tag Error", "[xmlpp][parser][tags][error]")
{
  REQUIRE_THROWS_AS(Parser("<root"), ParserError);
  // TODO: Other invalid chars.
}

TEST_CASE("Tags with parameters", "[xmlpp][parser][tags]")
{
  Parser s(
    "<root param1=\"ahoy\" param2=\"test&apos;s test\" párêmçï='test'/>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  REQUIRE(s.params().at("param1") == "ahoy");
  REQUIRE(s.params().at("párêmçï") == "test");
  REQUIRE(s.params().at("param2") == "test's test");
  REQUIRE(s.params().size() == 3);
}

TEST_CASE("Tags within tags", "[xmlpp][parser][tags]")
{
  Parser s("<root><branch/></root>");
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "root");
  s.next();
  REQUIRE(s.type() == entity_type::TAG);
  REQUIRE(s.value() == "branch");
}
TEST_CASE("Tag closing", "[xmlpp][parser][tags]")
{
  REQUIRE((++Parser("<root></root>")).type() == entity_type::TAG_ENDING);
  REQUIRE((++Parser("<root/>")).type() == entity_type::TAG_ENDING);
  Parser s("<root><branch/><branch></branch></root>");
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE(s.type() == entity_type::TAG_ENDING);
}
TEST_CASE("Tags closing mismatch", "[xmlpp][parser][tags]")
{
  REQUIRE_THROWS_AS(++++Parser("<root></notroot>"), ParserError);
}

TEST_CASE("Comments", "[xmlpp][parser][comments]")
{
  REQUIRE(Parser("<!-- test comment -->").type() == entity_type::COMMENT);
  CHECK(Parser("<!-- test comment -->").value() == " test comment ");
  REQUIRE(Parser("<!--- test comment --->").type() == entity_type::COMMENT);
  CHECK(Parser("<!--- test comment --->").value() == "- test comment -");
  Parser s("<!-- Begin--><root><!--branch--><branch/></root><!--End -->");
  REQUIRE((s++).type() == entity_type::COMMENT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::COMMENT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::COMMENT);
}

TEST_CASE("Texts", "[xmlpp][parser][texts]")
{
  REQUIRE(Parser("Some text").type() == entity_type::TEXT);
  CHECK(Parser("Some text").value() == "Some text");
  REQUIRE(Parser("  Some text").type() == entity_type::TEXT);
  CHECK(Parser("  Some text").value() == "  Some text");
  Parser s("  <root>Some text<branch/>Other text</root>");
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TEXT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TEXT);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
}

TEST_CASE("Text with escaping", "[xmlpp][parser][texts]")
{
  REQUIRE(
    Parser("text&apos;s &lt;&quot;escaped&quot;&gt; &amp; quoted").value() ==
    "text's <\"escaped\"> & quoted");
  REQUIRE(Parser("text&#32;with&#x20;spaces").value() == "text with spaces");
  REQUIRE(Parser("I &lt;3 J&#xF6;rg").value() == "I <3 Jörg");
  REQUIRE(Parser("<![CDATA[<\"Escaped's\">]]>").value() == "<\"Escaped's\">");
  REQUIRE(Parser("between <![CDATA[<\"Escaped\">]]> text").value() ==
          "between <\"Escaped\"> text");
}

TEST_CASE("Xml declartion", "[xmlpp][parser][declaration]")
{
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?><root/>").value() ==
          "root");
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?>text").value() ==
          "text");
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?>text").encoding() ==
          "UTF-8");
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?>text").version() ==
          "1.0");
  REQUIRE(Parser("<?xml version='1.1' encoding='UTF-8'?>text").version() ==
          "1.1");
}
