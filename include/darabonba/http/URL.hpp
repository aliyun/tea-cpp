#ifndef DARABONBA_HTTP_URL_H_
#define DARABONBA_HTTP_URL_H_

#include <cstdint>
#include <darabonba/http/Query.hpp>
#include <map>
#include <string>

namespace Darabonba {
namespace Http {
class URL {
public:
  explicit URL(const std::string &);
  explicit URL(const char *s) : URL(std::string(s)) {}
  URL() = default;

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