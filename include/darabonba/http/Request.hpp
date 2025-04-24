#ifndef DARABONBA_HTTP_REQUEST_H_
#define DARABONBA_HTTP_REQUEST_H_

#include <darabonba/Stream.hpp>
#include <darabonba/http/Header.hpp>
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

  std::string header(const std::string key) const {
    auto it = header_.find(key);
    if (it != header_.end())
      return it->second;
    return "";
  }
  const Header &header() const { return header_; };
  Header &header() { return header_; }
  Request &setHeader(const Header &header) {
    header_ = header;
    return *this;
  }
  Request &setHeader(Header &&header) {
    header_ = std::move(header);
    return *this;
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

protected:
  URL url_;
  Method method_ = Method::GET;
  Header header_;
  std::shared_ptr<IStream> body_;
};

} // namespace Http
} // namespace Darabonba

#endif