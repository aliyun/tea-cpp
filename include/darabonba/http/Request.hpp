#ifndef DARABONBA_HTTP_REQUEST_H_
#define DARABONBA_HTTP_REQUEST_H_

#include <darabonba/Stream.hpp>
#include <darabonba/http/Header.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/http/URL.hpp>
#include <iostream>
#include <memory>
#include <string>

namespace Darabonba {
namespace Http {

class Request {
public:
  enum class Method {
    GET,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    CONNECT,
    OPTIONS,
    PATCH,
  };

  Request() = default;
  Request(const std::string &url) : url_(url) {}
  Request(const URL &url) : url_(url) {}
  ~Request() = default;

  Request(const Request& other) :
      url_(other.url_),
      method_(other.method_),
      header_(other.header_),
      body_(other.body_ ? other.body_ : nullptr) {}

  Request &operator=(Request &&) = default;
  Request &operator=(const Request &) = default;

  const URL &url() const { return url_; }
  URL &url() { return url_; }

  std::string method() const;

  Request &setMethod(Method method) {
    method_ = method;
    return *this;
  };
  Request &setMethod(const std::string &method);

  const Query &query() const { return url_.query(); }
  Query &query() { return url_.query(); }
  Request &setQuery(const Query &query) {
    url_.setQuery(query);
    return *this;
  }
  Request &setQuery(Query &&query) {
    url_.setQuery(std::move(query));
    return *this;
  }

  void addQuery(std::string key, std::string value) {
    url_.query().emplace(key, value);
  }

  std::string header(const std::string key) const {
    auto it = header_.find(key);
    if (it != header_.end())
      return it->second;
    return "";
  }
  const Header &header() const { return header_; }
  Header &header() { return header_; }

  const Header &headers() const { return header_; }
  Header &headers() { return header_; }

  Request &setHeader(const Header &header) {
    header_ = header;
    return *this;
  }
  Request &setHeader(Header &&header) {
    header_ = std::move(header);
    return *this;
  }

  Request &setHeaders(Header &&header) {
    header_ = std::move(header);
    return *this;
  }

  Request &setHeaders(const Header &header) {
    header_ = header;
    return *this;
  }

  void addHeader(std::string key, std::string value) {
    header_.emplace(key, value);
  }

  std::shared_ptr<IStream> body() const { return body_; }
  Request &setBody(std::shared_ptr<IStream> body) {
    body_ = body;
    return *this;
  }
  template <typename T> Request &setBody(T &&obj) {
    auto p = new ISStream(std::forward<T>(obj));
    body_ = std::shared_ptr<IStream>(p);
    return *this;
  }

  // New methods to access protocol and path
  std::string getProtocol() const { return url_.scheme(); }
  void setProtocol(const std::string &protocol) { url_.setScheme(protocol); }

  std::string getPathname() const { return url_.pathName(); }
  void setPathname(const std::string &path) { url_.setPathName(path); }

protected:
  URL url_;
  Method method_ = Method::GET;
  Header header_;
  std::shared_ptr<IStream> body_;
};

} // namespace Http
} // namespace Darabonba

namespace nlohmann {
  template <>
  struct adl_serializer<std::shared_ptr<Darabonba::Http::Request>> {
    static void to_json(json &j, const std::shared_ptr<Darabonba::Http::Request> &body) {
      j = {{"req_address", reinterpret_cast<uintptr_t>(body.get())}};
    }

    static std::shared_ptr<Darabonba::Http::Request> from_json(const json &j) {
      if (j.contains("req_address")) {
        Darabonba::Http::Request *ptr = reinterpret_cast<Darabonba::Http::Request *>(j.at("req_address").get<uintptr_t>());
        return std::shared_ptr<Darabonba::Http::Request>(ptr);
      }
      return nullptr;
    }
  };
}

#endif
