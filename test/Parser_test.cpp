#include "Parser.hpp"
#include <cstddef>
#include "catch.hpp"

using namespace xmlpp;
using namespace std;

TEST_CASE("Tags", "[xmlpp][parser][tags]")
{
  Parser s("<root/>");
  REQUIRE(s.Type() == EntityType::TAG);
  REQUIRE(s.Value() == "root");
  REQUIRE(s.Parameters().size() == 0);
  REQUIRE(Parser("<notroot/>").Value() == "notroot");
  REQUIRE(Parser("<root />").Value() == "root");
  REQUIRE(Parser("<root></root>").Value() == "root");
}
TEST_CASE("Tag Error", "[xmlpp][parser][tags][error]")
{
  REQUIRE_THROWS_AS(Parser("<root"), parser_error);
  // TODO: Other invalid chars.
}

TEST_CASE("Tags with parameters", "[xmlpp][parser][tags]")
{
  Parser s(
    "<root param1=\"ahoy\" param2=\"test&apos;s test\" párêmçï='test'/>");
  REQUIRE(s.Type() == EntityType::TAG);
  REQUIRE(s.Value() == "root");
  REQUIRE(s.Parameters().at("param1") == "ahoy");
  REQUIRE(s.Parameters().at("párêmçï") == "test");
  REQUIRE(s.Parameters().at("param2") == "test's test");
  REQUIRE(s.Parameters().size() == 3);
}

TEST_CASE("Tags within tags", "[xmlpp][parser][tags]")
{
  Parser s("<root><branch/></root>");
  REQUIRE(s.Type() == EntityType::TAG);
  REQUIRE(s.Value() == "root");
  s.Next();
  REQUIRE(s.Type() == EntityType::TAG);
  REQUIRE(s.Value() == "branch");
}
TEST_CASE("Tag closing", "[xmlpp][parser][tags]")
{
  REQUIRE((++Parser("<root></root>")).Type() == EntityType::TAG_ENDING);
  REQUIRE((++Parser("<root/>")).Type() == EntityType::TAG_ENDING);
  Parser s("<root><branch/><branch></branch></root>");
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::TAG_ENDING);
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::TAG_ENDING);
  REQUIRE(s.Type() == EntityType::TAG_ENDING);
}
TEST_CASE("Tags closing mismatch", "[xmlpp][parser][tags]")
{
  REQUIRE_THROWS_AS(++++Parser("<root></notroot>"), parser_error);
}

TEST_CASE("Comments", "[xmlpp][parser][comments]")
{
  REQUIRE(Parser("<!-- test comment -->").Type() == EntityType::COMMENT);
  CHECK(Parser("<!-- test comment -->").Value() == " test comment ");
  REQUIRE(Parser("<!--- test comment --->").Type() == EntityType::COMMENT);
  CHECK(Parser("<!--- test comment --->").Value() == "- test comment -");
  Parser s("<!-- Begin--><root><!--branch--><branch/></root><!--End -->");
  REQUIRE((s++).Type() == EntityType::COMMENT);
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::COMMENT);
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::TAG_ENDING);
  REQUIRE((s++).Type() == EntityType::TAG_ENDING);
  REQUIRE((s++).Type() == EntityType::COMMENT);
}

TEST_CASE("Texts", "[xmlpp][parser][texts]")
{
  REQUIRE(Parser("Some text").Type() == EntityType::TEXT);
  CHECK(Parser("Some text").Value() == "Some text");
  REQUIRE(Parser("  Some text").Type() == EntityType::TEXT);
  CHECK(Parser("  Some text").Value() == "  Some text");
  Parser s("  <root>Some text<branch/>Other text</root>");
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::TEXT);
  REQUIRE((s++).Type() == EntityType::TAG);
  REQUIRE((s++).Type() == EntityType::TAG_ENDING);
  REQUIRE((s++).Type() == EntityType::TEXT);
  REQUIRE((s++).Type() == EntityType::TAG_ENDING);
}

TEST_CASE("Text with escaping", "[xmlpp][parser][texts]")
{
  REQUIRE(
    Parser("text&apos;s &lt;&quot;escaped&quot;&gt; &amp; quoted").Value() ==
    "text's <\"escaped\"> & quoted");
  REQUIRE(Parser("text&#32;with&#x20;spaces").Value() == "text with spaces");
  REQUIRE(Parser("I &lt;3 J&#xF6;rg").Value() == "I <3 Jörg");
  REQUIRE(Parser("<![CDATA[<\"Escaped's\">]]>").Value() == "<\"Escaped's\">");
  REQUIRE(Parser("between <![CDATA[<\"Escaped\">]]> text").Value() ==
          "between <\"Escaped\"> text");
}

TEST_CASE("Xml declartion", "[xmlpp][parser][declaration]")
{
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?><root/>").Value() ==
          "root");
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?>text").Value() ==
          "text");
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?>text").Encoding() ==
          "UTF-8");
  REQUIRE(Parser("<?xml version='1.0' encoding='UTF-8'?>text").Version() ==
          "1.0");
  REQUIRE(Parser("<?xml version='1.1' encoding='UTF-8'?>text").Version() ==
          "1.1");
}
