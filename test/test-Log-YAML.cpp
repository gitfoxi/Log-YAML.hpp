
#include "../Log-YAML.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <vector>
#include <boost/assign.hpp>

using namespace std;
using namespace boost::assign;

TEST_CASE("Scalars", "[Log]")
{
  Log::Log log("log", true);

  // TODO head

  SECTION("head") {
    REQUIRE(log.head() ==
            string("\"log\":\n"));
  }

  SECTION("int") {
    REQUIRE(log.log(1) ==
            string("  \"0\": 1\n"));
  }

  SECTION("double") {
    REQUIRE(log.log(1.1) ==
            string("  \"0\": 1.1\n"));
  }

  SECTION("char*") {
    REQUIRE(log.log((char*)"a") ==
            string("  \"0\": \"a\"\n"));
  }

  SECTION("string") {
    REQUIRE(log.log(string("a")) ==
            string("  \"0\": \"a\"\n"));
  }
}

TEST_CASE("vector", "[Log]")
{
  Log::Log log("log", true);

  SECTION("vector int []") {
    vector<int> v;
    REQUIRE(log.log(v) ==
            string("  \"0\": []\n"));
  }

  SECTION("vector int [1]") {
    vector<int> v;
    v += 1;
    REQUIRE(log.log(v) ==
            string("  \"0\": [1]\n"));
  }

  SECTION("vector int [1, 2]") {
    vector<int> v;
    v += 1, 2;
    REQUIRE(log.log(v) ==
            string("  \"0\": [1, 2]\n"));
  }

  SECTION("vector double[]") {
    vector<double> v;
      REQUIRE(log.log(v) ==
              string("  \"0\": []\n"));
  }

  SECTION("vector double [1.1]") {
    vector<double> v;
    v += 1.1;
    REQUIRE(log.log(v) ==
            string("  \"0\": [1.1]\n"));
  }

  SECTION("vector double [1.1, 2.2]") {
    vector<double> v;
    v += 1.1, 2.2;
      REQUIRE(log.log(v) ==
              string("  \"0\": [1.1, 2.2]\n"));
  }

  SECTION("vector string[]") {
    vector<string> v;
    REQUIRE(log.log(v) ==
            string("  \"0\": []\n"));
  }

  SECTION("vector string [\"a\"]") {
    vector<string> v;
    v += string("a");
    REQUIRE(log.log(v) ==
            string("  \"0\": [\"a\"]\n"));
  }

  SECTION("vector string [\"a\", \"b\"]") {
    vector<string> v;
    v += string("a"), string("b");
    REQUIRE(log.log(v) ==
            string("  \"0\": [\"a\", \"b\"]\n"));
  }

  SECTION("vector char* []") {
    vector<char* > v;
    REQUIRE(log.log(v) ==
            string("  \"0\": []\n"));
  }

  SECTION("vector char*  [\"a\"]") {
    vector<char* > v;
    v += (char*) ("a");
    REQUIRE(log.log(v) ==
            string("  \"0\": [\"a\"]\n"));
  }

  SECTION("vector char*  [\"a\", \"b\"]") {
    vector<char* > v;
    v += (char* )("a"), (char* )("b");
    REQUIRE(log.log(v) ==
            string("  \"0\": [\"a\", \"b\"]\n"));
  }

  // TODO logf, key, repeated key, const versions
  // TODO escape strings
}

TEST_CASE("Map", "[Log]")
{
  Log::Log log("log", true);

  SECTION("open") {
    REQUIRE(log.open((char *)"a") ==
            string("  \"a\":\n"));
  }

  SECTION("close") {
    log.open((char *)"a");
    REQUIRE(log.close() ==
            string(""));
  }

  SECTION("indent") {
    log.open((char *)"a");
    REQUIRE(log.log(1) ==
            string("    \"0\": 1\n"));
    REQUIRE(log.log(1.1) ==
            string("    \"1\": 1.1\n"));
    REQUIRE(log.log((char*)"a") ==
            string("    \"2\": \"a\"\n"));
  }

  SECTION("close outdent") {
    log.open((char *)"a");
    log.close();
    REQUIRE(log.log(1) ==
            string("  \"0\": 1\n"));
    REQUIRE(log.log(1.1) ==
            string("  \"1\": 1.1\n"));
    REQUIRE(log.log((char*)"a") ==
            string("  \"2\": \"a\"\n"));
  }

  SECTION("double close") {
    log.open((char *)"a");
    log.close();
    log.close();
    REQUIRE(log.log(1) ==
            string("  \"0\": 1\n"));
    REQUIRE(log.log(1.1) ==
            string("  \"1\": 1.1\n"));
    REQUIRE(log.log((char*)"a") ==
            string("  \"2\": \"a\"\n"));
  }
}
