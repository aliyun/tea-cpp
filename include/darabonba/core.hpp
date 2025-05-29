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
#include <iostream>
#include <sstream>
#include <type_traits>

template<typename T>
void appendToStream(std::ostringstream& oss, T&& arg) {
  oss << arg;
}

template<typename First, typename... Args>
void appendToStream(std::ostringstream& oss, First&& first, Args&&... args) {
  oss << first;
  appendToStream(oss, std::forward<Args>(args)...);
}

template<typename... Args>
std::string stringTemplate(Args&&... args) {
  std::ostringstream oss;
  appendToStream(oss, std::forward<Args>(args)...);
  return oss.str();
}

#define DARA_STRING_TEMPLATE(...) stringTemplate(__VA_ARGS__)

namespace Darabonba {
class Core {
public:
  /**
   * @note This function will the Http::Request
   */
  static std::future<std::shared_ptr<Http::MCurlResponse>>
  doAction(Http::Request &request, const Darabonba::Json &runtime = {});

  static std::string uuid();
  static Json merge(const Json &obj) { return obj; }
  template <typename... Args>
  static Json merge(const Json &arg, const Args &...args) {
    Darabonba::Json ret = arg;
    ret.merge_patch(merge(args...));
    return ret;
  }


};

template <typename T>
inline bool isNull(T* const & ptr);

template <typename T>
inline bool isNull(const T& value);


template <>
inline bool isNull<std::nullptr_t>(const std::nullptr_t&);

Json defaultVal(const Json& a, const Json& b);


bool allowRetry(const RetryOptions& options, const RetryPolicyContext& ctx);
int  getBackoffTime(const RetryOptions& options, const RetryPolicyContext& ctx);
void sleep(int seconds);

} // namespace Darabonba

#endif