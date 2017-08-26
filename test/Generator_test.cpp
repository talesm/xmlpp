#include "Generator.hpp"
#include <string>
#include "catch.hpp"

using namespace std;
using namespace xmlpp;

constexpr size_t TEST_BUF_SIZE = 1024;

#define RESULT_STR(str) "<?xml version='1.0' encoding='UTF-8'?>" str##s

TEST_CASE("Generator Basics", "[xmlpp][generator][basics]")
{
  char      buffer[TEST_BUF_SIZE];
  Generator g(buffer, TEST_BUF_SIZE);
  SECTION("Empty TAG")
  {
    g.RootTag("root").Close();
    REQUIRE(buffer == RESULT_STR("<root/>"));
  }
  SECTION("Empty TAG 2")
  {
    g.RootTag("other-root").Close();
    REQUIRE(buffer == RESULT_STR("<other-root/>"));
  }
  SECTION("XML HEADER")
  {
    g.Version("1.1").Encoding("ASCII");
    g.RootTag("other-root").Close();
    REQUIRE(buffer == "<?xml version='1.1' encoding='ASCII'?><other-root/>"s);
  }
  SECTION("Tag With Parameters")
  {
    auto tRoot = g.RootTag("root");
    tRoot.AddParameter("param1", "value1");
    tRoot.AddParameter("param2", "value2");
    tRoot.Close();
    REQUIRE(buffer == RESULT_STR("<root param1='value1' param2='value2'/>"));
  }
  SECTION("Tag With Sub tags")
  {
    auto tRoot = g.RootTag("root");
    tRoot.AddTag("subTag").Close();
    tRoot.AddTag("otherTag"); // auto closing
    tRoot.Close();
    REQUIRE(buffer == RESULT_STR("<root><subTag/><otherTag/></root>"));
  }
  SECTION("Text")
  {
    auto tRoot = g.RootTag("root");
    tRoot.AddText("Some random text");
    tRoot.Close();
    REQUIRE(buffer == RESULT_STR("<root>Some random text</root>"));
  }
  SECTION("Escaped text")
  {
    auto tRoot = g.RootTag("root");
    tRoot.AddText("Some <random> text \n&scaped\1\x19");
    tRoot.Close();
    REQUIRE(
      buffer ==
      RESULT_STR(
        "<root>Some &lt;random&gt; text \n&amp;scaped&#x01;&#x19;</root>"));
  }
  SECTION("Comments")
  {
    auto tRoot = g.RootTag("root");
    tRoot.AddComment("Some <random> comment");
    tRoot.Close();
    REQUIRE(buffer == RESULT_STR("<root><!--Some <random> comment--></root>"));
  }
}

TEST_CASE("Generator Errors", "[xmlpp][generator][errors]")
{
  char buffer[TEST_BUF_SIZE];

  SECTION("Too small buffer")
  {
    REQUIRE_THROWS_AS(Generator(buffer, 1).RootTag("r").Close(),
                      GeneratorError);
  }
  Generator g(buffer, TEST_BUF_SIZE);

  SECTION("Parameters after first descendant")
  {
    auto rootTag = g.RootTag("root");
    rootTag.AddText("Hi");
    REQUIRE_THROWS(rootTag.AddParameter("error", "error"));
  }

  SECTION("Add descedant on a closed tag")
  {
    auto rootTag   = g.RootTag("root");
    auto branchTag = rootTag.AddTag("branch");
    rootTag.Close();
    REQUIRE_THROWS(rootTag.AddText("Hi"));
    REQUIRE_THROWS(rootTag.AddTag("Hi"));
    REQUIRE_THROWS(rootTag.AddComment("Hi"));
    REQUIRE_THROWS(branchTag.AddText("Hi")); // TODO: Validate children when
                                             // parent closed.
  }

  SECTION("Check no double rootTag")
  {
    auto rootTag = g.RootTag("root");
    REQUIRE_THROWS(g.RootTag("root2"));
  }
}