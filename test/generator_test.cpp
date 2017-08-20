#include "catch.hpp"
#include "generator.hpp"
#include <string>

using namespace std;
using namespace xmlpp;

constexpr size_t TEST_BUF_SIZE = 1024;

#define RESULT_STR(str) "<?xml version='1.0' encoding='UTF-8'?>" str##s

TEST_CASE("Generator Basics", "[xmlpp][basics]") {
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
    tRoot.close();
    REQUIRE(buffer == RESULT_STR("<root><subTag/></root>"));
  }
}
