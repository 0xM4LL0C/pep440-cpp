#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "pep440/version.hpp"

using namespace pep440;

TEST_CASE("Simple version parsing", "[version]") {
  Version v = Version::parse("1.2.3");
  REQUIRE(v.epoch == 0);
  REQUIRE(v.release == std::vector<int>{1, 2, 3});
  REQUIRE_FALSE(v.pre);
  REQUIRE_FALSE(v.post);
  REQUIRE_FALSE(v.dev);
  REQUIRE_FALSE(v.local);
  REQUIRE(v.to_string() == "1.2.3");
}

TEST_CASE("Version with epoch", "[version]") {
  Version v = Version::parse("2!1.0");
  REQUIRE(v.epoch == 2);
  REQUIRE(v.release == std::vector<int>{1, 0});
  REQUIRE(v.to_string() == "2!1.0");
}

TEST_CASE("Version with pre-release", "[version]") {
  Version v = Version::parse("1.0a1");
  REQUIRE(v.pre);
  REQUIRE(v.pre->first == "a");
  REQUIRE(v.pre->second == 1);
  REQUIRE(v.to_string() == "1.0a1");
}

TEST_CASE("Version with post-release", "[version]") {
  Version v = Version::parse("1.0.post5");
  REQUIRE(v.post);
  REQUIRE(v.post.value() == 5);
  REQUIRE(v.to_string() == "1.0.post5");
}

TEST_CASE("Version with dev-release", "[version]") {
  Version v = Version::parse("1.0.dev3");
  REQUIRE(v.dev);
  REQUIRE(v.dev.value() == 3);
  REQUIRE(v.to_string() == "1.0.dev3");
}

TEST_CASE("Version with local segment", "[version]") {
  Version v = Version::parse("1.0+abc.def");
  REQUIRE(v.local);
  REQUIRE(v.local.value() == "abc.def");
  REQUIRE(v.to_string() == "1.0+abc.def");
}

TEST_CASE("Full complex version", "[version]") {
  Version v = Version::parse("1!2.3.4rc5.post6.dev7+build.meta");
  REQUIRE(v.epoch == 1);
  REQUIRE(v.release == std::vector<int>{2, 3, 4});
  REQUIRE(v.pre);
  REQUIRE(v.pre->first == "rc");
  REQUIRE(v.pre->second == 5);
  REQUIRE(v.post == 6);
  REQUIRE(v.dev == 7);
  REQUIRE(v.local.value() == "build.meta");
  REQUIRE(v.to_string() == "1!2.3.4rc5.post6.dev7+build.meta");
}

TEST_CASE("Version comparison", "[version]") {
  REQUIRE(Version::parse("1.0a1") < Version::parse("1.0b1"));
  REQUIRE(Version::parse("1.0b1") < Version::parse("1.0rc1"));
  REQUIRE(Version::parse("1.0rc1") < Version::parse("1.0"));
  REQUIRE(Version::parse("1.0") < Version::parse("1.0.post1"));
  REQUIRE(Version::parse("1.0.dev1") < Version::parse("1.0a1"));
  REQUIRE(Version::parse("0!1.0.0") < Version::parse("1!0.1.0"));
}

TEST_CASE("Parsing pre-release aliases", "[version]") {
  Version v1 = Version::parse("1.0alpha2");
  REQUIRE(v1.pre->first == "a");
  REQUIRE(v1.pre->second == 2);

  Version v2 = Version::parse("1.0beta3");
  REQUIRE(v2.pre->first == "b");
  REQUIRE(v2.pre->second == 3);

  Version v3 = Version::parse("1.0c4");
  REQUIRE(v3.pre->first == "rc");
  REQUIRE(v3.pre->second == 4);
}

TEST_CASE("Parsing multiple release segments", "[version]") {
  Version v = Version::parse("1.2.3.4.5");
  REQUIRE(v.release == std::vector<int>{1, 2, 3, 4, 5});
  REQUIRE(v.to_string() == "1.2.3.4.5");
}

TEST_CASE("Parsing with missing components", "[version]") {
  Version v = Version::parse("1.2");
  REQUIRE(v.release == std::vector<int>{1, 2});
  REQUIRE_FALSE(v.pre);
  REQUIRE_FALSE(v.post);
  REQUIRE_FALSE(v.dev);
  REQUIRE_FALSE(v.local);
}

TEST_CASE("Version normalization", "[version]") {
  Version v = Version::parse("1.0.0.0");
  REQUIRE(v.release == std::vector<int>{1, 0, 0, 0});
  REQUIRE(v.to_string() == "1.0.0.0");
}

TEST_CASE("Version equality", "[version]") {
  Version v1 = Version::parse("1.0.0");
  Version v2 = Version::parse("1.0.0");
  REQUIRE(v1 == v2);
}

