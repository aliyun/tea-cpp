#include <chrono>
#include <darabonba/Core.hpp>
#include <darabonba/policy/Retry.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <fstream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#include <objbase.h>
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
  str << std::hex << std::setfil('0') << std::setw(8) << guid.Data1 << '-'
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
  auto &url = request.url();
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

bool allowRetry(const RetryOptions* options, const RetryPolicyContext* ctx) {
  if (ctx->retriesAttempted() == 0) {
    return true;
  }

  if (options == nullptr || !options->isRetryable()) {
    return false;
  }

  int retriesAttempted = ctx->retriesAttempted();
  Exception ex = ctx->exception();

  auto noRetryConditions = options->noRetryCondition();
  for (const auto& condition : noRetryConditions) {
    if (std::find(condition.exception().begin(), condition.exception().end(), ex.name()) != condition.exception().end() ||
        std::find(condition.errorCode().begin(), condition.errorCode().end(), ex.code()) != condition.errorCode().end()) {
      return false;
        }
  }

  auto retryConditions = options->retryCondition();
  for (const auto& condition : retryConditions) {
    if (std::find(condition.exception().begin(), condition.exception().end(), ex.name()) == condition.exception().end() &&
        std::find(condition.errorCode().begin(), condition.errorCode().end(), ex.code()) == condition.errorCode().end()) {
      continue;
        }

    if (retriesAttempted >= condition.maxAttempts()) {
      return false;
    }
    return true;
  }

  return false;
}

int getBackoffTime(const RetryOptions* options, const RetryPolicyContext* ctx) {
  Exception ex = ctx->exception();
  auto conditions = options->retryCondition();

  for (const auto& condition : conditions) {
    if (std::find(condition.exception().begin(), condition.exception().end(), ex.name()) == condition.exception().end() &&
        std::find(condition.errorCode().begin(), condition.errorCode().end(), ex.code()) == condition.errorCode().end()) {
      continue;
        }

    int maxDelay = (condition.maxDelay() > 0) ? condition.maxDelay() : MAX_DELAY_TIME;
    int retryAfter = ex.retry_fater();

    if (retryAfter > 0) {
      return std::min(retryAfter, maxDelay);
    }

    if (!condition.backoff()) {
      return MIN_DELAY_TIME;
    }

    BackoffPolicy* strategy = condition.backoff().get();
    if (strategy) {
      return std::min(strategy->getDelayTime(*ctx), maxDelay);
    }

    return MIN_DELAY_TIME;
  }

  return MIN_DELAY_TIME;
}

void sleep(int millisecond) {
  std::this_thread::sleep_for(std::chrono::milliseconds(millisecond));
}

Json defaultVal(const Json& a, const Json& b) {
  if (a.is_null() || a.empty()) {
    return b;
  }
  return a;
}

} // namespace Darabonba