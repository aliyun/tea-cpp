#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <darabonba/RuntimeOptions.hpp>
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
  static bool allowRetry(const Json &retry, int retryTimes);
  static int getBackoffTime(const Json &backoff, int retryTimes);
  static void sleep(int seconds);
  static std::string uuid();
  static Json merge(const Json &obj) { return obj; }
  template <typename... Args>
  static Json merge(const Json &arg, const Args &...args) {
    Darabonba::Json ret = arg;
    ret.merge_patch(merge(args...));
    return ret;
  }
};

} // namespace Darabonba

#endif