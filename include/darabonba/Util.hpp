#ifndef DARABONBA_UTIL_H_
#define DARABONBA_UTIL_H_

#include <chrono>
#include <cstdint>
#include <darabonba/Core.hpp>
#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace Darabonba {
class Util {
public:
  /************************** helper **************************/
  static std::string getNonce() { return Core::uuid(); }

  static std::string getDateUTCString();

  static std::string getUserAgent(const std::string &userAgent);

  static void sleep(int millisecond) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millisecond));
  }

  static void validateModel(const Model &obj) { obj.validate(); }

  /************************** default **************************/
  static std::string defaultString(const std::string &real,
                                   const std::string &defaultVal) {
    return real.empty() ? defaultVal : real;
  }

  static int64_t defaultNumber(int64_t real, int64_t defaultVal) {
    return real ? real : defaultVal;
  }
  static uint64_t defaultNumber(uint64_t real, uint64_t defaultVal) {
    return real ? real : defaultVal;
  }

  /************************** parse **************************/
  static std::map<std::string, std::string> stringifyMapValue(const Json &m) {
    if (m.empty() || m.is_null()) {
      return {};
    }
    return m.get<std::map<std::string, std::string>>();
  }

  static Json anyifyMapValue(const std::map<std::string, std::string> &m) {
    return m;
  }

  static Json toMap(const Model &in) { return in.toMap(); }

  static Bytes toBytes(const std::string &val) {
    Bytes b;
    b.assign(val.begin(), val.end());
    return b;
  }

  static std::string toString(const Bytes &val) {
    return std::string(val.begin(), val.end());
  }

  static std::vector<Json> toArray(const Json &input) {
    if (!input.is_array()) {
      throw Exception("input is not a array");
    }
    return input.get<std::vector<Json>>();
  };

  static std::string toFormString(const Json &val);

  static std::string toJSONString(const Json &value) { return value.dump(); }

  static Json parseJSON(const std::string &val) { return Json::parse(val); }

  static Bytes readAsBytes(std::shared_ptr<IStream> raw);

  static std::string readAsString(std::shared_ptr<IStream> raw);

  static Json readAsJSON(std::shared_ptr<IStream> raw);

  /************************** assert **************************/
  static bool isUnset(const Json &value) {
    return value.empty() || value.is_null();
  };
  static bool isUnset(const Model &model) { return model.empty(); }
  template <typename T> static bool isUnset(std::shared_ptr<T> value) {
    return value == nullptr;
  }
  template <typename T, typename Deleter = std::default_delete<T>>
  static bool isUnset(std::unique_ptr<T, Deleter> value) {
    return value == nullptr;
  }
  template <typename T, typename Deleter>
  static bool isUnset(std::unique_ptr<T[], Deleter> value) {
    return value == nullptr;
  }
  template <typename T> static bool isUnset(T *value) {
    return value == nullptr;
  }

  static bool empty(const std::string &val) { return val.empty(); }

  static bool equalString(const std::string &val1, const std::string &val2) {
    return val1 == val2;
  }

  template <
      typename T1, typename T2,
      typename std::enable_if<std::is_arithmetic<T1>::value, bool>::type = true,
      typename std::enable_if<std::is_arithmetic<T2>::value, bool>::type = true>
  static bool equalNumber(T1 val1, T2 val2) {
    return val1 == val2;
  }

  static bool assertAsBoolean(const Json &value);

  static std::string assertAsString(const Json &value);

  static Bytes assertAsBytes(const Json &value);

  static int64_t assertAsNumber(const Json &value);

  // TODO: std::map, std::unordered_map, Daraboba::Model, stream
  static const Json &assertAsMap(const Json &value);

  // TODO:
  static IStream *assertAsReadable(Stream *value);

  static bool is2xx(int64_t code) { return 200 <= code && code < 300; }

  static bool is3xx(int64_t code) { return 300 <= code && code < 400; }

  static bool is4xx(int64_t code) { return 400 <= code && code < 500; }

  static bool is5xx(int64_t code) { return 500 <= code && code < 600; }
};
} // namespace Darabonba

#endif
