#include <darabonba/Exception.hpp>
#include <darabonba/policy/Retry.hpp>
#include <darabonba/http/Request.hpp>

namespace Darabonba {

// ResponseException Implementation
ResponseException::ResponseException() : Exception() {
  name_ = "ResponseException";
}

// RequiredArgumentException Implementation
RequiredArgumentException::RequiredArgumentException(const std::string &arg)
    : Exception("\"" + arg + "\" is required.") {
  name_ = "RequiredArgumentException";
}

const char *RequiredArgumentException::what() const noexcept {
  return msg_.c_str();
}

// RetryError Implementation
RetryError::RetryError(const std::string &message) : message_(message) {}

const char *RetryError::what() const noexcept { return message_.c_str(); }

UnretryableException::UnretryableException(
    const Policy::RetryPolicyContext &context)
    : Exception() {
  name_ = "UnretryableException";

  // Get the exception from context
  auto exception = context.getException();

  // If the exception is a ResponseException, rethrow it directly
  if (exception) {
    auto responseEx = std::dynamic_pointer_cast<ResponseException>(exception);
    if (responseEx) {
      throw *responseEx;
    }
  }

  lastException_ = exception;
  lastRequest_ = context.getLastRequest();
  if (lastException_) {
    msg_ = lastException_->what();
  } else {
    msg_ = "Unretryable exception occurred";
  }
}

}
