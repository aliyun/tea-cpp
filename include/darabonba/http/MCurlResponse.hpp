#ifndef DARABONBA_HTTP_MCULRESPONSE_H_
#define DARABONBA_HTTP_MCULRESPONSE_H_
#include <atomic>
#include <condition_variable>
#include <curl/curl.h>
#include <darabonba/Stream.hpp>
#include <darabonba/buffer/RingBuffer.hpp>
#include <darabonba/http/Header.hpp>
#include <darabonba/http/ResponseBase.hpp>
#include <darabonba/lock/SpinLock.hpp>
#include <memory>
#include <mutex>

namespace Darabonba {
namespace Http {

class MCurlHttpClient;

class MCurlResponseBody : public IOStream {

  friend class MCurlHttpClient;

public:
  MCurlResponseBody &operator=(const MCurlResponseBody &) = delete;

  MCurlResponseBody &operator=(MCurlResponseBody &&) = delete;

  virtual ~MCurlResponseBody() {}

  size_t getMaxSize() const { return maxSize_; }
  // size_t setMaxSize(size_t maxSize) { maxSize_ = maxSize; }

  size_t getReadableSize() const { return readableSize_; }

  /**
   * @note This method is thread safe.
   * @note The calling thread will be blocked if there is no data in the buffer.
   */
  virtual size_t read(char *buffer, size_t expectSize) override;

  virtual bool isFinished() const override {
    if (done_ && readableSize_ == 0) {
      return true;
    }
    return false;
  };

  /**
   * @brief This method is thread safe.
   */
  virtual size_t write(char *buffer, size_t expectSize) override;

  void waitForDone();

  /**
   * @brief Indicate whether http responses have been accepted
   */
  bool getDone() const { return done_; }

  /**
   * @brief Indicate whether http response body is ready to receive.
   */
  bool getReady() const { return ready_; }

  /**
   * @brief 发送一个请求，让 client 继续读取数据并放到流中。
   * 类似于 stringstream
   */
  bool fetch();

protected:
  enum { MAX_SIZE = 6000 };

  /**
   * @brief The maximum size of data stored in the body
   */
  std::atomic<size_t> maxSize_ = {MAX_SIZE};
  std::atomic<bool> done_ = {false};
  std::atomic<bool> ready_ = {false};

  // 用于等待整个数据接受完毕
  mutable std::mutex doneMutex_;
  std::condition_variable doneCV_;

  std::atomic<size_t> readableSize_ = {0};

  mutable std::mutex streamMutex_;
  std::condition_variable streamCV_;

  Lock::SpinLock bufferlock_;
  Buffer::RingBuffer buffer_;

  MCurlHttpClient *client_ = nullptr;
  CURL *easyHandle_ = nullptr;
};

class MCurlResponse : public ResponseBase {
public:
  MCurlResponse() = default;
  MCurlResponse(const MCurlResponse &other) = default;
  MCurlResponse(MCurlResponse &&) = default;
  virtual ~MCurlResponse() = default;

  MCurlResponse &setHeader(const Header &header) {
    header_ = header;
    return *this;
  }
  MCurlResponse &setHeader(Header &&header) {
    header_ = std::move(header);
    return *this;
  }

  MCurlResponse &operator=(const MCurlResponse &) = default;

  MCurlResponse &operator=(MCurlResponse &&) = default;

  virtual std::shared_ptr<MCurlResponseBody> getBody() const { return body_; };

  virtual void setBody(std::shared_ptr<MCurlResponseBody> body) {
    body_ = body;
  }

protected:
  std::shared_ptr<MCurlResponseBody> body_ = nullptr;
};

} // namespace Http
} // namespace Darabonba

namespace nlohmann {
template <>
struct adl_serializer<std::shared_ptr<Darabonba::Http::MCurlResponseBody>> {
  static void
  to_json(json &j,
          const std::shared_ptr<Darabonba::Http::MCurlResponseBody> &body) {
    if (body) {
      j = reinterpret_cast<uintptr_t>(body.get());
    } else {
      j = nullptr;
    }
  }

  static std::shared_ptr<Darabonba::Http::MCurlResponseBody>
  from_json(const json &j) {
    if (!j.is_null() && j.is_number_unsigned()) {
      Darabonba::Http::MCurlResponseBody *ptr =
          reinterpret_cast<Darabonba::Http::MCurlResponseBody *>(
              j.get<uintptr_t>());
      return std::shared_ptr<Darabonba::Http::MCurlResponseBody>(
          ptr, [](Darabonba::Http::MCurlResponseBody *) {
          }); // 不删除，因为所有权不明确
    }
    return nullptr;
  }
};

template <>
struct adl_serializer<std::shared_ptr<Darabonba::Http::MCurlResponse>> {
  static void
  to_json(json &j,
          const std::shared_ptr<Darabonba::Http::MCurlResponse> &body) {
    if (body) {
      j = reinterpret_cast<uintptr_t>(body.get());
    } else {
      j = nullptr;
    }
  }

  static std::shared_ptr<Darabonba::Http::MCurlResponse>
  from_json(const json &j) {
    if (!j.is_null() && j.is_number_unsigned()) {
      Darabonba::Http::MCurlResponse *ptr =
          reinterpret_cast<Darabonba::Http::MCurlResponse *>(
              j.get<uintptr_t>());
      return std::make_shared<Darabonba::Http::MCurlResponse>(*ptr);
    }
    return nullptr;
  }
};
} // namespace nlohmann
#endif