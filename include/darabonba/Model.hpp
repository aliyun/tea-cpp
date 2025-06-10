#ifndef DARABONBA_MODEL_H_
#define DARABONBA_MODEL_H_

#include <darabonba/Type.hpp>
#include <regex>
#include <type_traits>


#define DARABONBA_PTR_TO_JSON(key, attr)                                       \
  if (obj.attr) {                                                              \
    j[#key] = *(obj.attr);                                                     \
  }

#define DARABONBA_TO_JSON(key, attr) j[#key] = obj.attr;

#define DARABONBA_ANY_TO_JSON(key, attr)                                       \
  if (!obj.attr.is_null()) {                                                   \
    j[#key] = obj.attr;                                                        \
  }

#define DARABONBA_PTR_FROM_JSON(key, attr)                                     \
  if (j.count(#key)) {                                                         \
    if (j[#key].is_null()) {                                                   \
      obj.attr = nullptr;                                                      \
    } else {                                                                   \
      using Type = std::remove_reference<decltype(*obj.attr)>::type;           \
      obj.attr = std::make_shared<Type>(j[#key].template get<Type>());                  \
    }                                                                          \
  }

#define DARABONBA_ANY_FROM_JSON(key, attr)                                     \
  if (j.count(#key)) {                                                         \
    obj.attr = j[#key];                                                        \
  }

#define DARABONBA_FROM_JSON(key, attr)                                         \
  if (j.count(#key)) {                                                         \
    if (j[#key].is_null()) {                                                  \
      obj.attr = decltype(obj.attr)();                                         \
    } else {                                                                   \
      obj.attr =                                                               \
          j[#key].get<std::remove_reference<decltype(obj.attr)>::type>();      \
    }                                                                          \
  }

#define DARABONBA_PTR_SET_VALUE(attr, value)                                   \
  if (this->attr) {                                                            \
    *attr = (value);                                                           \
  } else {                                                                     \
    attr =                                                                     \
        std::make_shared<std::remove_reference<decltype(*this->attr)>::type>(  \
            (value));                                                          \
  }                                                                            \
  return *this;

#define DARABONBA_PTR_SET_RVALUE(attr, value)                                  \
  if (this->attr) {                                                            \
    *attr = std::move(value);                                                  \
  } else {                                                                     \
    attr =                                                                     \
        std::make_shared<std::remove_reference<decltype(*this->attr)>::type>(  \
            std::move(value));                                                 \
  }                                                                            \
  return *this;

#define DARABONBA_PTR_GET_DEFAULT(attr, ...)                                   \
  if (this->attr) {                                                            \
    return *attr;                                                              \
  }                                                                            \
  return __VA_ARGS__;

#define DARABONBA_PTR_GET(attr, ...)                                          \
  if (this->attr) {                                                            \
    return *attr;                                                              \
  }                                                                            \
  return __VA_ARGS__();

#define DARABONBA_PTR_GET_CONST(attr, ...)                                     \
  static const  __VA_ARGS__ empty;                                             \
  if (this->attr) {                                                            \
    return *attr;                                                              \
  }                                                                            \
  return empty;

#define DARABONBA_GET(attr) return this->attr;

#define DARABONBA_SET_VALUE(attr, value)                                       \
  this->attr = value;                                                          \
  return *this;

#define DARABONBA_SET_RVALUE(attr, value)                                      \
  this->attr = std::move(value);                                               \
  return *this;

#define DARABONBA_VALIDATE_REQUIRED(attr)                                      \
  if (this->attr == nullptr) {                                                 \
    throw Darabonba::ValidateException("RequiredValidateError",                \
                                       #attr " is required.");                 \
  }

#define DARABONBA_VALIDATE_MAX_LENGTH(attr, length)                            \
  if (this->attr != nullptr) {                                                 \
    auto size = this->attr->size();                                            \
    if (size > (length)) {                                                     \
      throw Darabonba::ValidateException("MaxLengthValidateError",             \
                                         #attr " is exceed max-length: " +     \
                                             std::to_string((length)));        \
    }                                                                          \
  }

#define DARABONBA_VALIDATE_MIN_LENGTH(attr, length)                            \
  if (this->attr != nullptr) {                                                 \
    auto size = this->attr->size();                                            \
    if (size < (length)) {                                                     \
      throw Darabonba::ValidateException("MaxLengthValidateError",             \
                                         #attr " is exceed max-length: " +     \
                                             std::to_string((length)));        \
    }                                                                          \
  }

#define DARABONBA_VALIDATE_MAXIMUM(attr, val)                                  \
  if (this->attr != nullptr) {                                                 \
    const auto &value = this->attr;                                            \
    if (value > (val)) {                                                       \
      throw Darabonba::ValidateException("MaximumValidateError",               \
                                         #attr " cannot be greater than " +    \
                                             std::to_string((val)));           \
    }                                                                          \
  }

#define DARABONBA_VALIDATE_MINIMUM(attr, val)                                  \
  if (this->attr != nullptr) {                                                 \
    const auto &value = *(this->attr);                                         \
    if (value < (val)) {                                                       \
      throw Darabonba::ValidateException("MinimumValidateError",               \
                                         #attr " cannot be less than " +       \
                                             std::to_string((val)));           \
    }                                                                          \
  }

#define DARABONBA_VALIDATE_PATTERN(attr, val)                                  \
  if (this->attr != nullptr) {                                                 \
    const auto &value = *(this->attr);                                         \
    std::regex pattern(val, std::regex::icase);                                \
    if (!regex_search(value, pattern)) {                                       \
      throw Darabonba::ValidateException("PatternValidateError",               \
                                         #attr " is not match " +              \
                                             std::to_string((val)));           \
    }                                                                          \
  }

namespace Darabonba {
class Model {
public:
  // ~Model() {}
  virtual void validate() const = 0;
  virtual Json toMap() const = 0;
  virtual void fromMap(const Json &) = 0;
  virtual bool empty() const = 0;
};

} // namespace Darabonba

#endif