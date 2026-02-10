#ifndef DARABONBA_HTTP_REQUEST_H_
#define DARABONBA_HTTP_REQUEST_H_

#include <darabonba/Stream.hpp>
#include <darabonba/http/Header.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/http/URL.hpp>
#include <memory>
#include <string>

namespace Darabonba {
namespace Http {

// Undefine Windows macros that conflict with HTTP method names
#ifdef _WIN32
#ifdef GET
#undef GET
#endif
#ifdef POST
#undef POST
#endif
#ifdef DELETE
#undef DELETE
#endif
#ifdef PUT
#undef PUT
#endif
#ifdef HEAD
#undef HEAD
#endif
#ifdef OPTIONS
#undef OPTIONS
#endif
#ifdef CONNECT
#undef CONNECT
#endif
#ifdef TRACE
#undef TRACE
#endif
#ifdef PATCH
#undef PATCH
#endif
#endif // _WIN32

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

  Request(const Request &other)
      : url_(other.url_), method_(other.method_), header_(other.header_),
        body_(other.body_ ? other.body_ : nullptr) {}

  Request &operator=(Request &&other) = default;
  Request &operator=(const Request &other) = default;

  // Getter methods with 'get' prefix
  const URL &getUrl() const { return url_; }
  URL &getUrl() { return url_; }

  std::string getMethod() const;

  Request &setMethod(Method method) {
    method_ = method;
    return *this;
  };
  Request &setMethod(const std::string &method);

  const Query &getQuery() const { return url_.getQuery(); }
  Query &getQuery() { return url_.getQuery(); }
  Request &setQuery(const Query &query) {
    url_.setQuery(query);
    return *this;
  }
  Request &setQuery(Query &&query) {
    url_.setQuery(std::move(query));
    return *this;
  }

  void addQuery(std::string key, std::string value) {
    url_.getQuery().emplace(key, value);
  }

  std::string getHeader(const std::string key) const {
    auto it = header_.find(key);
    if (it != header_.end())
      return it->second;
    return "";
  }
  const Header &getHeader() const { return header_; }
  Header &getHeader() { return header_; }

  const Header &getHeaders() const { return header_; }
  Header &getHeaders() { return header_; }

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

  void addHeader(std::string key, std::string value) { header_[key] = value; }

  std::shared_ptr<IStream> getBody() const { return body_; }
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
  std::string getProtocol() const { return url_.getScheme(); }
  void setProtocol(const std::string &protocol) { url_.setScheme(protocol); }

  std::string getPathname() const { return url_.getPathName(); }
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
template <> struct adl_serializer<std::shared_ptr<Darabonba::Http::Request>> {
  static void to_json(json &j,
                      const std::shared_ptr<Darabonba::Http::Request> &req) {
    if (!req) {
      j = nullptr;
      return;
    }
    // Serialize actual request data
    j = json{
      {"url", static_cast<std::string>(req->getUrl())},
      {"method", req->getMethod()},
      {"headers", req->getHeaders()},
      {"protocol", req->getProtocol()},
      {"pathname", req->getPathname()}
      // Note: body (IStream) is not serialized as it may be a stream
    };
  }

  static std::shared_ptr<Darabonba::Http::Request> from_json(const json &j) {
    if (j.is_null()) {
      return nullptr;
    }
    auto req = std::make_shared<Darabonba::Http::Request>();
    if (j.contains("url") && !j["url"].is_null()) {
      req->getUrl() = Darabonba::Http::URL(j["url"].get<std::string>());
    }
    if (j.contains("method") && !j["method"].is_null()) {
      req->setMethod(j["method"].get<std::string>());
    }
    if (j.contains("headers") && j["headers"].is_object()) {
      req->setHeaders(j["headers"].get<Darabonba::Http::Header>());
    }
    if (j.contains("protocol") && !j["protocol"].is_null()) {
      req->setProtocol(j["protocol"].get<std::string>());
    }
    if (j.contains("pathname") && !j["pathname"].is_null()) {
      req->setPathname(j["pathname"].get<std::string>());
    }
    return req;
  }
};
} // namespace nlohmann

#endif
