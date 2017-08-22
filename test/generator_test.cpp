#include "catch.hpp"
#include "generator.hpp"
#include <string>

using namespace std;
using namespace xmlpp;

constexpr size_t TEST_BUF_SIZE = 1024;

#define RESULT_STR(str) "<?xml version='1.0' encoding='UTF-8'?>" str##s

TEST_CASE("Generator Basics", "[xmlpp][generator][basics]") {
  char buffer[TEST_BUF_SIZE];
  generator g(buffer, TEST_BUF_SIZE);
  SECTION("Empty TAG") {
    g.rootTag("root").close();
    REQUIRE(buffer == RESULT_STR("<root/>"));
  }
  SECTION("Empty TAG 2") {
    g.rootTag("other-root").close();
    REQUIRE(buffer == RESULT_STR("<other-root/>"));
  }
  SECTION("XML HEADER") {
    g.version("1.1").encoding("ASCII");
    g.rootTag("other-root").close();
    REQUIRE(buffer == "<?xml version='1.1' encoding='ASCII'?><other-root/>"s);
  }
  SECTION("Tag With Parameters") {
    auto tRoot = g.rootTag("root");
    tRoot.addParameter("param1", "value1");
    tRoot.addParameter("param2", "value2");
    tRoot.close();
    REQUIRE(buffer == RESULT_STR("<root param1='value1' param2='value2'/>"));
  }
  SECTION("Tag With Sub tags") {
    auto tRoot = g.rootTag("root");
    tRoot.addTag("subTag").close();
    tRoot.addTag("otherTag"); // auto closing
    tRoot.close();
    REQUIRE(buffer == RESULT_STR("<root><subTag/><otherTag/></root>"));
  }
  SECTION("Text") {
    auto tRoot = g.rootTag("root");
    tRoot.addText("Some random text");
    tRoot.close();
    REQUIRE(buffer == RESULT_STR("<root>Some random text</root>"));
  }
  SECTION("Escaped text") {
    auto tRoot = g.rootTag("root");
    tRoot.addText("Some <random> text \n&scaped\1\x19");
    tRoot.close();
    REQUIRE(
        buffer ==
        RESULT_STR(
            "<root>Some &lt;random&gt; text \n&amp;scaped&#x01;&#x19;</root>"));
  }
  SECTION("Comments") {
    auto tRoot = g.rootTag("root");
    tRoot.addComment("Some <random> comment");
    tRoot.close();
    REQUIRE(buffer == RESULT_STR("<root><!--Some <random> comment--></root>"));
  }
}

TEST_CASE("Generator Errors", "[xmlpp][generator][errors]") {
  char buffer[TEST_BUF_SIZE];

  SECTION("Too small buffer") {
    REQUIRE_THROWS_AS(generator(buffer, 1).rootTag("r").close(),
                      generator_error);
  }
  generator g(buffer, TEST_BUF_SIZE);

  SECTION("Parameters after first descendant") {
    auto rootTag = g.rootTag("root");
    rootTag.addText("Hi");
    REQUIRE_THROWS(rootTag.addParameter("error", "error"));
  }

  SECTION("Add descedant on a closed tag") {
    auto rootTag = g.rootTag("root");
    auto branchTag = rootTag.addTag("branch");
    rootTag.close();
    REQUIRE_THROWS(rootTag.addText("Hi"));
    REQUIRE_THROWS(rootTag.addTag("Hi"));
    REQUIRE_THROWS(rootTag.addComment("Hi"));
    REQUIRE_THROWS(branchTag.addText("Hi")); // TODO: Validate children when
    // parent closed.
  }

  SECTION("Check no double rootTag") {
    auto rootTag = g.rootTag("root");
    REQUIRE_THROWS(g.rootTag("root2"));
  }
}