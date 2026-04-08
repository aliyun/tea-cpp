#include <darabonba/Exception.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <mutex>

namespace Darabonba {
namespace Http {

std::future<std::shared_ptr<MCurlResponse>>
MCurlHttpClient::makeRequest(const Request &request,
                             const Darabonba::Json &options) {
  if (!running_ || !mCurl_) {
    std::promise<std::shared_ptr<MCurlResponse>> promise;
    promise.set_value(nullptr);
    return promise.get_future();
  }
  auto easyHandle = curl_easy_init();
  if (!easyHandle) {
    std::promise<std::shared_ptr<MCurlResponse>> promise;
    promise.set_value(nullptr);
    return promise.get_future();
  }

  // Atomically load config to avoid race condition with setConnectionPoolConfig
  // This ensures we use a consistent snapshot of the configuration
  auto configPtr = std::atomic_load(&poolConfig_);
  ConnectionPoolConfig config = configPtr ? *configPtr : ConnectionPoolConfig();

  // Apply connection pool settings to easy handle
  curl_easy_setopt(easyHandle, CURLOPT_MAXCONNECTS,
                   static_cast<long>(config.max_connections));

  // Set maximum connection age (curl 7.65.0+)
  // This limits how long a connection can be reused
#if LIBCURL_VERSION_NUM >= 0x074100
  curl_easy_setopt(easyHandle, CURLOPT_MAXAGE_CONN,
                   config.connection_idle_timeout);
#endif

  // Apply keep-alive settings
  if (config.keep_alive) {
    curl_easy_setopt(easyHandle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(easyHandle, CURLOPT_TCP_KEEPIDLE, 60L);
    curl_easy_setopt(easyHandle, CURLOPT_TCP_KEEPINTVL, 60L);
    // Enable HTTP keep-alive
    curl_easy_setopt(easyHandle, CURLOPT_FORBID_REUSE, 0L);
  } else {
    // Disable connection reuse when keepAlive is false
    curl_easy_setopt(easyHandle, CURLOPT_FORBID_REUSE, 1L);
  }

  if (!options.is_null()) {
    // process the runtime options
    // ssl
    if (options.value("ignoreSSL", false)) {
      curl_easy_setopt(easyHandle, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(easyHandle, CURLOPT_SSL_VERIFYHOST, 0L);
    } else {
      curl_easy_setopt(easyHandle, CURLOPT_SSL_VERIFYPEER, 1L);
      curl_easy_setopt(easyHandle, CURLOPT_SSL_VERIFYHOST,
                       2L); // 2L for full verification
    }
    // timeout
    auto connectTimeout = options.value("connectTimeout", 5000L);
    if (connectTimeout > 0) {
      curl_easy_setopt(easyHandle, CURLOPT_CONNECTTIMEOUT_MS, connectTimeout);
    }
    long readTimeout = options.value("readTimeout", 10000L);
    if (readTimeout > 0) {
      curl_easy_setopt(easyHandle, CURLOPT_TIMEOUT_MS, readTimeout);
    }
    // set proxy
    // TODO: sock5
    std::string httpProxy = options.value("httpProxy", "");
    if (!httpProxy.empty()) {
      curl_easy_setopt(easyHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
      Curl::setCurlProxy(easyHandle, httpProxy);
    }
    std::string httpsProxy = options.value("httpsProxy", "");
    if (!httpsProxy.empty()) {
      curl_easy_setopt(easyHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
      Curl::setCurlProxy(easyHandle, httpsProxy);
    }
    std::string noProxy = options.value("noProxy", "");
    if (!noProxy.empty()) {
      curl_easy_setopt(easyHandle, CURLOPT_NOPROXY, noProxy.c_str());
    }
  }

  if (nullptr != getenv("DEBUG")) {
    curl_easy_setopt(easyHandle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(easyHandle, CURLOPT_DEBUGFUNCTION, Curl::debugFunction);
  }

  // set request method
  curl_easy_setopt(easyHandle, CURLOPT_CUSTOMREQUEST,
                   request.getMethod().c_str());
  // set request url
  curl_easy_setopt(easyHandle, CURLOPT_URL,
                   static_cast<std::string>(request.getUrl()).c_str());

  // set the storage of request body
  Curl::setCurlRequestBody(easyHandle, request.getBody());

  // init header
  auto curlStorage = std::unique_ptr<CurlStorage>(new CurlStorage{
      easyHandle, Curl::setCurlHeader(easyHandle, request.getHeader()),
      request.getBody(), std::make_shared<MCurlResponse>(),
      std::unique_ptr<std::promise<std::shared_ptr<MCurlResponse>>>(
          new std::promise<std::shared_ptr<MCurlResponse>>())});

  // set response body
  // TODO:: custom response body by user
  auto body = std::make_shared<MCurlResponseBody>();
  body->easyHandle_ = easyHandle;
  body->client_ = this;
  auto &resp = curlStorage->resp;
  resp->setBody(body);

  // set the storage of response header
  curl_easy_setopt(easyHandle, CURLOPT_HEADERDATA, &(resp->getHeaders()));
  // set how to receive response header
  curl_easy_setopt(easyHandle, CURLOPT_HEADERFUNCTION, Curl::writeHeader);
  // set the storage of response body
  curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, curlStorage.get());
  // set how to receive response body
  curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, recvBody);

  auto ret = curlStorage->promise->get_future();
  {
    std::lock_guard<Lock::SpinLock> guard(reqLock_);
    reqQueue_.emplace_back(std::move(curlStorage));
    ++reqQueueSize_;
  }
  // wake the curl_multi_poll
  curl_multi_wakeup(mCurl_);
  return ret;
}

void MCurlHttpClient::perform() {
  while (running_) {
    // Apply config update if needed (before curl_multi_perform)
    if (configNeedsUpdate_.load()) {
      applyConnectionPoolSettings();
      configNeedsUpdate_.store(false);
    }

    if (reqQueueSize_) {
      decltype(reqQueue_) reqQueue;
      {
        std::lock_guard<Lock::SpinLock> guard(reqLock_);
        reqQueue = std::move(reqQueue_);
        reqQueueSize_ = 0;
      }
      while (!reqQueue.empty()) {
        auto storage = std::move(reqQueue.front());
        reqQueue.pop_front();
        // add the easy_curl to multi_curl
        curl_multi_add_handle(mCurl_, storage->easyHandle);
        runningCurl_[storage->easyHandle] = std::move(storage);
      }
    }
    if (continueReadingQueueSize_) {
      decltype(continueReadingQueue_) continueReadingQueue;
      {
        std::lock_guard<Lock::SpinLock> guard(continueReadingLock_);
        continueReadingQueue = std::move(continueReadingQueue_);
        continueReadingQueueSize_ = 0;
      }
      for (auto easyHandle : continueReadingQueue) {
        if (runningCurl_.count(easyHandle)) {
          // set continue reading
          curl_easy_pause(easyHandle, CURLPAUSE_CONT);
        }
      }
    }
    auto code = curl_multi_poll(mCurl_, nullptr, 0, WAIT_MS, nullptr);
    if (code != CURLM_OK) {
      // TODO:: handle error
      continue;
    }
    int running_handles;
    curl_multi_perform(mCurl_, &running_handles);

    int msgs_in_queue = 0;
    CURLMsg *msg = nullptr;

    while ((msg = curl_multi_info_read(mCurl_, &msgs_in_queue)) != nullptr) {
      if (msg->msg == CURLMSG_DONE) {
        auto easyHandle = msg->easy_handle;

        // Safe lookup - don't use operator[] which creates nullptr entries
        auto it = runningCurl_.find(easyHandle);
        if (it == runningCurl_.end()) {
          // Handle not found - clean up and skip
          curl_multi_remove_handle(mCurl_, easyHandle);
          curl_easy_cleanup(easyHandle);
          continue;
        }
        auto curlStorage = std::move(it->second);
        runningCurl_.erase(it);

        // Null check - but we still need to set done_ and notify even if promise is null
        if (!curlStorage) {
          curl_multi_remove_handle(mCurl_, easyHandle);
          curl_easy_cleanup(easyHandle);
          continue;
        }

        CURLcode curlResult = msg->data.result;
        if (curlResult != CURLE_OK) {
          if (curlStorage->promise) {
            curlStorage->promise->set_exception(
                std::make_exception_ptr(Darabonba::ResponseException(
                    "NetworkError",
                    std::string("Curl error: ") + curl_easy_strerror(curlResult))));
          }
        } else {
          auto body = dynamic_cast<MCurlResponseBody *>(
              curlStorage->resp->getBody().get());
          if (body) {
            if (!body->getReady()) {
              setResponseReady(curlStorage.get());
            }
            body->done_ = true;
            body->streamCV_.notify_one();
            body->doneCV_.notify_one();
          } else {
            // This should never happen - log as error if DEBUG is enabled
            if (nullptr != getenv("DEBUG")) {
              std::cerr << "[ERROR perform] Response body is null!" << std::endl;
            }
          }
        }

        if (curlStorage->reqBody) {
          curlStorage->reqBody.reset();
        }
        curl_multi_remove_handle(mCurl_, easyHandle);
        curl_easy_cleanup(easyHandle);
      } else {
        // TODO: handle other case
      }
    }
  }
  // close the existing network connections.
  for (auto &p : runningCurl_) {
    if (p.second) {
      curl_slist_free_all(p.second->reqHeader);
    }
    curl_multi_remove_handle(mCurl_, p.first);
    curl_easy_cleanup(p.first);
  }
  runningCurl_.clear();
  stop_ = true;
  stopCV_.notify_all();
}

bool MCurlHttpClient::start() {
  if (!mCurl_ || running_)
    return false;
  // Apply connection pool settings before starting
  applyConnectionPoolSettings();
  running_ = true;
  performThread_ = std::thread(std::bind(&MCurlHttpClient::perform, this));
  return true;
}

void MCurlHttpClient::applyConnectionPoolSettings() {
  if (!mCurl_)
    return;

  // Atomically load config for thread-safe access
  auto configPtr = std::atomic_load(&poolConfig_);
  if (!configPtr)
    return;

  const ConnectionPoolConfig& config = *configPtr;

  // Set maximum total connections in the multi handle
  curl_multi_setopt(mCurl_, CURLMOPT_MAX_TOTAL_CONNECTIONS,
                    static_cast<long>(config.max_connections));

  // Set maximum connections per host
  curl_multi_setopt(mCurl_, CURLMOPT_MAX_HOST_CONNECTIONS,
                    static_cast<long>(config.max_host_connections));

  // Enable/disable pipelining
  curl_multi_setopt(mCurl_, CURLMOPT_PIPELINING,
                    config.pipelining ? 1L : 0L);
}

bool MCurlHttpClient::stop() {
  if (!running_)
    return false;
  running_ = false;
  curl_multi_wakeup(mCurl_);
  std::unique_lock<std::mutex> lock(stopMutex_);
#ifdef _WIN32
  // Windows: Use timeout to prevent deadlock during DLL unload
  stopCV_.wait_for(lock, std::chrono::seconds(5), [this]() { return stop_.load(); });
#else
  stopCV_.wait(lock, [this]() { return stop_.load(); });
#endif
  return true;
}

void MCurlHttpClient::clearQueue() {
  std::lock_guard<Lock::SpinLock> guard1(reqLock_);
  std::lock_guard<Lock::SpinLock> guard2(continueReadingLock_);
  while (!reqQueue_.empty()) {
    auto storage = std::move(reqQueue_.front());
    reqQueue_.pop_front();
    if (storage) {
      curl_slist_free_all(storage->reqHeader);
      curl_easy_cleanup(storage->easyHandle);
    }
  }
  reqQueue_.clear();
  continueReadingQueue_.clear();
}

bool MCurlHttpClient::addContinueReadingHandle(CURL *easyHandle) {
  if (!running_ || !mCurl_ || !easyHandle)
    return false;
  {
    std::lock_guard<Lock::SpinLock> guard(continueReadingLock_);
    continueReadingQueue_.emplace_back(easyHandle);
    ++continueReadingQueueSize_;
  }
  curl_multi_wakeup(mCurl_);
  return true;
}

bool MCurlHttpClient::setResponseReady(CurlStorage *curlStorage) {
  // Null pointer guard
  if (!curlStorage) {
    return false;
  }
  // clear the request header
  curl_slist_free_all(curlStorage->reqHeader);
  curlStorage->reqHeader = nullptr;
  // clear the request body
  curlStorage->reqBody = nullptr;

  // set the status code
  long responseCode;
  curl_easy_getinfo(curlStorage->easyHandle, CURLINFO_RESPONSE_CODE,
                    &responseCode);
  curlStorage->resp->setStatusCode(responseCode);
  // set ready
  curlStorage->resp->getBody()->ready_ = true;
  curlStorage->promise->set_value(curlStorage->resp);
  curlStorage->promise = nullptr;
  return true;
}

size_t MCurlHttpClient::recvBody(char *buffer, size_t size, size_t nmemb,
                                 void *userdata) {
  auto curlStorage = static_cast<CurlStorage *>(userdata);
  // Null pointer guard
  if (!curlStorage || !curlStorage->resp) {
    return 0;
  }
  auto body = curlStorage->resp->getBody();
  if (!body)
    return 0;
  if (body->getReadableSize() >= body->getMaxSize()) {
    return CURL_WRITEFUNC_PAUSE;
  }
  auto expectSize = size * nmemb;
  auto realSize = body->write(buffer, expectSize);
  if (!body->getReady()) {
    setResponseReady(curlStorage);
  }

  return realSize;
}

} // namespace Http
} // namespace Darabonba