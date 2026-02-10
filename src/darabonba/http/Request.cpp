#include <algorithm>
#include <cctype>
#include <darabonba/http/Request.hpp>

namespace Darabonba {
namespace Http {
std::string Request::getMethod() const {
  switch (method_) {
  case Method::GET:
    return "GET";
  case Method::POST:
    return "POST";
  case Method::HEAD:
    return "HEAD";
  case Method::PUT:
    return "PUT";
  case Method::DELETE:
    return "DELETE";
  case Method::TRACE:
    return "TRACE";
  case Method::CONNECT:
    return "CONNECT";
  case Method::OPTIONS:
    return "OPTIONS";
  case Method::PATCH:
    return "PATCH";
  }
  return "";
}

Request &Request::setMethod(const std::string &method) {
  std::string upper = method;
  std::transform(upper.begin(), upper.end(), upper.begin(),
                 [](unsigned char c) -> char { return static_cast<char>(std::toupper(c)); });
  if (upper == "GET") {
    method_ = Method::GET;
  } else if (upper == "POST") {
    method_ = Method::POST;
  } else if (upper == "HEAD") {
    method_ = Method::HEAD;
  } else if (upper == "PUT") {
    method_ = Method::PUT;
  } else if (upper == "DELETE") {
    method_ = Method::DELETE;
  } else if (upper == "TRACE") {
    method_ = Method::TRACE;
  } else if (upper == "CONNECT") {
    method_ = Method::CONNECT;
  } else if (upper == "OPTIONS") {
    method_ = Method::OPTIONS;
  } else if (upper == "PATCH") {
    method_ = Method::PATCH;
  }
  // TODO error method
  return *this;
}

} // namespace Http
} // namespace Darabonba