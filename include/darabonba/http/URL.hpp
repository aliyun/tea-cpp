#ifndef DARABONBA_HTTP_URL_H_
#define DARABONBA_HTTP_URL_H_

#include <cstdint>
#include <darabonba/http/Query.hpp>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

namespace Darabonba {
namespace Http {

/**
 * URL class with encoding utilities aligned with tea-python's Url class
 */
class URL {
public:
  explicit URL(const std::string &);
  explicit URL(const char *s) : URL(std::string(s)) {}
  URL() = default;

  /**
   * Parse a URL string and create a URL object (aligned with tea-python)
   * @param urlStr The URL string to parse
   * @return URL object
   */
  static URL parse(const std::string &urlStr) {
    return URL(urlStr);
  }

  /**
   * URL encode a string (aligned with tea-python's url_encode)
   * Encodes each path segment separately, preserving '/' characters
   * @param urlStr The string to encode
   * @return URL encoded string
   */
  static std::string urlEncode(const std::string &urlStr) {
    if (urlStr.empty()) {
      return "";
    }

    std::string result;
    size_t start = 0;
    size_t pos = 0;

    while ((pos = urlStr.find('/', start)) != std::string::npos) {
      if (pos > start) {
        result += percentEncode(urlStr.substr(start, pos - start));
      }
      result += '/';
      start = pos + 1;
    }

    if (start < urlStr.length()) {
      result += percentEncode(urlStr.substr(start));
    }

    return result;
  }

  /**
   * Percent encode a URI string (aligned with tea-python's percent_encode)
   * @param uri The URI string to encode
   * @return Percent encoded string
   */
  static std::string percentEncode(const std::string &uri) {
    if (uri.empty()) {
      return "";
    }

    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex << std::uppercase;

    for (unsigned char c : uri) {
      // Keep alphanumeric and unreserved characters
      if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        encoded << c;
      } else {
        encoded << '%' << std::setw(2) << static_cast<int>(c);
      }
    }

    std::string result = encoded.str();
    // Apply tea-python specific replacements
    // Replace %2B with %20 (space), keep %2A as is, replace %7E with ~
    size_t pos = 0;
    while ((pos = result.find("%7E", pos)) != std::string::npos) {
      result.replace(pos, 3, "~");
    }

    return result;
  }

  /**
   * Path encode a path string (aligned with tea-python's path_encode)
   * Similar to urlEncode but specifically for paths
   * @param path The path string to encode
   * @return Path encoded string
   */
  static std::string pathEncode(const std::string &path) {
    if (path.empty() || path == "/") {
      return path;
    }

    std::string result;
    size_t start = 0;
    size_t pos = 0;

    while ((pos = path.find('/', start)) != std::string::npos) {
      if (pos > start) {
        result += percentEncode(path.substr(start, pos - start));
      }
      result += '/';
      start = pos + 1;
    }

    if (start < path.length()) {
      result += percentEncode(path.substr(start));
    }

    return result;
  }

  explicit operator std::string() const;

  URL(const URL &other) = default;
  URL(URL &&other) = default;
  ~URL() = default;

  URL &operator=(const URL &URL) = default;
  URL &operator=(URL &&other) = default;
  bool operator==(const URL &URL) const;
  bool operator!=(const URL &URL) const { return !(*this == URL); };

  const Query &getQuery() const { return query_; }
  Query &getQuery() { return query_; }
  std::string getQuery(const std::string &name) const {
    const auto it = query_.find(name);
    return (it != query_.end()) ? it->second : "";
  }
  URL &setQuery(const Query &query) {
    query_ = query;
    return *this;
  }
  URL &setQuery(Query &&query) {
    query_ = std::move(query);
    return *this;
  }

  const std::string &getHost() const { return host_; }
  URL &setHost(const std::string &host);

  void clear();
  bool empty() const;
  bool isValid() const;

  uint16_t getPort() const { return port_; }
  URL &setPort(uint16_t port) {
    port_ = port;
    return *this;
  }

  const std::string &getUser() const { return user_; }
  URL &setUser(const std::string &user) {
    user_ = user;
    return *this;
  }
  const std::string &getPassword() const { return password_; }
  URL &setPassword(const std::string &password) {
    password_ = password;
    return *this;
  }
  std::string getUserInfo() const;
  URL &setUserInfo(const std::string &userInfo);

  const std::string &getPathName() const { return pathName_; }
  URL &setPathName(const std::string &path) {
    pathName_ = path;
    return *this;
  }

  const std::string &getScheme() const { return scheme_; }
  URL &setScheme(const std::string &scheme);

  std::string getAuthority() const;
  URL &setAuthority(const std::string &authority);

  const std::string &getFragment() const { return fragment_; }
  bool hasFragment() const { return !fragment_.empty(); }
  URL &setFragment(const std::string &fragment) {
    fragment_ = fragment;
    return *this;
  }

protected:
  enum { INVALID_PORT = 0 };

  std::string scheme_;
  std::string user_;
  std::string password_;
  std::string host_;
  std::string pathName_;
  uint16_t port_ = 0;
  Query query_;
  std::string fragment_;
};
} // namespace Http
} // namespace Darabonba

#endif