#include <darabonba/Exception.hpp>

// TeaException Implementation
TeaException::TeaException(const std::string &message) : message_(message) {}

const char* TeaException::what() const noexcept {
    return message_.c_str();
}

// DaraException Implementation
DaraException::DaraException(const std::string &code, const std::string &message, const std::string &description, const std::string &accessDeniedDetail)
: TeaException(message), code_(code), description_(description), accessDeniedDetail_(accessDeniedDetail) {}

const char* DaraException::what() const noexcept {
    return (std::string("Error: ") + code_ + " " + message_ + "; Description: " + description_).c_str();
}

// ResponseException Implementation
ResponseException::ResponseException(const std::string &code, const std::string &message, int statusCode, int retryAfter, const std::string &description, const std::string &accessDeniedDetail)
: DaraException(code, message, description, accessDeniedDetail), statusCode_(statusCode), retryAfter_(retryAfter) {}

// RequiredArgumentException Implementation
RequiredArgumentException::RequiredArgumentException(const std::string &arg)
: TeaException("\"" + arg + "\" is required.") {}

const char* RequiredArgumentException::what() const noexcept {
    return message_.c_str();
}

// RetryError Implementation
RetryError::RetryError(const std::string &message) : message_(message) {}

const char* RetryError::what() const noexcept {
    return message_.c_str();
}

// UnretryableException Implementation
UnretryableException::UnretryableException(const std::string &message, const std::shared_ptr<TeaException> &internalEx)
: TeaException(message), inner_exception_(internalEx) {}

const char* UnretryableException::what() const noexcept {
    return inner_exception_ ? inner_exception_->what() : message_.c_str();
}
