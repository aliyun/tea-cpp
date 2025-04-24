
#include <cctype>
#include <curl/curl.h>
#include <darabonba/http/Query.hpp>
#include <iomanip>
#include <sstream>
#include <unordered_set>

inline static int hexVal(char c) {
  if ('0' <= c && c <= '9')
    return c - '0';
  else if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  else if ('A' <= c && c <= 'F')
    return c - 'A' + 10;
  else
    return -1;
}
inline static bool isHexChar(char c) {
  return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
         ('A' <= c && c <= 'F');
}

namespace Darabonba {
namespace Http {

Query::Query(const std::string &content) {
  std::string::size_type startPos = 0;
  while (startPos < content.length()) {
    std::size_t endPos = content.find('&', startPos);
    if (endPos == std::string::npos)
      endPos = content.length();
    std::size_t equalPos = content.find('=', startPos);
    if (equalPos == std::string::npos) {
      this->emplace(content.substr(startPos), "");
      return;
    } else if (equalPos > endPos) {
      this->emplace(content.substr(startPos, endPos - startPos), "");
      startPos = endPos + 1;
      continue;
    }
    this->emplace(content.substr(startPos, equalPos - startPos),
                  content.substr(equalPos + 1, endPos - equalPos - 1));
    startPos = endPos + 1;
  }
}

Query::Query(const char *s) : Query(std::string(s)) {}

std::string Query::encode(const std::string &content, const std::string &safe) {
  static const char table[] = "0123456789ABCDEF";
  std::unordered_set<char> s(safe.begin(), safe.end());
  std::string ret;
  for (auto c : content) {
    if (isalnum(c) || s.count(c))
      ret.push_back(c);
    else {
      ret.push_back('%');
      ret.push_back(table[(c >> 4) & 0xf]);
      ret.push_back(table[c & 0xf]);
    }
  }
  return ret;
}

std::string Query::decode(const std::string &content) {
  std::string ret;
  for (size_t i = 0; i < content.size();) {
    auto c = content[i];
    if (c == '%' && isHexChar(content[i + 1]) && isHexChar(content[i + 2])) {
      ret.push_back((hexVal(content[i + 1]) << 4) | (hexVal(content[i + 2])));
      i += 3;
    } else {
      ret.push_back(c);
      ++i;
    }
  }
  return ret;
}

Query::operator std::string() const {
  std::string ret;
  for (const auto &p : *this) {
    ret += encode(p.first) + '=' + encode(p.second) + '&';
  }
  ret.pop_back();
  return ret;
}

} // namespace Http
} // namespace Darabonba
