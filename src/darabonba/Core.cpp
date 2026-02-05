#include <algorithm>
#include <chrono>
#include <darabonba/Core.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/policy/Retry.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <memory>

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

// Global SDK state management
namespace {
struct SDKState {
  std::atomic<bool> initialized{false};
  std::mutex mutex;
  std::map<ConnectionPoolConfig, std::unique_ptr<Http::MCurlHttpClient>> http_clients;
};

SDKState& GetSDKState() {
  static SDKState state;
  return state;
}

// Internal initialization - called automatically on first use
void EnsureInitialized() {
  auto& state = GetSDKState();
  if (state.initialized.load()) {
    return;
  }
  std::lock_guard<std::mutex> lock(state.mutex);
  if (!state.initialized.load()) {
    curl_global_init(CURL_GLOBAL_ALL);
    state.initialized.store(true);
  }
}

// Helper to extract connection pool config from request
ConnectionPoolConfig getConnectionPoolConfig(Http::Request &request, const Darabonba::Json &runtime) {
  ConnectionPoolConfig config;
  
  auto &url = request.getUrl();
  auto &header = request.getHeader();
  
  // Extract host from URL or header
  if (!url.getHost().empty()) {
    config.host = url.getHost();
  } else {
    auto it = header.find("host");
    if (it != header.end()) {
      config.host = it->second;
    }
  }
  
  // Extract connection pool configuration from runtime options
  if (!runtime.is_null()) {
    config.max_connections = runtime.value("maxConnections", 5UL);
    config.connection_idle_timeout = runtime.value("connectionIdleTimeout", 300L);
    config.pipelining = runtime.value("pipelining", false);
    config.max_host_connections = runtime.value("maxHostConnections", 2UL);
  }
  
  return config;
}

// Helper to extract request-level config
RequestConfig getRequestConfig(const Darabonba::Json &runtime) {
  RequestConfig config;
  
  if (!runtime.is_null()) {
    config.connect_timeout_ms = runtime.value("connectTimeout", 5000L);
    config.read_timeout_ms = runtime.value("readTimeout", 0L);
    config.ignore_ssl = runtime.value("ignoreSSL", false);
    config.http_proxy = runtime.value("httpProxy", "");
    config.https_proxy = runtime.value("httpsProxy", "");
    config.no_proxy = runtime.value("noProxy", "");
    config.credential = runtime.value("credential", "");
  }
  
  return config;
}

} // anonymous namespace



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
  // Ensure SDK is initialized (lazy initialization)
  EnsureInitialized();
  
  auto& state = GetSDKState();
  
  // Get connection pool configuration (host-level)
  ConnectionPoolConfig pool_config = getConnectionPoolConfig(request, runtime);
  // Get request-level configuration
  RequestConfig request_config = getRequestConfig(runtime);
  
  std::lock_guard<std::mutex> lock(state.mutex);
  
  // Check if we have a client for this host/connection pool
  auto it = state.http_clients.find(pool_config);
  if (it == state.http_clients.end()) {
    // Create new client for this connection pool configuration
    auto newClient = std::unique_ptr<Http::MCurlHttpClient>(new Http::MCurlHttpClient());
    // TODO: Apply connection pool settings to the client
    newClient->start();
    it = state.http_clients.insert({pool_config, std::move(newClient)}).first;
  }
  
  // Modify the request URL if needed before using the client
  auto &header = request.getHeader();
  auto &url = request.getUrl();
  if (url.getHost().empty()) {
    auto it_header = header.find("host");
    if (it_header != header.end()) {
      url.setHost(it_header->second);
    }
  }
  if (url.getScheme().empty()) {
    url.setScheme("https");
  }
  
  // Use the appropriate client for this host/connection pool
  // Pass request-level configuration to be applied per-request
  Darabonba::Json request_runtime = Darabonba::Json::object();
  request_runtime["connectTimeout"] = request_config.connect_timeout_ms;
  request_runtime["readTimeout"] = request_config.read_timeout_ms;
  request_runtime["ignoreSSL"] = request_config.ignore_ssl;
  request_runtime["httpProxy"] = request_config.http_proxy;
  request_runtime["httpsProxy"] = request_config.https_proxy;
  request_runtime["noProxy"] = request_config.no_proxy;
  
  return it->second->makeRequest(request, request_runtime);
}

