#include <darabonba/core.hpp>

Darabonba::Error Darabonba::UnretryableError::getLastException() {
  return _last_exception;
}

Darabonba::Request Darabonba::UnretryableError::getLastRequest() {
  return _last_request;
}
