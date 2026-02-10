#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/MCurlResponse.hpp>
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

size_t MCurlResponseBody::read(char *buffer, size_t expectSize) {
  if (done_ && readableSize_ == 0)
    return 0;
  size_t realSize = 0;
  do {
    realSize = (std::min)({expectSize, readableSize_.load(), maxSize_.load()});
    if (realSize != 0) {
      {
        std::lock_guard<Lock::SpinLock> lock(bufferlock_);
        realSize = buffer_.read(buffer, realSize);
      }
      readableSize_ -= realSize;
      return realSize;
    }

    if (done_) {
      client_ = nullptr;
      easyHandle_ = nullptr;
      return 0;
    }
    // blocking wait
    fetch();
    std::unique_lock<std::mutex> lock(streamMutex_);
    streamCV_.wait(
        lock, [this]() -> bool { return done_.load() || readableSize_ > 0; });
  } while (realSize == 0);

  return realSize;
}

bool MCurlResponseBody::fetch() {
  if (!easyHandle_ || !client_)
    return false;
  client_->addContinueReadingHandle(easyHandle_);
  return true;
}

size_t MCurlResponseBody::write(char *buffer, size_t expectSize) {
  {
    std::lock_guard<Lock::SpinLock> lock(bufferlock_);
    buffer_.write(buffer, expectSize);
  }
  readableSize_ += expectSize;
  return expectSize;
}

} // namespace Http
} // namespace Darabonba