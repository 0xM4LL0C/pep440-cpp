#pragma once
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace pep440 {
const std::regex VERSION_REGEX(
    R"(^(?:v)?(?:(\d+)!)?(\d+(?:\.\d+)*)(?:(?:[-_\.]?)(a|b|c|rc|alpha|beta|pre|preview)(?:[-_\.]?)(\d*))?(?:-(\d+)|(?:[-_\.]?(post|rev|r)(?:[-_\.]?)(\d*)))?(?:[-_\.]?(dev)(?:[-_\.]?)(\d*))?(?:\+([a-z0-9]+(?:[-_\.][a-z0-9]+)*))?$)",
    std::regex::icase | std::regex::optimize);

struct VersionParseError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct Version {
  int epoch = 0;
  std::vector<int> release;
  std::optional<std::pair<std::string, int>> pre;
  std::optional<int> post;
  std::optional<int> dev;
  std::optional<std::string> local;

  static Version parse(const std::string& input);
  std::string to_string() const;
  bool strict_eq(const Version& other) const;

  bool operator==(const Version& other) const;
  bool operator!=(const Version& other) const;
  bool operator<(const Version& other) const;
  bool operator>(const Version& other) const {
    return other < *this;
  }
  bool operator<=(const Version& other) const {
    return !(other < *this);
  }
  bool operator>=(const Version& other) const {
    return !(*this < other);
  }
};

enum class Operator {
  Eq,
  NotEq,
  Lt,
  Lte,
  Gt,
  Gte,
  Compatible,
};

struct Range {
  Operator op;
  Version version;

  static Range parse(const std::string& input);
  bool matches(const Version& v) const;
  std::string to_string() const;
};

struct RangeSet {
  std::vector<Range> specs;

  static RangeSet parse(const std::string& input);
  bool matches(const Version& v) const;
  std::string to_string() const;
};

static std::vector<int> normalize_release(std::vector<int> r);
static std::string normalize_pre_label(const std::string& label);
static int pre_release_rank(const std::string& label);
static Operator parse_op(const std::string& o);
}  // namespace pep440
