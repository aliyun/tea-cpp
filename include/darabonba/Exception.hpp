#ifndef DARABONBA_EXCEPTIONS_HPP
#define DARABONBA_EXCEPTIONS_HPP

#include <darabonba/Model.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/Request.hpp>
#include <exception>
#include <memory>
#include <string>

namespace Darabonba {

// Forward declarations
namespace Policy {
class RetryPolicyContext;
}

// DaraException Class - Base exception class aligned with tea-python
class DaraException : public std::exception {
public:
  DaraException() = default;

  // Constructor from const char* (to avoid ambiguity)
  DaraException(const char *msg) : message_(msg) {}

  DaraException(const std::string &msg) : message_(msg) {}

  DaraException(std::string &&msg) : message_(std::move(msg)) {}

  // Dictionary-style initialization (aligned with tea-python)
  explicit DaraException(const Darabonba::Json &dic) {
    name_ = "DaraException";
    if (dic.contains("code") && !dic["code"].is_null()) {
      code_ = dic["code"].get<std::string>();
    }
    if (dic.contains("message") && !dic["message"].is_null()) {
      message_ = dic["message"].get<std::string>();
    }
    if (dic.contains("data") && !dic["data"].is_null()) {
      data_ = dic["data"];
    }
    if (dic.contains("description") && !dic["description"].is_null()) {
      description_ = dic["description"].get<std::string>();
    }
    if (dic.contains("accessDeniedDetail") && !dic["accessDeniedDetail"].is_null()) {
      accessDeniedDetail_ = dic["accessDeniedDetail"];
    }
    // Extract statusCode from data if present
    if (data_.contains("statusCode") && !data_["statusCode"].is_null()) {
      statusCode_ = data_["statusCode"].get<int64_t>();
    }
  }

  DaraException(const DaraException &ex) = default;

  DaraException(DaraException &&) = default;

  virtual ~DaraException() = default;

  DaraException &operator=(const DaraException &) = default;

  DaraException &operator=(DaraException &&) = default;

  friend void to_json(nlohmann::json &j, const DaraException &ex) {
    j = nlohmann::json{
      {"code", ex.code_},
      {"message", ex.message_},
      {"data", ex.data_},
      {"description", ex.description_},
      {"accessDeniedDetail", ex.accessDeniedDetail_}
    };
  }

  friend void from_json(const nlohmann::json &j, DaraException &ex) {
    if (j.contains("code") && !j["code"].is_null()) {
      ex.code_ = j["code"].get<std::string>();
    }
    if (j.contains("message") && !j["message"].is_null()) {
      ex.message_ = j["message"].get<std::string>();
    }
    if (j.contains("data") && !j["data"].is_null()) {
      ex.data_ = j["data"];
    }
    if (j.contains("description") && !j["description"].is_null()) {
      ex.description_ = j["description"].get<std::string>();
    }
    if (j.contains("accessDeniedDetail") && !j["accessDeniedDetail"].is_null()) {
      ex.accessDeniedDetail_ = j["accessDeniedDetail"];
    }
  }

  virtual const char *what() const noexcept override { return message_.c_str(); }

  // Getters for each field (aligned with tea-python)
  const std::string &getMessage() const { return message_; }
  const std::string &getName() const { return name_; }
  const std::string &getCode() const { return code_; }
  const Darabonba::Json &getData() const { return data_; }
  const std::string &getDescription() const { return description_; }
  const Darabonba::Json &getAccessDeniedDetail() const { return accessDeniedDetail_; }
  int64_t getStatusCode() const { return statusCode_; }
  int32_t getRetryAfter() const { return retryAfter_; }

  // Setters
  void setCode(const std::string &code) { code_ = code; }
  void setMessage(const std::string &message) { message_ = message; }
  void setData(const Darabonba::Json &data) { data_ = data; }
  void setDescription(const std::string &description) { description_ = description; }
  void setAccessDeniedDetail(const Darabonba::Json &detail) { accessDeniedDetail_ = detail; }
  void setStatusCode(int64_t statusCode) { statusCode_ = statusCode; }
  void setRetryAfter(int32_t retryAfter) { retryAfter_ = retryAfter; }

  // String representation (aligned with tea-python)
  virtual std::string toString() const {
    return "Error: " + code_ + " " + message_ + " Response: " + data_.dump();
  }

protected:
  std::string code_;
  std::string message_;
  Darabonba::Json data_;
  std::string description_;
  Darabonba::Json accessDeniedDetail_;
  int64_t statusCode_ = 0;
  int32_t retryAfter_ = 0;
  std::string name_ = "DaraException";
};


class ValidateException : public DaraException {
public:
  ValidateException() { name_ = "ValidateException"; }

  ValidateException(const std::string &code, const std::string &msg)
      : DaraException(msg) {
    name_ = "ValidateException";
    code_ = code;
    message_ = code + ": " + msg;
  }

  ValidateException(const ValidateException &) = default;
  ValidateException(ValidateException &&) = default;
  virtual ~ValidateException() = default;
  ValidateException &operator=(const ValidateException &) = default;
  ValidateException &operator=(ValidateException &&) = default;
};

// ResponseException Class (aligned with tea-python)
class ResponseException : public DaraException {
public:
  ResponseException();
  ResponseException(const ResponseException &) = default;
  ResponseException(ResponseException &&) = default;
  
