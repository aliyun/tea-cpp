#ifndef DARABONBA_TYPE_H_
#define DARABONBA_TYPE_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace Darabonba {

using Json = nlohmann::json;
using Bytes = nlohmann::json::binary_t;
// using Bytes = std::vector<uint8_t>;

class Exception : public std::exception {
public:
  Exception() = default;
  Exception(const std::string &msg) : msg_(msg) {}
  Exception(std::string &&msg) : msg_(std::move(msg)) {}
  virtual ~Exception() = default;

  virtual const char *what() const noexcept override { return msg_.c_str(); }

protected:
  std::string msg_;
};

class ValidateException : public Exception {
public:
  ValidateException() = default;
  ValidateException(const std::string &code, const std::string msg)
      : Exception(code + ": " + msg) {}

  virtual ~ValidateException() = default;

protected:
};

} // namespace Darabonba

#endif