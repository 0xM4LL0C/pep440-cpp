#include "pep440/version.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace pep440 {

static std::vector<int> normalize_release(std::vector<int> r) {
  while (!r.empty() && r.back() == 0) {
    r.pop_back();
  }
  return r;
}

static std::string normalize_pre_label(const std::string& label) {
  std::string l = label;
  std::transform(l.begin(), l.end(), l.begin(), ::tolower);
  if (l == "alpha") {
    return "a";
  } else if (l == "beta") {
    return "b";
  } else if (l == "c" || l == "rc" || l == "pre" || l == "preview") {
    return "rc";
  }
  return l;
}

Version Version::parse(const std::string& input) {
  Version v;

  std::smatch match;
  if (!std::regex_match(input, match, VERSION_REGEX)) {
    throw VersionParseError("invalid version: " + input);
  }

  if (match[1].matched) {
    v.epoch = std::stoi(match[1].str());
  }

  std::stringstream ss(match[2].str());
  std::string segment;
  while (std::getline(ss, segment, '.')) {
    v.release.push_back(std::stoi(segment));
  }

  if (match[3].matched) {
    std::string pre_l = normalize_pre_label(match[3].str());
    int pre_n = match[4].matched && !match[4].str().empty()
                    ? std::stoi(match[4].str())
                    : 0;
    v.pre = std::make_pair(pre_l, pre_n);
  }

  if (match[5].matched) {
    v.post = std::stoi(match[5].str());
  } else if (match[6].matched || match[7].matched) {
    int post_n = match[7].matched && !match[7].str().empty()
                     ? std::stoi(match[7].str())
                     : 0;
    v.post = post_n;
  }

  if (match[8].matched) {
    int dev_n = match[9].matched && !match[9].str().empty()
                    ? std::stoi(match[9].str())
                    : 0;
    v.dev = dev_n;
  }

  if (match[10].matched) {
    v.local = match[10].str();
  }

  return v;
}

std::string Version::to_string() const {
  std::stringstream ss;
  if (epoch != 0) {
    ss << epoch << "!";
  }

  for (size_t i = 0; i < release.size(); ++i) {
    if (i > 0) {
      ss << ".";
    }
    ss << release[i];
  }

  if (pre.has_value()) {
    ss << pre->first << pre->second;
  }

  if (post.has_value()) {
    ss << ".post" << post.value();
  }

  if (dev.has_value()) {
    ss << ".dev" << dev.value();
  }

  if (local.has_value()) {
    ss << "+" << local.value();
  }

  return ss.str();
}

bool Version::operator==(const Version& other) const {
  return epoch == other.epoch &&
         normalize_release(release) == normalize_release(other.release) &&
         pre == other.pre && post == other.post && dev == other.dev;
}

bool Version::operator!=(const Version& other) const {
  return !(*this == other);
}

bool Version::strict_eq(const Version& other) const {
  return epoch == other.epoch &&
         normalize_release(release) == normalize_release(other.release) &&
         pre == other.pre && post == other.post && dev == other.dev &&
         local == other.local;
}

static int pre_release_rank(const std::string& label) {
  if (label == "a" || label == "alpha") {
    return 0;
  } else if (label == "b" || label == "beta") {
    return 1;
  } else if (label == "rc" || label == "c" || label == "pre" ||
             label == "preview") {
    return 2;
  }
  return 3;  // unknown label
}

bool Version::operator<(const Version& other) const {
  if (epoch != other.epoch) {
    return epoch < other.epoch;
  }
  if (release != other.release) {
    return release < other.release;
  }

  if (dev != other.dev) {
    if (!dev.has_value()) {
      return false;
    } else if (!other.dev.has_value()) {
      return true;
    }
    return dev.value() < other.dev.value();
  }

  if (pre != other.pre) {
    if (!pre.has_value()) {
      return false;
    } else if (!other.pre.has_value()) {
      return true;
    }

    int rank1 = pre_release_rank(pre->first);
    int rank2 = pre_release_rank(other.pre->first);
    if (rank1 != rank2) {
      return rank1 < rank2;
    }

    return pre->second < other.pre->second;
  }

  if (post != other.post) {
    if (!post.has_value()) {
      return true;
    } else if (!other.post.has_value()) {
      return false;
    }
    return post.value() < other.post.value();
  }

  return false;
}

