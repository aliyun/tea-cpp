#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <cstdint>
#include <darabonba/Runtime.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/policy/Retry.hpp>
#include <future>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <functional>

namespace Darabonba {
namespace Policy {
class RetryOptions;
class RetryPolicyContext;
} // namespace Policy

namespace Http {
class MCurlHttpClient;
} // namespace Http


/**
 * @brief Connection pool configuration (host-level)
 * Applied to all requests for the same host
 */
struct ConnectionPoolConfig {
  std::string host;
  size_t max_connections = 128;        // Maximum total connections (CURLMOPT_MAX_TOTAL_CONNECTIONS)
  int64_t connection_idle_timeout = 30L;  // Connection idle timeout (seconds), curl default is 118s
  bool pipelining = false;           // Enable HTTP pipelining
  size_t max_host_connections = 128;   // Max connections per host (CURLMOPT_MAX_HOST_CONNECTIONS)
  bool keep_alive = true;            // Enable TCP keep-alive

  // Compare operator for map keys
  bool operator<(const ConnectionPoolConfig& other) const {
    if (host != other.host) return host < other.host;
    if (max_connections != other.max_connections) return max_connections < other.max_connections;
    if (connection_idle_timeout != other.connection_idle_timeout) return connection_idle_timeout < other.connection_idle_timeout;
    if (pipelining != other.pipelining) return pipelining < other.pipelining;
    if (keep_alive != other.keep_alive) return keep_alive < other.keep_alive;
    return max_host_connections < other.max_host_connections;
  }
};

/**
 * @brief Request-level configuration
 * Applied per individual request
 */
struct RequestConfig {
  int64_t connect_timeout_ms = 5000L;   // Per-request connection timeout
  int64_t read_timeout_ms = 10000L;         // Per-request read timeout
  bool ignore_ssl = false;           // Per-request SSL verification
  std::string http_proxy;            // Per-request proxy
  std::string https_proxy;           // Per-request HTTPS proxy
  std::string no_proxy;              // Per-request no-proxy list
  std::string credential;            // Per-request credentials
};

class Core {
public:
  /**
   * @note This function will the Http::Request
   */
  static std::future<std::shared_ptr<Http::MCurlResponse>>
  doAction(Http::Request &request, const Darabonba::Json &runtime = {});

  static std::string uuid();
  
  /**
   * @brief Clear a specific connection pool configuration
   * @param config The configuration to clear
   */
  static void ClearHttpClient(const ConnectionPoolConfig &config);
  
  /**
   * @brief Clear all HttpClients
   */
  static void ClearAllHttpClients();
  
  /**
   * @brief Get the count of active HttpClients
   * @return Number of active HttpClients
   */
  static size_t GetHttpClientCount();
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

template <typename T> inline bool isNull(const T &value) {
  (void)value;
  return false; // 默认情况下非指针类型不是nullptr
}

template <typename T> inline bool isNull(T *const &ptr) {
  return ptr == nullptr;
}

// 对于特定类型的特化，如果该类型有意义
template <> inline bool isNull<std::nullptr_t>(const std::nullptr_t &) {
  return true;
}

// std::string 的特化：空字符串视为 null
template <> inline bool isNull<std::string>(const std::string &str) {
  return str.empty();
}

// std::shared_ptr 的特化：空指针视为 null
template <typename T> inline bool isNull(const std::shared_ptr<T> &ptr) {
  return ptr == nullptr;
}

// 容器类型特化：空容器视为 null
// std::map
template <typename K, typename V> inline bool isNull(const std::map<K, V> &m) {
  return m.empty();
}

// std::vector
template <typename T> inline bool isNull(const std::vector<T> &v) {
  return v.empty();
}

/**
 * @brief Safe map access - check if key exists and value is not null
 * @param m The map to search
 * @param key The key to look for
 * @return true if key exists and value is not null (not empty string for string values)
 */
template <typename K, typename V> inline bool hasValue(const std::map<K, V> &m, const K &key) {
  auto it = m.find(key);
  return it != m.end() && !isNull(it->second);
}

/**
 * @brief Check if map key is null (key not exists or value is null)
 * @param m The map to search
 * @param key The key to look for
 * @return true if key not exists OR value is null (consistent with isNull(value) semantics)
 * 
 * Usage: isNull(map, key) instead of isNull(map[key]) which fails on const map
 */
template <typename K, typename V> inline bool isNull(const std::map<K, V> &m, const K &key) {
  auto it = m.find(key);
  return it == m.end() || isNull(it->second);
}

/**
 * @brief Safe map get - returns value or default if key not found
 * @param m The map to search
 * @param key The key to look for
 * @param defaultValue The value to return if key not found
 * @return The value if key exists, otherwise defaultValue
 */
template <typename K, typename V> inline V getOrDefault(const std::map<K, V> &m, const K &key, const V &defaultValue) {
  auto it = m.find(key);
  return (it != m.end()) ? it->second : defaultValue;
}

Json defaultVal(const Json &a, const Json &b);

bool allowRetry(const Policy::RetryOptions &options,
                const Policy::RetryPolicyContext &ctx);
int getBackoffTime(const Policy::RetryOptions &options,
                   const Policy::RetryPolicyContext &ctx);

/**
 * @brief Sleep for the specified number of milliseconds
 * @param milliseconds The number of milliseconds to sleep
 */
void sleep(int milliseconds);

} // namespace Darabonba

#endif
