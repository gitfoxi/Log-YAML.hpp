
// TODO logf, const versions, stderr, generic containers
// float INF, NAN

#include "../Log-YAML.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// #include <cstdint>
#include <stdint.h>
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

TEST_CASE("set", "[Log]")
{
  Log::Log log("log", true);

  SECTION("set int [1]") {
    set<int> v;
    v.insert(1);
    REQUIRE(log.log(v) ==
            string("  \"0\": [1]\n"));
  }
}

TEST_CASE("vector", "[Log]")
{
  Log::Log log("log", true);

  SECTION("vector int []") {
    vector<int> v;
    REQUIRE(log.log("s", v) ==
            string("  \"s\": []\n"));
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

}

TEST_CASE("Escape", "[Log]")
{
  Log::Log log("log", true);

  SECTION("escape char*") {
    REQUIRE(log.log((char *)"\"\n", (char *)"\"\n") ==
            string("  \"\\\"\\n\": \"\\\"\\n\"\n"));
  }

  SECTION("escape string") {
    REQUIRE(log.log(string("\"\n"), string("\"\n")) ==
            string("  \"\\\"\\n\": \"\\\"\\n\"\n"));
  }
}

TEST_CASE("Key", "[Log]")
{
  Log::Log log("log", true);

  SECTION("repeat key") {
    log.log("a", 1);
    REQUIRE(log.log("a", 1) ==
            "  \"a'\": 1\n");
  }

  SECTION("anon key") {
    REQUIRE(log.log(1) ==
            "  \"0\": 1\n");
    REQUIRE(log.log(1) ==
            "  \"1\": 1\n");
  }

  SECTION("repeat key after sub") {
    log.log("a", 1);
    log.open("sub");
    REQUIRE(log.log("a", 1) ==
            "    \"a\": 1\n");
    log.close();
    REQUIRE(log.log("a", 1) ==
            "  \"a'\": 1\n");
  }

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

TEST_CASE("const", "[Log]") {
  Log::Log log("log", true);

  SECTION("const int") {
    REQUIRE(log.log((const int)1) ==
            string("  \"0\": 1\n"));
  }

  SECTION("const unsigned") {
    REQUIRE(log.log((const unsigned)1) ==
            string("  \"0\": 1\n"));
  }

#if 0
  SECTION("vector<const string>") {
    vector<const string> vc;
    const string a("a");
    vc += a;
    REQUIRE(log.log(vc) ==
            string("  \"0\": [\"a\"]\n"));
  }

  SECTION("const vector<const string>") {
    vector<const string> vc;
    const string a("a");
    vc += a;
    const vector<const string> cvc(vc);
    REQUIRE(log.log(cvc) ==
            string("  \"0\": [\"a\"]\n"));
  }

  SECTION("const vector<long long>") {
    vector<uint64_t> vc;
    const uint64_t i = 1;
    vc.push_back(i);
    const vector<uint64_t> cvc(vc);
    REQUIRE(log.log(cvc) ==
            string("  \"0\": [1]\n"));
  }
#endif
}
