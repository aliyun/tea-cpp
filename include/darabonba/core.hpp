#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <darabonba/Runtime.hpp>
#include <darabonba/policy/Retry.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/Request.hpp>
#include <future>
#include <memory>
#include <string>
#include <type_traits>

namespace Darabonba {
class Core {
public:
  /**
   * @note This function will the Http::Request
   */
  static std::future<std::shared_ptr<Http::MCurlResponse>>
  doAction(Http::Request &request, const Darabonba::Json &runtime = {});

  static std::string uuid();
  static void merge_helper(Json &) {} // 递归终止条件

  template <typename T, typename... Args>
  static void merge_helper(Json &result, const T &arg, const Args &...args) {
    result.update(Json(arg));
    merge_helper(result, args...);
  }

  template <typename... Args>
  static Json merge(const Json &arg, const Args &...args) {
    Json result = arg;
    merge_helper(result, args...);
    return result;
  }

};


template <typename T>
inline bool isNull(const T& value) {
  return false; // 默认情况下非指针类型不是nullptr
}

template <typename T>
inline bool isNull(T* const & ptr) {
  return ptr == nullptr;
}

// 对于特定类型的特化，如果该类型有意义
template <>
inline bool isNull<std::nullptr_t>(const std::nullptr_t&) {
  return true;
}

Json defaultVal(const Json& a, const Json& b);


bool allowRetry(const RetryOptions& options, const RetryPolicyContext& ctx);
int  getBackoffTime(const RetryOptions& options, const RetryPolicyContext& ctx);
void sleep(int seconds);

} // namespace Darabonba

#endif