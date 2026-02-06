#ifndef DARABONBA_STRING_H_
#define DARABONBA_STRING_H_

#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace Darabonba {
class String {
public:
  static std::vector<std::string>
  split(const std::string &raw, const std::string &delim, int limit = -1) {
    if (delim.empty()) {
      return {raw};
    }
    std::vector<std::string> ret;
    std::string::size_type pos = 0;
    int i = 0;
    for (; limit < 0 || i < limit; ++i) {
      auto nextPos = raw.find(delim, pos);
      if (nextPos == std::string::npos) {
        break;
      }
      ret.emplace_back(raw.substr(pos, nextPos - pos));
      pos = nextPos + delim.size();
    }
    if (limit < 0 || i < limit) {
      ret.emplace_back(raw.substr(pos, raw.size() - pos));
    } else {
      ret.back() += delim + raw.substr(pos, raw.size() - pos);
    }
    return ret;
  }

  static std::string replace(const std::string &raw, const std::string &oldStr,
                             const std::string &newStr, int count = -1) {
    if (oldStr.empty() || raw.empty()) {
      return raw;
    }
    std::string::size_type pos = 0;
    std::string ret;
    for (int i = 0; count < 0 || i < count; ++i) {
      auto nextPos = raw.find(oldStr, pos);
      if (nextPos == std::string::npos) {
        break;
      }
      ret.append(raw, pos, nextPos - pos);
      ret.append(newStr);
      pos = nextPos + oldStr.size();
    }
    ret.append(raw, pos, raw.size() - pos);
    return ret;
  }

  static bool contains(const std::string &s, const std::string &substr) {
    return s.find(substr) != std::string::npos;
  }

  static int count(const std::string &s, const std::string &substr) {
    if (substr.empty())
      return 0;
    int count = 0;
    std::string::size_type pos = s.find(substr);
    while (pos != std::string::npos) {
      count++;
      pos = s.find(substr, pos + substr.length());
    }
    return count;
  }

  static bool hasPrefix(const std::string &s, const std::string &prefix) {
    auto targetLength = prefix.size();
    if (s.size() < targetLength) {
      return false;
    }
    return memcmp(s.c_str(), prefix.c_str(), targetLength) == 0;
  }

  static bool hasSuffix(const std::string &s, const std::string &substr) {
    auto targetLength = substr.size();
    if (s.size() < targetLength) {
      return false;
    }
    return memcmp(s.c_str() + s.size() - targetLength, substr.c_str(),
                  targetLength) == 0;
  }

  static int index(const std::string &s, const std::string &substr) {
    auto ret = s.find(substr);
    return ret == std::string::npos ? -1 : static_cast<int>(ret);
  }

  static std::string toLower(const std::string &s) {
    std::string ret(s.size(), '\0');
    std::transform(s.begin(), s.end(), ret.begin(),
                   [](char c) { return std::tolower(c); });
    return ret;
  }

  static std::string toUpper(const std::string &s) {
    std::string ret(s.size(), '\0');
    std::transform(s.begin(), s.end(), ret.begin(),
                   [](char c) { return std::toupper(c); });
    return ret;
  }

  static std::string subString(const std::string &s,
                               std::string::size_type start,
                               std::string::size_type end = std::string::npos) {
    return end == std::string::npos ? s.substr(start)
                                    : s.substr(start, end - start);
  }

  static bool equals(const std::string &expect, const std::string &actual) {
    return expect == actual;
  }

  static std::string trim(const std::string &s) {
    auto begin = s.begin(), end = s.end();
    for (; begin != end && std::isspace(*begin); ++begin)
      ;
    for (; begin != end && std::isspace(*std::prev(end)); --end)
      ;
    return std::string(begin, end);
  }
};
} // namespace Darabonba

#endif
