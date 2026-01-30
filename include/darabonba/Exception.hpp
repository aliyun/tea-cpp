#ifndef DARABONBA_EXCEPTIONS_HPP
#define DARABONBA_EXCEPTIONS_HPP

#include <string>
#include <exception>
#include <memory>
#include <darabonba/Type.hpp>
#include <darabonba/Model.hpp>

namespace Darabonba {

// Exception Class
class Exception : public std::exception {
public:
  Exception() = default;

  Exception(const std::string &msg) : msg_(msg) {}

  Exception(std::string &&msg) : msg_(std::move(msg)) {}

  Exception(const Exception &ex) = default;

  Exception(Exception &&) = default;

  virtual ~Exception() = default;

  Exception &operator=(const Exception &) = default;

  Exception &operator=(Exception &&) = default;

  friend void to_json(nlohmann::json &j, const Exception &ex) {
    j = nlohmann::json{{"message", ex.msg_}};
  }

  friend void from_json(const nlohmann::json &j, Exception &ex) {
    j.at("message").get_to(ex.msg_);
  }


  virtual const char *what() const noexcept override { return msg_.c_str(); }

  // Getters for each field to use in code, if needed
  const std::string& getMessage() const { return msg_; }
  const std::string& getName() const { return name_; }
  const std::string& getCode() const { return  code_;}
  const std::int32_t& getRetryAfter() const { return  retry_after_;}

protected:
  std::string msg_;
  std::string name_ = "Exception";
  std::string code_;
  std::int32_t retry_after_ = 0;  // 初始化为 0
};

class ValidateException : public Exception {
public:
  ValidateException() {
    name_ = "ValidateException";
  }

  ValidateException(const std::string &code, const std::string msg)
      : Exception(code + ": " + msg) {
    name_ = "ValidateException";
  }

  virtual ~ValidateException() = default;

protected:
};

// ResponseException Class
class ResponseException : public Exception {
public:
  ResponseException() ;
  ResponseException(const ResponseException &) = default ;
  ResponseException(ResponseException &&) = default ;
  ResponseException(const Darabonba::Json &obj) {
    name_ = "ResponseException";
    from_json(obj, *this);
    msg_ = DARA_STRING_TEMPLATE("SDKError:\n   StatusCode: " , getStatusCode() , ", Code: " , getCode(), ", Message: " , getMessage());
  }
  virtual ~ResponseException() = default ;
  ResponseException& operator=(const ResponseException &) = default ;
  ResponseException& operator=(ResponseException &&) = default ;
  inline int64_t getStatusCode() const { DARABONBA_PTR_GET_DEFAULT(statusCode_, 0) };
  inline int64_t getRetryAfter() const { DARABONBA_PTR_GET_DEFAULT(retryAfter_, 0) };
  inline std::string getCode() const { DARABONBA_PTR_GET_DEFAULT(code_, "") };
  inline std::string getMessage() const { DARABONBA_PTR_GET_DEFAULT(message_, "") };
  inline std::string getDescription() const { DARABONBA_PTR_GET_DEFAULT(description_, "") };
  inline std::string getAccessDeniedDetail() const { DARABONBA_PTR_GET_DEFAULT(accessDeniedDetail_, "") };

  // 反序列化 JSON
  friend void from_json(const Darabonba::Json &j, ResponseException& obj) {
    DARABONBA_PTR_FROM_JSON(statusCode, statusCode_);
    DARABONBA_PTR_FROM_JSON(code, code_);
    DARABONBA_PTR_FROM_JSON(message, message_);
    DARABONBA_PTR_FROM_JSON(retryAfter, retryAfter_);
    DARABONBA_PTR_FROM_JSON(description, description_);
    DARABONBA_PTR_FROM_JSON(accessDeniedDetail, accessDeniedDetail_);
  }

private:
  std::shared_ptr<std::string> code_;
  std::shared_ptr<int64_t> statusCode_;
  std::shared_ptr<int64_t> retryAfter_;
  std::shared_ptr<std::string> message_;
  std::shared_ptr<std::string> description_;
  std::shared_ptr<std::string> accessDeniedDetail_;
};


// RequiredArgumentException Class
class RequiredArgumentException : public Exception {
public:
  explicit RequiredArgumentException(const std::string &arg);

  const char *what() const noexcept override;

private:
  std::string arg_;
};

// RetryError Class
class RetryError : public std::exception {
public:
  explicit RetryError(const std::string &message);

  const char *what() const noexcept override;

private:
  std::string message_;
};
}// namesapce Darabonba
#endif // DARABONBA_EXCEPTIONS_HPP
