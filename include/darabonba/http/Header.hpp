#ifndef DARABONBA_HTTP_HEADER_H_
#define DARABONBA_HTTP_HEADER_H_
#include <darabonba/Type.hpp>
#include <initializer_list>
#include <map>
#include <string>

namespace Darabonba {
namespace Http {
class Header : public std::map<std::string, std::string> {
  friend void to_json(Darabonba::Json &j, const Header &obj) {
    j = std::map<std::string, std::string>(obj);
  }
  friend void from_json(const Darabonba::Json &j, Header &obj) {
    obj = j.get<std::map<std::string, std::string>>();
  }

public:
  static const std::string ACCEPT_ENCODING;
  static const std::string AUTHORIZATION;
  static const std::string CONNECTION;
  static const std::string CONTENT_LENGTH;
  static const std::string COOKIE;
  static const std::string DATE;
  static const std::string HOST;
  static const std::string LOCATION;
  static const std::string REFERER;
  static const std::string TRANSFER_ENCODING;
  static const std::string USER_AGENT;
  Header() = default;
  Header(const Header &) = default;
  Header(Header &&) = default;
  Header(const std::map<std::string, std::string> &obj)
      : std::map<std::string, std::string>(obj) {}
  Header(std::map<std::string, std::string> &&obj)
      : std::map<std::string, std::string>(std::move(obj)) {}
  Header(std::initializer_list<std::pair<std::string, std::string>> list)
      : std::map<std::string, std::string>(list.begin(), list.end()) {}
  Header(const Darabonba::Json &obj) { from_json(obj, *this); }

  Header &operator=(const Header &) = default;
  Header &operator=(Header &&) = default;
  using std::map<std::string, std::string>::operator=;

  operator std::string() const {
    std::string ret;
    for (const auto &p : *this) {
      std::string firstLowerCase = p.first;
      std::transform(firstLowerCase.begin(), firstLowerCase.end(), firstLowerCase.begin(), ::tolower);

      std::string modifiedSecond = p.second;

      if (firstLowerCase == "content-type") {
        const std::string boundaryPrefix = "multipart/form-data; boundary=";
        if (p.second.compare(0, boundaryPrefix.size(), boundaryPrefix) == 0) {
          modifiedSecond = "multipart/form-data";  // 修改 second
        }
      }
      ret += p.first + ": " + modifiedSecond + "\r\n";
    }
    return ret;
  }
};

} // namespace Http
} // namespace Darabonba

#endif