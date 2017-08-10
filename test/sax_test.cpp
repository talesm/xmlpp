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

TEST_CASE("Texts", "[xmlpp][sax][texts]") {
  REQUIRE(sax("Some text").type() == entity_type::TEXT);
  CHECK(sax("Some text").value() == "Some text");
  REQUIRE(sax("  Some text").type() == entity_type::TEXT);
  CHECK(sax("  Some text").value() == "  Some text");
  sax s("  <root>Some text<branch/>Other text</root>");
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TEXT);
  REQUIRE((s++).type() == entity_type::TAG);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
  REQUIRE((s++).type() == entity_type::TEXT);
  REQUIRE((s++).type() == entity_type::TAG_ENDING);
}

TEST_CASE("Text with escaping", "[xmlpp][sax][texts]") {
  REQUIRE(sax("text&apos;s &lt;&quot;escaped&quot;&gt; &amp; quoted").value() ==
          "text's <\"escaped\"> & quoted");
  REQUIRE(sax("text&#32;with&#x20;spaces").value() == "text with spaces");
  REQUIRE(sax("I &lt;3 J&#xF6;rg").value() == "I <3 Jörg");
}

TEST_CASE("Xml declartion", "[xmlpp][sax][declaration]") {
  REQUIRE(sax("<?xml version='1.0' encoding='UTF-8'?><root/>").value() ==
          "root");
  REQUIRE(sax("<?xml version='1.0' encoding='UTF-8'?>text").value() == "text");
  REQUIRE(sax("<?xml version='1.0' encoding='UTF-8'?>text").encoding() ==
          "UTF-8");
  REQUIRE(sax("<?xml version='1.0' encoding='UTF-8'?>text").version() == "1.0");
  REQUIRE(sax("<?xml version='1.1' encoding='UTF-8'?>text").version() == "1.1");
}

// TODO: entities.
// TODO: CDATA
// TODO: filters
// TODO: CHECK for invalid symbols in tags.
