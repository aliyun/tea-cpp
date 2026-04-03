#ifndef DARABONBA_TYPE_H_
#define DARABONBA_TYPE_H_

#include <cstdint>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// 特化处理 nlohmann::json 类型：字符串类型提取值，其他类型保持原样
// 需要同时提供 const 和非 const 版本，否则转发引用会优先匹配非 const 左值
inline void appendToStream(std::ostringstream &oss, const nlohmann::json &arg) {
  if (arg.is_string()) {
    oss << arg.get<std::string>();
  } else {
    oss << arg;
  }
}

inline void appendToStream(std::ostringstream &oss, nlohmann::json &arg) {
  if (arg.is_string()) {
    oss << arg.get<std::string>();
  } else {
    oss << arg;
  }
}

inline void appendToStream(std::ostringstream &oss, nlohmann::json &&arg) {
  if (arg.is_string()) {
    oss << arg.get<std::string>();
  } else {
    oss << arg;
  }
}

// 通用模板处理其他类型（禁用 nlohmann::json 类型）
template <typename T>
typename std::enable_if<!std::is_same<typename std::decay<T>::type, nlohmann::json>::value>::type
appendToStream(std::ostringstream &oss, T &&arg) {
  oss << std::forward<T>(arg);
}

template <typename First, typename... Args>
void appendToStream(std::ostringstream &oss, First &&first, Args &&...args) {
  appendToStream(oss, std::forward<First>(first));  // 调用单参数版本，确保 json 特化生效
  appendToStream(oss, std::forward<Args>(args)...);
}

template <typename... Args> std::string stringTemplate(Args &&...args) {
  std::ostringstream oss;
  appendToStream(oss, std::forward<Args>(args)...);
  return oss.str();
}

#define DARA_STRING_TEMPLATE(...) stringTemplate(__VA_ARGS__)

namespace Darabonba {

using Json = nlohmann::json;
using Bytes = nlohmann::json::binary_t;
// using Bytes = std::vector<uint8_t>;

} // namespace Darabonba

#endif