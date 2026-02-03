#include <darabonba/Exception.hpp>
#include <darabonba/policy/Retry.hpp>
#include <darabonba/http/Request.hpp>

namespace Darabonba {

// ResponseException Implementation
ResponseException::ResponseException() : DaraException() {
  name_ = "ResponseException";
}

// RequiredArgumentException Implementation
RequiredArgumentException::RequiredArgumentException(const std::string &arg)
    : DaraException("\"" + arg + "\" is required."), arg_(arg) {
  name_ = "RequiredArgumentException";
}

const char *RequiredArgumentException::what() const noexcept {
  return message_.c_str();
}

// RetryError Implementation
RetryError::RetryError(const std::string &message) : message_(message) {}

const char *RetryError::what() const noexcept { return message_.c_str(); }

// UnretryableException Implementation
UnretryableException::UnretryableException(
    const Policy::RetryPolicyContext &context)
    : DaraException() {
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
    message_ = lastException_->what();
  } else {
    message_ = "Unretryable exception occurred";
  }
}

}