  // Constructor with individual parameters (aligned with tea-python)
  ResponseException(
      const std::string &code,
      const std::string &message,
      int64_t statusCode = 0,
      int32_t retryAfter = 0,
      const Darabonba::Json &data = nullptr,
      const Darabonba::Json &accessDeniedDetail = nullptr,
      const std::string &description = "",
      const std::string &stack = "")
  {
    name_ = "ResponseException";
    code_ = code;
    message_ = message;
    statusCode_ = statusCode;
    retryAfter_ = retryAfter;
    data_ = data;
    if (!data_.is_null() && statusCode != 0) {
      data_["statusCode"] = statusCode;
    }
    accessDeniedDetail_ = accessDeniedDetail;
    description_ = description;
    stack_ = stack;
  }

  // Dictionary-style initialization (aligned with tea-python)
  ResponseException(const Darabonba::Json &obj) : DaraException(obj) {
    name_ = "ResponseException";
    if (obj.contains("statusCode") && !obj["statusCode"].is_null()) {
      statusCode_ = obj["statusCode"].get<int64_t>();
    }
    if (obj.contains("retryAfter") && !obj["retryAfter"].is_null()) {
      retryAfter_ = obj["retryAfter"].get<int32_t>();
    }
    if (obj.contains("stack") && !obj["stack"].is_null()) {
      stack_ = obj["stack"].get<std::string>();
    }
  }

  virtual ~ResponseException() = default;
  ResponseException &operator=(const ResponseException &) = default;
  ResponseException &operator=(ResponseException &&) = default;

  // Override what() to return SDKError format
  const char *what() const noexcept override {
    if (msg_.empty()) {
      msg_ = "SDKError: code=" + code_ + ", message=" + message_ + ", statusCode=" + std::to_string(statusCode_);
    }
    return msg_.c_str();
  }

  // Getters (use base class getters where applicable)
  const std::string &getStack() const { return stack_; }
  std::string getStatusMessage() const { return statusMessage_; }

  // Setters
  void setStack(const std::string &stack) { stack_ = stack; }
  void setStatusMessage(const std::string &statusMessage) { statusMessage_ = statusMessage; }

  // String representation (aligned with tea-python)
  std::string toString() const override {
    return "Error: " + code_ + " " + message_ + " Response: " + data_.dump();
  }

  // JSON serialization
  friend void to_json(Darabonba::Json &j, const ResponseException &obj) {
    j = Darabonba::Json{
      {"code", obj.code_},
      {"message", obj.message_},
      {"statusCode", obj.statusCode_},
      {"retryAfter", obj.retryAfter_},
      {"data", obj.data_},
      {"description", obj.description_},
      {"accessDeniedDetail", obj.accessDeniedDetail_},
      {"stack", obj.stack_}
    };
  }

  friend void from_json(const Darabonba::Json &j, ResponseException &obj) {
    from_json(j, static_cast<DaraException&>(obj));
    if (j.contains("statusCode") && !j["statusCode"].is_null()) {
      obj.statusCode_ = j["statusCode"].get<int64_t>();
    }
    if (j.contains("retryAfter") && !j["retryAfter"].is_null()) {
      obj.retryAfter_ = j["retryAfter"].get<int32_t>();
    }
    if (j.contains("stack") && !j["stack"].is_null()) {
      obj.stack_ = j["stack"].get<std::string>();
    }
  }

private:
  std::string stack_;
  std::string statusMessage_;
  mutable std::string msg_;  // Cached message for what()
};

// RequiredArgumentException Class (aligned with tea-python)
class RequiredArgumentException : public DaraException {
public:
  explicit RequiredArgumentException(const std::string &arg);

  RequiredArgumentException(const RequiredArgumentException &) = default;
  RequiredArgumentException(RequiredArgumentException &&) = default;
  virtual ~RequiredArgumentException() = default;
  RequiredArgumentException &operator=(const RequiredArgumentException &) = default;
  RequiredArgumentException &operator=(RequiredArgumentException &&) = default;

  const char *what() const noexcept override;

private:
  std::string arg_;
};

// RetryError Class (aligned with tea-python)
class RetryError : public std::exception {
public:
  explicit RetryError(const std::string &message);

  RetryError(const RetryError &) = default;
  RetryError(RetryError &&) = default;
  virtual ~RetryError() = default;
  RetryError &operator=(const RetryError &) = default;
  RetryError &operator=(RetryError &&) = default;

  const char *what() const noexcept override;

  // Getters (aligned with tea-python)
  const std::string &getMessage() const { return message_; }
  const std::string &getName() const { return name_; }
  const Darabonba::Json &getData() const { return data_; }

  // Setters
  void setData(const Darabonba::Json &data) { data_ = data; }

private:
  std::string message_;
  std::string name_ = "RetryError";
  Darabonba::Json data_;
};

// UnretryableException Class (aligned with tea-python)
class UnretryableException : public DaraException {
public:
  // Constructor that takes RetryPolicyContext
  explicit UnretryableException(const Policy::RetryPolicyContext &context);

  UnretryableException(const UnretryableException &) = default;
  UnretryableException(UnretryableException &&) = default;
  virtual ~UnretryableException() = default;

  UnretryableException &operator=(const UnretryableException &) = default;
  UnretryableException &operator=(UnretryableException &&) = default;

  // Get the inner exception
  std::shared_ptr<DaraException> getLastException() const {
    return lastException_;
  }

  // Get the HTTP request
  std::shared_ptr<Http::Request> getLastRequest() const { return lastRequest_; }

  // Override what() to return inner exception's message
  const char *what() const noexcept override {
    if (lastException_) {
      return lastException_->what();
    }
    return message_.c_str();
  }

private:
  std::shared_ptr<DaraException> lastException_;
  std::shared_ptr<Http::Request> lastRequest_;
};

} // namespace Darabonba
#endif // DARABONBA_EXCEPTIONS_HPP
