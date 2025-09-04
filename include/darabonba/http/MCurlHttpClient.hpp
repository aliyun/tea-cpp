#include <atomic>
#include <condition_variable>
#include <darabonba/Runtime.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/lock/SpinLock.hpp>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace Darabonba {
namespace Http {

class MCurlResponse;
class MCurlResponseBody;

class MCurlHttpClient {
  friend class MCurlResponseBody;

public:
  MCurlHttpClient() : mCurl_(curl_multi_init()) {}
  ~MCurlHttpClient() {
    stop();
    performThread_.join();
    clearQueue();
    curl_multi_cleanup(mCurl_);
    mCurl_ = nullptr;
  }

  std::future<std::shared_ptr<MCurlResponse>>
  makeRequest(const Request &request, const Darabonba::Json &options = {}) ;

  /**
   * @brief Start a background thread to handle network IO
   */
  bool start();

  /**
   * @brief Stop the backgroud thread.
   * @note All existing connections will be removed.
   */
  bool stop();

protected:
  enum { WAIT_MS = 2000 };

  struct CurlStorage {
  public:
    CURL *easyHandle;
    // request header
    curl_slist *reqHeader;
    // request body
    std::shared_ptr<IStream> reqBody;
    // http response
    std::shared_ptr<MCurlResponse> resp;

    std::unique_ptr<std::promise<std::shared_ptr<MCurlResponse>>> promise;
  };

  void perform();

  bool addContinueReadingHandle(CURL *easyHandle);

  static size_t recvBody(char *buffer, size_t size, size_t nmemb,
                         void *userdata);

  static bool setResponseReady(CurlStorage *curlStorage);

  void clearQueue();

  std::thread performThread_;

  std::atomic<bool> running_ = {false};

  Lock::SpinLock reqLock_;
  std::list<std::unique_ptr<CurlStorage>> reqQueue_;
  std::atomic<size_t> reqQueueSize_ = {0};

  Lock::SpinLock continueReadingLock_;
  std::list<CURL *> continueReadingQueue_;
  std::atomic<size_t> continueReadingQueueSize_ = {0};

  CURLM *mCurl_ = nullptr;

  /**
   * @note runningCurl_ can be accessed in performThread_
   */
  std::unordered_map<CURL *, std::unique_ptr<CurlStorage>> runningCurl_;

  std::mutex stopMutex_;
  std::condition_variable stopCV_;
  std::atomic<bool> stop_ = {false};
};

} // namespace Http
} // namespace Darabonba