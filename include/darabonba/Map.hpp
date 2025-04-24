#ifndef DARABONBA_ASSERT_H_
#define DARABONBA_ASSERT_H_

#include <darabonba/Type.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace Darabonba {
class Map {
public:
  template <typename T>
  static size_t size(const std::unordered_map<std::string, T> &map) {
    return map.size();
  }
  template <typename T>
  static size_t size(const std::map<std::string, T> &map) {
    return map.size();
  }
  static size_t size(const Json &map) { return map.size(); }

  template <typename T>
  static std::vector<std::string>
  keySet(const std::unordered_map<std::string, T> &map) {
    std::vector<std::string> ret;
    ret.reserve(map.size());
    for (auto it = map.begin(); it != map.end(); ++it) {
      ret.emplace_back(it->first);
    }
    return ret;
  }
  template <typename T>
  static std::vector<std::string> keySet(const std::map<std::string, T> &map) {
    std::vector<std::string> ret;
    ret.reserve(map.size());
    for (auto it = map.begin(); it != map.end(); ++it) {
      ret.emplace_back(it->first);
    }
    return ret;
  }
  static std::vector<std::string> keySet(const Json &map) {
    std::vector<std::string> ret;
    ret.reserve(map.size());
    for (auto it = map.begin(); it != map.end(); ++it) {
      ret.emplace_back(it.key());
    }
    return ret;
  }
};
} // namespace Darabonba

#endif