// Function to clear specific client configuration
void Core::ClearHttpClient(const ConnectionPoolConfig& config) {
  auto& state = GetSDKState();
  std::lock_guard<std::mutex> lock(state.mutex);
  
  auto it = state.http_clients.find(config);
  if (it != state.http_clients.end()) {
    state.http_clients.erase(it);
  }
}

// Function to clear all clients
void Core::ClearAllHttpClients() {
  auto& state = GetSDKState();
  std::lock_guard<std::mutex> lock(state.mutex);
  
  state.http_clients.clear();
}

// Function to get client count
size_t Core::GetHttpClientCount() {
  auto& state = GetSDKState();
  std::lock_guard<std::mutex> lock(state.mutex);
  
  return state.http_clients.size();
}

bool allowRetry(const RetryOptions &options, const RetryPolicyContext &ctx) {
  if (ctx.getRetriesAttempted() == 0) {
    return true;
  }

  if (!options.isRetryable()) {
    return false;
  }

  int retriesAttempted = ctx.getRetriesAttempted();
  shared_ptr<DaraException> ex = ctx.getException();

  auto noRetryConditions = options.getNoRetryCondition();
  for (const auto &condition : noRetryConditions) {
    if (std::find(condition.getException().begin(),
                  condition.getException().end(),
                  ex->getName()) != condition.getException().end() ||
        std::find(condition.getErrorCode().begin(),
                  condition.getErrorCode().end(),
                  ex->getCode()) != condition.getErrorCode().end()) {
      return false;
    }
  }

  auto retryConditions = options.getRetryCondition();
  for (const auto &condition : retryConditions) {
    if (std::find(condition.getException().begin(),
                  condition.getException().end(),
                  ex->getName()) == condition.getException().end() &&
        std::find(condition.getErrorCode().begin(),
                  condition.getErrorCode().end(),
                  ex->getCode()) == condition.getErrorCode().end()) {
      continue;
    }

    if (retriesAttempted >= condition.getMaxAttempts()) {
      return false;
    }
    return true;
  }

  return false;
}

int getBackoffTime(const RetryOptions &options, const RetryPolicyContext &ctx) {
  shared_ptr<DaraException> ex = ctx.getException();
  auto conditions = options.getRetryCondition();

  for (const auto &condition : conditions) {
    if (std::find(condition.getException().begin(),
                  condition.getException().end(),
                  ex->getName()) == condition.getException().end() &&
        std::find(condition.getErrorCode().begin(),
                  condition.getErrorCode().end(),
                  ex->getCode()) == condition.getErrorCode().end()) {
      continue;
    }

    int maxDelay = (condition.getMaxDelay() > 0) ? condition.getMaxDelay()
                                                 : MAX_DELAY_TIME;
    int retryAfter = ex->getRetryAfter();

    if (retryAfter > 0) {
      return (std::min)(retryAfter, maxDelay);
    }

    if (!condition.getBackoff()) {
      return MIN_DELAY_TIME;
    }

    BackoffPolicy *strategy = condition.getBackoff().get();
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

Json defaultVal(const Json &a, const Json &b) {
  if (a.is_null()) {
    return b;
  }
  switch (a.type()) {
  case Json::value_t::string:
    return a.get<std::string>().empty() ? b : a;
  case Json::value_t::boolean:
    return !a.get<bool>() ? b : a;
  case Json::value_t::number_integer:
    return a.get<int>() == 0 ? b : a;
  case Json::value_t::number_unsigned:
    return a.get<unsigned int>() == 0 ? b : a;
  case Json::value_t::number_float:
    return a.get<float>() == 0.0 ? b : a;
  case Json::value_t::binary:
    return a.get_binary().empty() ? b : a;
  case Json::value_t::object:
  case Json::value_t::array:
    return a.empty() ? b : a;
  default:
    return b; // All other non-valid or discarded cases
  }
}

} // namespace Darabonba