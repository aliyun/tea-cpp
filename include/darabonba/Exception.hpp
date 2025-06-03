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

// ResponseException Class
  class ResponseException : public Exception {
  public:
//    ResponseException(
//        const std::string &code,
//        const std::string &message,
//        int statusCode = 0,
//        int retryAfter = 0,
//        const std::string &description = "",
//        const std::string &accessDeniedDetail = ""
//    ) : Exception(message), statusCode_(statusCode), retryAfter_(retryAfter), code_(code), description_(description),
//        accessDeniedDetail_(accessDeniedDetail) {}

    ResponseException() ;
    ResponseException(const ResponseException &) = default ;
    ResponseException(ResponseException &&) = default ;
    ResponseException(const Darabonba::Json &obj) {
      from_json(obj, *this);
    }
    virtual ~ResponseException() = default ;
    ResponseException& operator=(const ResponseException &) = default ;
    ResponseException& operator=(ResponseException &&) = default ;

    // 反序列化 JSON
    friend void from_json(const Darabonba::Json &j, ResponseException& obj) {
      DARABONBA_PTR_FROM_JSON(statusCode, statusCode_);
      DARABONBA_PTR_FROM_JSON(code, code_);
      DARABONBA_FROM_JSON(msg, msg_);
      DARABONBA_PTR_FROM_JSON(retryAfter, retryAfter_);
      DARABONBA_PTR_FROM_JSON(description, description_);
      DARABONBA_PTR_FROM_JSON(accessDeniedDetail, accessDeniedDetail_);
    }

  private:
    std::shared_ptr<std::string> code_;
    std::shared_ptr<int64_t> statusCode_;
    std::shared_ptr<int64_t> retryAfter_;
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