TEST_CASE("Invalid version throws", "[version]") {
  REQUIRE_THROWS_AS(Version::parse("not.a.version"), VersionParseError);
  REQUIRE_THROWS_AS(Version::parse("1..0"), VersionParseError);
  REQUIRE_THROWS_AS(Version::parse("1.0-foo"), VersionParseError);
  REQUIRE_THROWS_AS(Version::parse("1.0++abc"), VersionParseError);
  REQUIRE_THROWS_AS(Version::parse("!1.0"), VersionParseError);
}

TEST_CASE("Pre-release aliases normalized", "[version]") {
  REQUIRE(Version::parse("1.0alpha1").to_string() == "1.0a1");
  REQUIRE(Version::parse("1.0beta2").to_string() == "1.0b2");
  REQUIRE(Version::parse("1.0preview3").to_string() == "1.0rc3");
}

TEST_CASE("PEP 440 edge cases", "[version]") {
  Version v1 = Version::parse("1.0.dev");
  REQUIRE(v1.dev == 0);
  REQUIRE(v1.to_string() == "1.0.dev0");

  Version v2 = Version::parse("1.0.post");
  REQUIRE(v2.post == 0);
  REQUIRE(v2.to_string() == "1.0.post0");

  Version v3 = Version::parse("1.0a");
  REQUIRE(v3.pre->first == "a");
  REQUIRE(v3.pre->second == 0);
  REQUIRE(v3.to_string() == "1.0a0");

  REQUIRE(Version::parse("1.0.0") == Version::parse("1.0"));
  REQUIRE(Version::parse("1.0.0.0") == Version::parse("1.0"));

  REQUIRE(Version::parse("1.0+abc") == Version::parse("1.0+xyz"));
  REQUIRE_FALSE(Version::parse("1.0+abc") < Version::parse("1.0+xyz"));
  REQUIRE(Version::parse("1.0+abc").strict_eq(Version::parse("1.0+abc")));
  REQUIRE_FALSE(Version::parse("1.0+abc").strict_eq(Version::parse("1.0+xyz")));
  REQUIRE(Version::parse("1.0+abc") == Version::parse("1.0+xyz"));

  REQUIRE(Version::parse("1.0.dev1") < Version::parse("1.0a1"));
  REQUIRE(Version::parse("1.0a1") < Version::parse("1.0b1"));
  REQUIRE(Version::parse("1.0b1") < Version::parse("1.0rc1"));
  REQUIRE(Version::parse("1.0rc1") < Version::parse("1.0"));
  REQUIRE(Version::parse("1.0") < Version::parse("1.0.post1"));

  REQUIRE(Version::parse("1!1.0") > Version::parse("0!2.0"));
  REQUIRE(Version::parse("1.0a1") < Version::parse("1.0"));
}

TEST_CASE("Range parse and to_string") {
  auto r = Range::parse(">=1.2.3");
  REQUIRE(r.op == Operator::Gte);
  REQUIRE(r.version == Version::parse("1.2.3"));
  REQUIRE(r.to_string() == ">=1.2.3");

  r = Range::parse("!=0.9");
  REQUIRE(r.op == Operator::NotEq);
  REQUIRE(r.version == Version::parse("0.9"));
  REQUIRE(r.to_string() == "!=0.9");

  r = Range::parse("~=2.0");
  REQUIRE(r.op == Operator::Compatible);
  REQUIRE(r.version == Version::parse("2.0"));
  REQUIRE(r.to_string() == "~=2.0");
}

TEST_CASE("Range matches behavior") {
  Version v1 = Version::parse("1.2.3");
  Version v2 = Version::parse("1.2.4");
  Version v3 = Version::parse("2.0.0");

  auto r = Range::parse(">=1.2.3");
  REQUIRE(r.matches(v1));
  REQUIRE(r.matches(v2));
  REQUIRE(!r.matches(Version::parse("1.2.2")));

  r = Range::parse("<2.0");
  REQUIRE(r.matches(v1));
  REQUIRE(r.matches(v2));
  REQUIRE(!r.matches(v3));

  r = Range::parse("!=1.2.3");
  REQUIRE(!r.matches(v1));
  REQUIRE(r.matches(v2));

  r = Range::parse("==1.2.3");
  REQUIRE(r.matches(v1));
  REQUIRE(!r.matches(v2));

  r = Range::parse("~=1.2");
  REQUIRE(r.matches(v1));
  REQUIRE(r.matches(v2));
  REQUIRE(!r.matches(v3));
}

TEST_CASE("RangeSet parse, to_string and matches") {
  auto rs = RangeSet::parse(">=1.0, <2.0, !=1.5");
  REQUIRE(rs.specs.size() == 3);
  REQUIRE(rs.to_string() == ">=1.0,<2.0,!=1.5");

  Version v1 = Version::parse("1.4.0");
  Version v2 = Version::parse("1.5.0");
  Version v3 = Version::parse("2.0.0");

  REQUIRE(rs.matches(v1));
  REQUIRE(!rs.matches(v2));
  REQUIRE(!rs.matches(v3));
}

TEST_CASE("RangeSet empty matches all") {
  RangeSet rs;
  REQUIRE(rs.matches(Version::parse("0.0.1")));
  REQUIRE(rs.matches(Version::parse("999.999.999")));
}