static Operator parse_op(const std::string& o) {
  if (o == "==") {
    return Operator::Eq;
  } else if (o == "!=") {
    return Operator::NotEq;
  } else if (o == "<=") {
    return Operator::Lte;
  } else if (o == ">=") {
    return Operator::Gte;
  } else if (o == "<") {
    return Operator::Lt;
  } else if (o == ">") {
    return Operator::Gt;
  } else if (o == "~=") {
    return Operator::Compatible;
  }
  throw std::invalid_argument("Invelid operator: " + o);
}

Range Range::parse(const std::string& s) {
  auto pos = s.find_first_not_of(" \t");
  auto trimmed = s.substr(pos);
  std::string op, ver;
  if (trimmed.rfind("~=", 0) == 0) {
    op = "~=";
    ver = trimmed.substr(2);
  } else if (trimmed.rfind(">=", 0) == 0) {
    op = ">=";
    ver = trimmed.substr(2);
  } else if (trimmed.rfind("<=", 0) == 0) {
    op = "<=";
    ver = trimmed.substr(2);
  } else if (trimmed.rfind("!=", 0) == 0) {
    op = "!=";
    ver = trimmed.substr(2);
  } else if (trimmed.rfind("==", 0) == 0) {
    op = "==";
    ver = trimmed.substr(2);
  } else if (trimmed.rfind(">", 0) == 0) {
    op = ">";
    ver = trimmed.substr(1);
  } else if (trimmed.rfind("<", 0) == 0) {
    op = "<";
    ver = trimmed.substr(1);
  } else {
    throw VersionParseError("bad range: " + s);
  }
  Range r;
  r.op = parse_op(op);
  r.version = Version::parse(ver);
  return r;
}

bool Range::matches(const Version& v) const {
  switch (op) {
    case Operator::Eq:
      return v == version;
    case Operator::NotEq:
      return v != version;
    case Operator::Lt:
      return v < version;
    case Operator::Lte:
      return v <= version;
    case Operator::Gt:
      return v > version;
    case Operator::Gte:
      return v >= version;
    case Operator::Compatible:
      auto& rel = version.release;
      auto pref = Version::parse(version.to_string());
      Version low = version;
      Version high = version;
      if (rel.size() > 1)
        high.release.back()++;
      else
        high.release[0]++;
      return v >= low && v < high;
  }
  return false;
}

std::string Range::to_string() const {
  std::stringstream ss;

  switch (this->op) {
    case Operator::Eq:
      ss << "==";
      break;
    case Operator::NotEq:
      ss << "!=";
      break;
    case Operator::Lt:
      ss << "<";
      break;
    case Operator::Lte:
      ss << "<=";
      break;
    case Operator::Gt:
      ss << ">";
      break;
    case Operator::Gte:
      ss << ">=";
      break;
    case Operator::Compatible:
      ss << "~=";
      break;
  }

  ss << this->version.to_string();
  return ss.str();
}

RangeSet RangeSet::parse(const std::string& s) {
  RangeSet rs;
  std::stringstream ss(s);
  std::string part;
  while (std::getline(ss, part, ',')) {
    if (part.find_first_not_of(" \t") != std::string::npos) {
      rs.specs.push_back(Range::parse(part));
    }
  }
  return rs;
}

bool RangeSet::matches(const Version& v) const {
  for (auto& r : specs) {
    if (!r.matches(v)) {
      return false;
    }
  }
  return true;
}

std::string RangeSet::to_string() const {
  std::stringstream ss;

  for (size_t i = 0; i < this->specs.size(); i++) {
    ss << this->specs[i].to_string();

    if (i != (this->specs.size() - 1)) {
      ss << ",";
    }
  }

  return ss.str();
}

}  // namespace pep440
