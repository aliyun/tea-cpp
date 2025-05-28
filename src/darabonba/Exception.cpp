#include <darabonba/Exception.hpp>

namespace Darabonba {


// RequiredArgumentException Implementation
RequiredArgumentException::RequiredArgumentException(const std::string &arg)
: Exception("\"" + arg + "\" is required.") {}

const char* RequiredArgumentException::what() const noexcept {
    return msg_.c_str();
}

// RetryError Implementation
RetryError::RetryError(const std::string &message) : message_(message) {}

const char* RetryError::what() const noexcept {
    return message_.c_str();
}


} // namespace Darabonba
