#ifndef DARABONBA_MODEL_H_
#define DARABONBA_MODEL_H_

#include <darabonba/Type.hpp>
#include <regex>
#include <type_traits>
#include <sstream>
#include <limits>
#include <cstdint>

namespace Darabonba {
namespace detail {

// ========== 通用模板（适用于所有复杂类型）==========
template <typename T>
inline T safe_convert(const Json& j) {
  // 对于复杂类型，直接使用 get<T>()
  // 如果类型不匹配，让 nlohmann::json 抛出异常
  return j.template get<T>();
}

// ========== std::string 特化版本 ==========
template <>
inline std::string safe_convert<std::string>(const Json& j) {
  // Fast path: 类型匹配时直接返回
  try {
    return j.get<std::string>();
  } catch (...) {
    // 继续尝试类型转换
  }
  
  // Null 处理
  if (j.is_null()) {
    return "";
  }
  
  // int -> string
  if (j.is_number_integer()) {
    return std::to_string(j.get<long long>());
  }
  
  // float -> string
  if (j.is_number_float()) {
    double val = j.get<double>();
    if (val == static_cast<long long>(val)) {
      return std::to_string(static_cast<long long>(val));
    } else {
      return std::to_string(val);
    }
  }
  
  // bool -> string
  if (j.is_boolean()) {
    return j.get<bool>() ? "true" : "false";
  }
  
  // 其他类型 -> string
  return j.dump();
}

// ========== int32_t 特化版本 ==========
template <>
inline int32_t safe_convert<int32_t>(const Json& j) {
  // Fast path
  try {
    return j.get<int32_t>();
  } catch (...) {
    // 继续尝试类型转换
  }
  
  // Null 处理
  if (j.is_null()) {
    return 0;
  }
  
  // string -> int32
  if (j.is_string()) {
    try {
      std::string str = j.get<std::string>();
      long long val = std::stoll(str);
      if (val < INT32_MIN || val > INT32_MAX) {
        throw std::out_of_range("Value out of int32 range");
      }
      return static_cast<int32_t>(val);
    } catch (...) {
      throw std::runtime_error("Cannot convert string to int32");
    }
  }
  
  // bool -> int32
  if (j.is_boolean()) {
    return j.get<bool>() ? 1 : 0;
  }
  
  throw std::runtime_error("Cannot convert JSON value to int32");
}

// ========== int64_t 特化版本 ==========
template <>
inline int64_t safe_convert<int64_t>(const Json& j) {
  try {
    return j.get<int64_t>();
  } catch (...) {}
  
  if (j.is_null()) {
    return 0;
  }
  
  if (j.is_string()) {
    try {
      return std::stoll(j.get<std::string>());
    } catch (...) {
      throw std::runtime_error("Cannot convert string to int64");
    }
  }
  
  if (j.is_boolean()) {
    return j.get<bool>() ? 1 : 0;
  }
  
  throw std::runtime_error("Cannot convert JSON value to int64");
}

// ========== bool 特化版本 ==========
template <>
inline bool safe_convert<bool>(const Json& j) {
  try {
    return j.get<bool>();
  } catch (...) {}
  
  if (j.is_null()) {
    return false;
  }
  
  // string -> bool
  if (j.is_string()) {
    std::string str = j.get<std::string>();
    return (str == "true" || str == "TRUE" || str == "1");
  }
  
  // int -> bool
  if (j.is_number()) {
    return j.get<long long>() != 0;
  }
  
  throw std::runtime_error("Cannot convert JSON value to bool");
}

} // namespace detail
} // namespace Darabonba

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
      obj.attr = std::make_shared<Type>(                                       \
          Darabonba::detail::safe_convert<Type>(j[#key]));                     \
    }                                                                          \
  }

#define DARABONBA_ANY_FROM_JSON(key, attr)                                     \
  if (j.count(#key)) {                                                         \
    obj.attr = j[#key];                                                        \
  }

#define DARABONBA_FROM_JSON(key, attr)                                         \
  if (j.count(#key)) {                                                         \
    if (j[#key].is_null()) {                                                   \
      obj.attr = decltype(obj.attr)();                                         \
    } else {                                                                   \
      using Type = std::remove_reference<decltype(obj.attr)>::type;            \
      obj.attr = Darabonba::detail::safe_convert<Type>(j[#key]);               \
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

#define DARABONBA_PTR_GET(attr, ...)                                           \
  if (this->attr) {                                                            \
    return *attr;                                                              \
  }                                                                            \
  return __VA_ARGS__();

#define DARABONBA_PTR_GET_CONST(attr, ...)                                     \
  static const __VA_ARGS__ empty;                                              \
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
  virtual ~Model() = default;
  
  // Rule of Five: explicitly default copy/move operations
  Model() = default;
  Model(const Model&) = default;
  Model(Model&&) = default;
  Model& operator=(const Model&) = default;
  Model& operator=(Model&&) = default;
  
  virtual void validate() const = 0;
  virtual Json toMap() const = 0;
  virtual void fromMap(const Json &) = 0;
  virtual bool empty() const = 0;
};

} // namespace Darabonba

#endif