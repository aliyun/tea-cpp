#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <darabonba/RuntimeOptions.hpp>
#include <darabonba/policy/Retry.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/Request.hpp>
#include <future>
#include <memory>
#include <string>

namespace Darabonba {
class Core {
public:
  /**
   * @note This function will the Http::Request
   */
  static std::future<std::shared_ptr<Http::MCurlResponse>>
  doAction(Http::Request &request, const Darabonba::Json &runtime = {});
  static bool allowRetry(const RetryOptions& options, const RetryPolicyContext& ctx);
  static int  getBackoffTime(const RetryOptions& options, const RetryPolicyContext& ctx);
  static void sleep(int seconds);

template <typename T>
static T defaultValue(const T &a, const T &b) {
    // 假设 a 是一个指针类型，在使用 nullptr 检查时这样它不会引发错误。
    if constexpr (std::is_pointer<T>::value) {
        if (a == nullptr) {
            return b;
        }
    } else {
        // 处理其他类型，以防止输入非指针类型和不进行错误检查。
        if (a == T{}) {
            return b;
        }
    }
    return a;
}

  static std::string uuid();
  static Json merge(const Json &obj) { return obj; }
  template <typename... Args>
  static Json merge(const Json &arg, const Args &...args) {
    Darabonba::Json ret = arg;
    ret.merge_patch(merge(args...));
    return ret;
  }
  // 基础版：处理指针（包括原始指针和智能指针）
  template <typename T>
  static bool isNull(const T &value) {
      return value == nullptr;
  }

};

} // namespace Darabonba

#endif