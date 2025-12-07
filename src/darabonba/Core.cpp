#include <chrono>
#include <darabonba/Core.hpp>
#include <darabonba/policy/Retry.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#include <objbase.h>
// Undefine Windows macros that conflict with std::min/std::max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#else
#include <uuid/uuid.h>
#endif

using std::string;

// 定义最大和最小延迟时间
const int MAX_DELAY_TIME = 120 * 1000;
const int MIN_DELAY_TIME = 100;

namespace Darabonba {

struct Deleter {
  void operator()(Http::MCurlHttpClient *ptr) {
    delete ptr;
    curl_global_cleanup();
  }
};

string Core::uuid() {
#ifdef _WIN32
  GUID guid;
  if (CoCreateGuid(&guid)) {
    return "";
  }

  std::stringstream str;
  str.setf(std::ios_base::uppercase);
  str << std::hex << std::setfill('0') << std::setw(8) << guid.Data1 << '-'
      << std::setw(4) << guid.Data2 << '-' << std::setw(4) << guid.Data3 << '-'
      << std::setw(2) << (int)guid.Data4[0] << std::setw(2)
      << (int)guid.Data4[1] << '-' << std::setw(2) << (int)guid.Data4[2]
      << std::setw(2) << (int)guid.Data4[3] << '-' << std::setw(2)
      << (int)guid.Data4[4] << std::setw(2) << (int)guid.Data4[5]
      << std::setw(2) << (int)guid.Data4[6] << std::setw(2)
      << (int)guid.Data4[7];

  return str.str();
#else
  std::stringstream str;
  str.setf(std::ios_base::uppercase);
  uuid_t uu;
  uuid_generate(uu);

  str << std::hex << std::setfill('0') << std::setw(2) << (int)uu[0]
      << std::setw(2) << (int)uu[1] << std::setw(2) << (int)uu[2]
      << std::setw(2) << (int)uu[3] << "-" << std::setw(2) << (int)uu[4]
      << std::setw(2) << (int)uu[5] << "-" << std::setw(2) << (int)uu[6]
      << std::setw(2) << (int)uu[7] << "-" << std::setw(2) << (int)uu[8]
      << std::setw(2) << (int)uu[9] << "-" << std::setw(2) << (int)uu[10]
      << std::setw(2) << (int)uu[11] << std::setw(2) << (int)uu[12]
      << std::setw(2) << (int)uu[13] << std::setw(2) << (int)uu[14]
      << std::setw(2) << (int)uu[15];
  return str.str();
#endif
}

std::future<std::shared_ptr<Http::MCurlResponse>>
Core::doAction(Http::Request &request, const Darabonba::Json &runtime) {
  static auto client = []() {
    curl_global_init(CURL_GLOBAL_ALL);
    auto ret = std::unique_ptr<Http::MCurlHttpClient, Deleter>(
        new Http::MCurlHttpClient());
    ret->start();
    return ret;
  }();
  // modfiy the request url
  auto &header = request.header();
  auto &url = request.getUrl();
  if (url.host().empty()) {
    auto it = header.find("host");
    if (it != header.end()) {
      url.setHost(it->second);
    }
  }
  if (url.scheme().empty()) {
    url.setScheme("https");
  }
  return client->makeRequest(request, runtime);
}

bool allowRetry(const RetryOptions& options, const RetryPolicyContext& ctx) {
  if (ctx.retriesAttempted() == 0) {
    return true;
  }

  if (!options.isRetryable()) {
    return false;
  }

  int retriesAttempted = ctx.retriesAttempted();
  shared_ptr<Exception> ex = ctx.exception();

  auto noRetryConditions = options.noRetryCondition();
  for (const auto& condition : noRetryConditions) {
    if (std::find(condition.exception().begin(), condition.exception().end(), ex->name()) != condition.exception().end() ||
        std::find(condition.errorCode().begin(), condition.errorCode().end(), ex->code()) != condition.errorCode().end()) {
      return false;
        }
  }

  auto retryConditions = options.retryCondition();
  for (const auto& condition : retryConditions) {
    if (std::find(condition.exception().begin(), condition.exception().end(), ex->name()) == condition.exception().end() &&
        std::find(condition.errorCode().begin(), condition.errorCode().end(), ex->code()) == condition.errorCode().end()) {
      continue;
        }

    if (retriesAttempted >= condition.maxAttempts()) {
      return false;
    }
    return true;
  }

  return false;
}

int getBackoffTime(const RetryOptions& options, const RetryPolicyContext& ctx) {
  shared_ptr<Exception> ex = ctx.exception();
  auto conditions = options.retryCondition();

  for (const auto& condition : conditions) {
    if (std::find(condition.exception().begin(), condition.exception().end(), ex->name()) == condition.exception().end() &&
        std::find(condition.errorCode().begin(), condition.errorCode().end(), ex->code()) == condition.errorCode().end()) {
      continue;
        }

    int maxDelay = (condition.maxDelay() > 0) ? condition.maxDelay() : MAX_DELAY_TIME;
    int retryAfter = ex->retry_fater();

    if (retryAfter > 0) {
      return (std::min)(retryAfter, maxDelay);
    }

    if (!condition.backoff()) {
      return MIN_DELAY_TIME;
    }

    BackoffPolicy* strategy = condition.backoff().get();
    if (strategy) {
      return (std::min)(strategy->getDelayTime(ctx), maxDelay);
    }

    return MIN_DELAY_TIME;
  }

  return MIN_DELAY_TIME;
}

void sleep(int millisecond) {
  std::this_thread::sleep_for(std::chrono::milliseconds(millisecond));
}

Json defaultVal(const Json& a, const Json& b) {
  if (a.is_null()) {
    return b;
  }
  switch(a.type()) {
  case Json::value_t::string:
    return a.get<std::string>().empty() ? b : a;
  case Json::value_t::boolean:
    return !a.get<bool>() ? b : a;;
  case Json::value_t::number_integer:
    return a.get<int>() == 0 ? b : a;;
  case Json::value_t::number_unsigned:
    return a.get<unsigned int>() == 0  ? b : a;;
  case Json::value_t::number_float:
    return a.get<float>() == 0.0 ? b : a;;
  case Json::value_t::binary:
    return a.get_binary().empty() ? b : a;;
  case Json::value_t::object:
  case Json::value_t::array:
    return a.empty() ? b : a;;
  default:
    return b; // All other non-valid or discarded cases
  }
  return a;
}

} // namespace Darabonba