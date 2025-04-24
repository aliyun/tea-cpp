#include <chrono>
#include <darabonba/Core.hpp>
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

namespace Darabonba {

struct Deleter {
  void operator()(Http::MCurlHttpClient *ptr) {
    delete ptr;
    curl_global_cleanup();
  }
};

void Core::sleep(int seconds) {
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

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

int Core::getBackoffTime(const Json &backoff, int retryTimes) {
  if (backoff.is_null() || backoff.empty()) {
    return 0;
  }
  string policy = backoff.value("policy", "no");
  int backOffTime = 0;
  if (policy == "no") {
    return backOffTime;
  }

  backOffTime = backoff.value("period", backOffTime);
  if (backOffTime <= 0) {
    return retryTimes;
  }
  return backOffTime;
}

bool Core::allowRetry(const Json &retry, int retryTimes) {
  if (retryTimes == 0)
    return true;
  if (retry.is_null() || retry.empty()) {
    return false;
  }
  bool retryable = retry.value("retryable", false);
  if (retryable) {
    int maxAttempts = retry.value("maxAttempts", 0);
    return maxAttempts >= retryTimes;
  }
  return false;
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

} // namespace Darabonba