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
    long readTimeout = options.value("readTimeout", 0L);
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
        auto curlStorage = std::move(runningCurl_[easyHandle]);
        runningCurl_.erase(easyHandle);

        auto body = dynamic_cast<MCurlResponseBody *>(
            curlStorage->resp->getBody().get());
        if (body) {
          if (!body->getReady()) {
            setResponseReady(curlStorage.get());
          }
          // response body has been fully received
          body->done_ = true;
          body->doneCV_.notify_one();
        }

        if (curlStorage->reqBody) {
          curlStorage->reqBody.reset(); // 使用智能指针的 reset 方法释放资源
        }
        // remove the easy hanle
        curl_multi_remove_handle(mCurl_, easyHandle);
        curl_easy_cleanup(easyHandle);
      } else {
        // TODO: handle other case
      }
    }
  }
  // close the existing network connections.
  for (auto &p : runningCurl_) {
    curl_slist_free_all(p.second->reqHeader);
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
  running_ = true;
  performThread_ = std::thread(std::bind(&MCurlHttpClient::perform, this));
  return true;
}

bool MCurlHttpClient::stop() {
  if (!running_)
    return false;
  running_ = false;
  curl_multi_wakeup(mCurl_);
  std::unique_lock<std::mutex> lock(stopMutex_);
  stopCV_.wait(lock, [this]() { return stop_.load(); });
  return true;
}

void MCurlHttpClient::clearQueue() {
  std::lock_guard<Lock::SpinLock> guard1(reqLock_);
  std::lock_guard<Lock::SpinLock> guard2(continueReadingLock_);
  while (!reqQueue_.empty()) {
    auto storage = std::move(reqQueue_.front());
    reqQueue_.pop_front();
    curl_slist_free_all(storage->reqHeader);
    curl_easy_cleanup(storage->easyHandle);
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
  auto body = curlStorage->resp->getBody();
  if (!body)
    return 0;
  if (body->getReadableSize() >= body->getMaxSize()) {
    return CURL_WRITEFUNC_PAUSE;
  }
  auto expectSize = size * nmemb;
  auto realSize = body->write(buffer, expectSize);
  body->readableSize_ += realSize;
  body->streamCV_.notify_one();

  if (!body->getReady()) {
    setResponseReady(curlStorage);
  }

  return realSize;
}

} // namespace Http
} // namespace Darabonba