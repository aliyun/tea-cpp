#ifndef DARABONBA_EXCEPTIONS_HPP
#define DARABONBA_EXCEPTIONS_HPP

#include <string>
#include <exception>
#include <memory>
#include <darabonba/Type.hpp>

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
    ResponseException(
        const std::string &code,
        const std::string &message,
        int statusCode = 0,
        int retryAfter = 0,
        const std::string &description = "",
        const std::string &accessDeniedDetail = ""
    ) : Exception(message), statusCode_(statusCode), retryAfter_(retryAfter), code_(code), description_(description),
        accessDeniedDetail_(accessDeniedDetail) {}

    ResponseException(const nlohmann::json &obj) {
      from_json(obj, *this);
    }

    // 序列化 JSON
    friend void to_json(nlohmann::json &j, const ResponseException &ex) {
      j = nlohmann::json{
          {"code",               ex.code_},
          {"message",            ex.msg_},
          {"statusCode",         ex.statusCode_},
          {"retryAfter",         ex.retryAfter_},
          {"description",        ex.description_},
          {"accessDeniedDetail", ex.accessDeniedDetail_}
      };
    }

    // 反序列化 JSON
    friend void from_json(const nlohmann::json &j, ResponseException &ex) {
      j.at("code").get_to(ex.code_);
      j.at("message").get_to(ex.msg_);
      j.at("statusCode").get_to(ex.statusCode_);
      j.at("retryAfter").get_to(ex.retryAfter_);
      j.at("description").get_to(ex.description_);
      j.at("accessDeniedDetail").get_to(ex.accessDeniedDetail_);
    }

  private:
    std::string code_;
    int statusCode_;
    int retryAfter_;
    std::string description_;
    std::string accessDeniedDetail_;
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
