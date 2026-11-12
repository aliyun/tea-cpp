#include <darabonba/Exception.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <chrono>
#include <mutex>
#include <string>

using std::string;

namespace Darabonba {
namespace Http {

void MCurlResponseBody::waitForDone() {
  if (done_)
    return;
  if (maxSize_ != (std::numeric_limits<size_t>::max)()) {
    maxSize_ = (std::numeric_limits<size_t>::max)();
    client_->addContinueReadingHandle(easyHandle_);
  }
  std::unique_lock<std::mutex> lock(doneMutex_);
  doneCV_.wait(lock, [this]() -> bool { return done_.load(); });
  client_ = nullptr;
  easyHandle_ = nullptr;
}

void MCurlResponseBody::fail(const std::string &message) {
  {
    std::lock_guard<std::mutex> lock(errorMutex_);
    if (errorMessage_.empty()) {
      errorMessage_ = message;
    }
  }
  failed_.store(true, std::memory_order_release);
  finish();
}

void MCurlResponseBody::finish() {
  {
    // Both wait predicates observe done_; publish it under their locks.
    std::lock_guard<std::mutex> doneLock(doneMutex_);
    std::lock_guard<std::mutex> streamLock(streamMutex_);
    done_.store(true);
  }
  streamCV_.notify_all();
  doneCV_.notify_all();
}

size_t MCurlResponseBody::read(char *buffer, size_t expectSize) {
  auto raiseIfFailed = [this]() {
    if (!failed_.load(std::memory_order_acquire)) {
      return;
    }
    std::string message;
    {
      std::lock_guard<std::mutex> lock(errorMutex_);
      message = errorMessage_;
    }
    client_ = nullptr;
    easyHandle_ = nullptr;
    throw Darabonba::ResponseException(
        "NetworkError",
        message.empty() ? "response body failed" : message);
  };

  for (;;) {
    if (done_ && readableSize_ == 0) {
      raiseIfFailed();
      client_ = nullptr;
      easyHandle_ = nullptr;
      return 0;
    }

    size_t realSize =
        (std::min)({expectSize, readableSize_.load(), maxSize_.load()});
    if (realSize != 0) {
      {
        std::lock_guard<Lock::SpinLock> lock(bufferlock_);
        realSize = buffer_.read(buffer, realSize);
      }
      if (realSize == 0) {
        readableSize_ = 0;
        continue;
      }
      readableSize_ -= realSize;
      return realSize;
    }

    if (done_) {
      raiseIfFailed();
      client_ = nullptr;
      easyHandle_ = nullptr;
      return 0;
    }

    fetch();
    bool ready = false;
    {
      std::unique_lock<std::mutex> lock(streamMutex_);
      ready = streamCV_.wait_for(lock, readWait_, [this]() -> bool {
        return done_.load() || failed_.load() || readableSize_.load() > 0;
      });
    }
    if (!ready) {
      raiseIfFailed();
      if (done_ && readableSize_ == 0) {
        client_ = nullptr;
        easyHandle_ = nullptr;
        return 0;
      }
      throw Darabonba::ResponseException(
          "Timeout", "response body stalled before the transfer finished");
    }
  }
}

bool MCurlResponseBody::fetch() {
  if (!easyHandle_ || !client_)
    return false;
  client_->addContinueReadingHandle(easyHandle_);
  return true;
}

size_t MCurlResponseBody::write(char *buffer, size_t expectSize) {
  {
    // Publish readable data under the mutex used by the wait predicate.
    std::lock_guard<std::mutex> streamLock(streamMutex_);
    std::lock_guard<Lock::SpinLock> bufferLock(bufferlock_);
    buffer_.write(buffer, expectSize);
    readableSize_ += expectSize;
  }
  streamCV_.notify_all();  // wake all waiters (multi-reader / stress tests)
  return expectSize;
}

} // namespace Http
} // namespace Darabonba
