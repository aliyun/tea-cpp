#include <darabonba/core.hpp>

Darabonba::Error::Error() = default;

Darabonba::Error::Error(const std::string &msg) {
    message = std::string(msg.begin(), msg.end());
}

Darabonba::Error::Error(const char* msg) {
  message = std::string(msg);
}

Darabonba::Error::Error(const json &errorInfo) {
  if (errorInfo.find("message") != errorInfo.end()) {
    message = errorInfo["message"].get<std::string>();
  }
  if (errorInfo.find("code") != errorInfo.end()) {
    code = errorInfo["code"].get<std::string>();
  }
  if (errorInfo.find("name") != errorInfo.end()) {
    name = errorInfo["name"].get<std::string>();
  }
  if (errorInfo.find("description") != errorInfo.end()) {
    description = errorInfo["description"].get<std::string>();
  }
  if (errorInfo.find("statusCode") != errorInfo.end()) {
    description = errorInfo["statusCode"].get<std::string>();
  }
  if (errorInfo.find("data") != errorInfo.end()) {
    data = errorInfo["data"];
  }
}

Darabonba::Error Darabonba::UnretryableError::getLastException() {
  return _last_exception;
}

Darabonba::Request Darabonba::UnretryableError::getLastRequest() {
  return _last_request;
}
