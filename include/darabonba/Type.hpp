#ifndef DARABONBA_TYPE_H_
#define DARABONBA_TYPE_H_

#include <cstdint>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// 特化处理 nlohmann::json 类型：字符串类型提取值，其他类型保持原样
inline void appendToStream(std::ostringstream &oss, const nlohmann::json &arg) {
  if (arg.is_string()) {
    oss << arg.get<std::string>();
  } else {
    oss << arg;
  }
}

// 通用模板处理其他类型
template <typename T> void appendToStream(std::ostringstream &oss, T &&arg) {
  oss << std::forward<T>(arg);
}

template <typename First, typename... Args>
void appendToStream(std::ostringstream &oss, First &&first, Args &&...args) {
  oss << first;
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