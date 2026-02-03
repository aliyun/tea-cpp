#ifndef DARABONBA_RETRY_HPP
#define DARABONBA_RETRY_HPP

#include <algorithm>
#include <cmath>
#include <darabonba/Exception.hpp>
#include <darabonba/Model.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/Request.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// 定义最大和最小延迟时间
const int MaxDelayTime = 120 * 1000;
const int MinDelayTime = 100;

namespace Darabonba {
namespace Policy {

class RetryPolicyContext; // 前向声明

// BackoffPolicy 基类
class BackoffPolicy {
public:
  // 友元函数实现 JSON 序列化
  friend void to_json(Darabonba::Json &j, const BackoffPolicy &obj) {
    // 此宏需要支持序列化 std::string，增加处理逻辑在使用时
    DARABONBA_TO_JSON(policy, policy_);
  }

  // 友元函数实现 JSON 反序列化
  friend void from_json(const Darabonba::Json &j, BackoffPolicy &obj) {
    DARABONBA_FROM_JSON(policy, policy_);
  }

  BackoffPolicy() = default;
  BackoffPolicy(const BackoffPolicy &) = default;
  BackoffPolicy(BackoffPolicy &&) = default;
  BackoffPolicy(const Darabonba::Json &obj) { from_json(obj, *this); }
  explicit BackoffPolicy(const std::map<std::string, std::string> &option)
      : policy_(option.at("policy")) {}
  BackoffPolicy &operator=(const BackoffPolicy &) = default;
  BackoffPolicy &operator=(BackoffPolicy &&) = default;

  virtual ~BackoffPolicy() = default;

  // Getter 和 Setter 方法
  const std::string &getPolicy() const { return policy_; }
  void setPolicy(const std::string &policy) { policy_ = policy; }

  // 纯虚函数，待其他具体子类扩展实现
  virtual int getDelayTime(const RetryPolicyContext &ctx) const = 0;

  static std::unique_ptr<BackoffPolicy>
  createBackoffPolicy(const std::map<std::string, std::string> &option);

protected:
  std::string policy_;
};

// 具体的 Backoff 策略
class FixedBackoffPolicy : public BackoffPolicy {
public:
  // 友元函数实现 JSON 序列化
  friend void to_json(Darabonba::Json &j, const FixedBackoffPolicy &obj) {
    // 序列化基类成员
    to_json(j, static_cast<const BackoffPolicy &>(obj));
    // 序列化派生类成员
    DARABONBA_TO_JSON(period, period_);
  }

  // 友元函数实现 JSON 反序列化
  friend void from_json(const Darabonba::Json &j, FixedBackoffPolicy &obj) {
    // 反序列化基类成员
    from_json(j, static_cast<BackoffPolicy &>(obj));
    // 反序列化派生类成员
    DARABONBA_FROM_JSON(period, period_);
  }

  FixedBackoffPolicy(const Darabonba::Json &obj) { from_json(obj, *this); }
  explicit FixedBackoffPolicy(const std::map<std::string, std::string> &option)
      : BackoffPolicy(option), period_(std::stoi(option.at("period"))) {}

  FixedBackoffPolicy(const FixedBackoffPolicy &) = default;
  FixedBackoffPolicy(FixedBackoffPolicy &&) = default;
  FixedBackoffPolicy &operator=(const FixedBackoffPolicy &) = default;
  FixedBackoffPolicy &operator=(FixedBackoffPolicy &&) = default;
  virtual ~FixedBackoffPolicy() = default;

  int getDelayTime(const RetryPolicyContext &ctx) const override;

  // Getter 和 Setter 方法
  int getPeriod() const { return period_; }
  void setPeriod(int period) { period_ = period; }

private:
  int period_;
};

class RandomBackoffPolicy : public BackoffPolicy {
public:
  // 友元函数实现 JSON 序列化
  friend void to_json(Darabonba::Json &j, const RandomBackoffPolicy &obj) {
    // 序列化基类成员
    to_json(j, static_cast<const BackoffPolicy &>(obj));
    // 序列化派生类成员
    DARABONBA_TO_JSON(period, period_);
    DARABONBA_TO_JSON(cap, cap_);
  }

  // 友元函数实现 JSON 反序列化
  friend void from_json(const Darabonba::Json &j, RandomBackoffPolicy &obj) {
    // 反序列化基类成员
    from_json(j, static_cast<BackoffPolicy &>(obj));
    // 反序列化派生类成员
    DARABONBA_FROM_JSON(period, period_);
    DARABONBA_FROM_JSON(cap, cap_);
  }

  RandomBackoffPolicy(const Darabonba::Json &obj) { from_json(obj, *this); }
  explicit RandomBackoffPolicy(const std::map<std::string, std::string> &option)
      : BackoffPolicy(option), period_(std::stoi(option.at("period"))),
        cap_(option.count("cap") ? std::stoi(option.at("cap")) : 20 * 1000) {}

  RandomBackoffPolicy(const RandomBackoffPolicy &) = default;
  RandomBackoffPolicy(RandomBackoffPolicy &&) = default;
  RandomBackoffPolicy &operator=(const RandomBackoffPolicy &) = default;
  RandomBackoffPolicy &operator=(RandomBackoffPolicy &&) = default;
  virtual ~RandomBackoffPolicy() = default;

  int getDelayTime(const RetryPolicyContext &ctx) const override;

  // Getter 和 Setter 方法
  int getPeriod() const { return period_; }
  void setPeriod(int period) { period_ = period; }

  int getCap() const { return cap_; }
  void setCap(int cap) { cap_ = cap; }

private:
  int period_;
  int cap_;
};

class ExponentialBackoffPolicy : public BackoffPolicy {
public:
  friend void to_json(Darabonba::Json &j, const ExponentialBackoffPolicy &obj) {
    to_json(j, static_cast<const BackoffPolicy &>(obj));
    DARABONBA_TO_JSON(period, period_);
    DARABONBA_TO_JSON(cap, cap_);
  }

  friend void from_json(const Darabonba::Json &j,
                        ExponentialBackoffPolicy &obj) {
    from_json(j, static_cast<BackoffPolicy &>(obj));
    DARABONBA_FROM_JSON(period, period_);
    DARABONBA_FROM_JSON(cap, cap_);
  }

  ExponentialBackoffPolicy(const Darabonba::Json &obj) {
    from_json(obj, *this);
  }
  explicit ExponentialBackoffPolicy(
      const std::map<std::string, std::string> &option)
      : BackoffPolicy(option), period_(std::stoi(option.at("period"))),
        cap_(option.count("cap") ? std::stoi(option.at("cap"))
                                 : 3 * 24 * 60 * 60 * 1000) {}

  ExponentialBackoffPolicy &
  operator=(const ExponentialBackoffPolicy &) = default;
  ExponentialBackoffPolicy &operator=(ExponentialBackoffPolicy &&) = default;
  virtual ~ExponentialBackoffPolicy() = default;

  int getDelayTime(const RetryPolicyContext &ctx) const override;

  int getPeriod() const { return period_; }
  void setPeriod(int period) { period_ = period; }

  int getCap() const { return cap_; }
  void setCap(int cap) { cap_ = cap; }

private:
  int period_;
  int cap_;
};

class EqualJitterBackoffPolicy : public BackoffPolicy {
public:
  friend void to_json(Darabonba::Json &j, const EqualJitterBackoffPolicy &obj) {
    to_json(j, static_cast<const BackoffPolicy &>(obj));
    DARABONBA_TO_JSON(period, period_);
    DARABONBA_TO_JSON(cap, cap_);
  }

  friend void from_json(const Darabonba::Json &j,
                        EqualJitterBackoffPolicy &obj) {
    from_json(j, static_cast<BackoffPolicy &>(obj));
    DARABONBA_FROM_JSON(period, period_);
    DARABONBA_FROM_JSON(cap, cap_);
  }

  EqualJitterBackoffPolicy(const Darabonba::Json &obj) {
    from_json(obj, *this);
  }
  explicit EqualJitterBackoffPolicy(
      const std::map<std::string, std::string> &option)
      : BackoffPolicy(option), period_(std::stoi(option.at("period"))),
        cap_(option.count("cap") ? std::stoi(option.at("cap"))
                                 : 3 * 24 * 60 * 60 * 1000) {}

  EqualJitterBackoffPolicy &
  operator=(const EqualJitterBackoffPolicy &) = default;
  EqualJitterBackoffPolicy &operator=(EqualJitterBackoffPolicy &&) = default;
  virtual ~EqualJitterBackoffPolicy() = default;

  int getDelayTime(const RetryPolicyContext &ctx) const override;

  int getPeriod() const { return period_; }
  void setPeriod(int period) { period_ = period; }

  int getCap() const { return cap_; }
  void setCap(int cap) { cap_ = cap; }

private:
  int period_;
  int cap_;
};

class FullJitterBackoffPolicy : public BackoffPolicy {
public:
  friend void to_json(Darabonba::Json &j, const FullJitterBackoffPolicy &obj) {
    to_json(j, static_cast<const BackoffPolicy &>(obj));
    DARABONBA_TO_JSON(period, period_);
    DARABONBA_TO_JSON(cap, cap_);
  }

  friend void from_json(const Darabonba::Json &j,
                        FullJitterBackoffPolicy &obj) {
    from_json(j, static_cast<BackoffPolicy &>(obj));
    DARABONBA_FROM_JSON(period, period_);
    DARABONBA_FROM_JSON(cap, cap_);
  }

  FullJitterBackoffPolicy(const Darabonba::Json &obj) { from_json(obj, *this); }
  explicit FullJitterBackoffPolicy(
      const std::map<std::string, std::string> &option)
      : BackoffPolicy(option), period_(std::stoi(option.at("period"))),
        cap_(option.count("cap") ? std::stoi(option.at("cap"))
                                 : 3 * 24 * 60 * 60 * 1000) {}

  FullJitterBackoffPolicy &operator=(const FullJitterBackoffPolicy &) = default;
  FullJitterBackoffPolicy &operator=(FullJitterBackoffPolicy &&) = default;
  virtual ~FullJitterBackoffPolicy() = default;

  int getDelayTime(const RetryPolicyContext &ctx) const override;

  int getPeriod() const { return period_; }
  void setPeriod(int period) { period_ = period; }

  int getCap() const { return cap_; }
  void setCap(int cap) { cap_ = cap; }

private:
  int period_;
  int cap_;
};

class RetryCondition {
public:
  friend void to_json(Darabonba::Json &j, const RetryCondition &obj) {
    if (obj.maxAttempts_) {
      j["maxAttempts"] = *obj.maxAttempts_;
    }
    DARABONBA_PTR_TO_JSON(backoff, backoff_);
    DARABONBA_TO_JSON(exception, exception_);
    DARABONBA_TO_JSON(errorCode, errorCode_);
    if (obj.maxDelay_) {
      j["maxDelay"] = *obj.maxDelay_;
    }
  }

  friend void from_json(const Darabonba::Json &j, RetryCondition &obj) {
    if (j.count("maxAttempts") && !j["maxAttempts"].is_null()) {
      obj.maxAttempts_ = std::make_shared<int>(j["maxAttempts"].get<int>());
    }
    //      DARABONBA_PTR_FROM_JSON(backoff, backoff_);
    DARABONBA_FROM_JSON(exception, exception_);
    DARABONBA_FROM_JSON(errorCode, errorCode_);
    if (j.count("maxDelay") && !j["maxDelay"].is_null()) {
      obj.maxDelay_ = std::make_shared<int>(j["maxDelay"].get<int>());
    }
  }

  RetryCondition() = default;
  RetryCondition(const RetryCondition &) = default;
  RetryCondition(RetryCondition &&) = default;
  RetryCondition(const Darabonba::Json &obj) { from_json(obj, *this); }
  virtual ~RetryCondition() = default;
  RetryCondition &operator=(const RetryCondition &) = default;
  RetryCondition &operator=(RetryCondition &&) = default;

  const std::shared_ptr<int> &getMaxAttemptsPtr() const { return maxAttempts_; }
  int getMaxAttempts() const {
    if (maxAttempts_)
      return *maxAttempts_;
    return 0;
  }
  RetryCondition &setMaxAttempts(int attempts) {
    maxAttempts_ = std::make_shared<int>(attempts);
    return *this;
  }

  const std::shared_ptr<BackoffPolicy> &getBackoff() const { return backoff_; }
  RetryCondition &setBackoff(const std::shared_ptr<BackoffPolicy> policy) {
    backoff_ = policy;
    return *this;
  }

  const std::vector<std::string> &getException() const { return exception_; }
  RetryCondition &setException(const std::vector<std::string> &exceptions) {
    exception_ = exceptions;
    return *this;
  }

  const std::vector<std::string> &getErrorCode() const { return errorCode_; }
  RetryCondition &setErrorCode(const std::vector<std::string> &codes) {
    errorCode_ = codes;
    return *this;
  }

  const std::shared_ptr<int> &getMaxDelayPtr() const { return maxDelay_; }
  int getMaxDelay() const {
    if (maxDelay_)
      return *maxDelay_;
    return 0;
  }
  RetryCondition &setMaxDelay(int delay) {
    maxDelay_ = std::make_shared<int>(delay);
    return *this;
  }

protected:
  std::shared_ptr<int> maxAttempts_ = nullptr;
  std::shared_ptr<BackoffPolicy> backoff_ = nullptr;
  std::vector<std::string> exception_;
  std::vector<std::string> errorCode_;
  std::shared_ptr<int> maxDelay_ = nullptr;
};

class RetryOptions {
public:
  friend void to_json(Darabonba::Json &j, const RetryOptions &obj) {
    DARABONBA_TO_JSON(retryCondition, retryCondition_);
    DARABONBA_TO_JSON(noRetryCondition, noRetryCondition_);
  }

  friend void from_json(const Darabonba::Json &j, RetryOptions &obj) {
    DARABONBA_FROM_JSON(retryCondition, retryCondition_);
    DARABONBA_FROM_JSON(noRetryCondition, noRetryCondition_);
  }

  RetryOptions() = default;
  RetryOptions(const RetryOptions &) = default;
  RetryOptions(RetryOptions &&) = default;
  RetryOptions(const Darabonba::Json &obj) { from_json(obj, *this); };
  explicit RetryOptions(const std::map<std::string, std::string> &options);
  RetryOptions &operator=(const RetryOptions &) = default;
  RetryOptions &operator=(RetryOptions &&) = default;
  ~RetryOptions() = default;

  // 检查是否可重试
  bool isRetryable() const { return retryable_; }
  void setRetryable(bool retryable) { retryable_ = retryable; }

  // 访问重试条件
  const std::vector<RetryCondition> &getRetryCondition() const {
    return retryCondition_;
  }
  RetryOptions &
  setRetryCondition(const std::vector<RetryCondition> &conditions) {
    retryCondition_ = conditions;
    return *this;
  }

  // 访问非重试条件
  const std::vector<RetryCondition> &getNoRetryCondition() const {
    return noRetryCondition_;
  }
  RetryOptions &
  setNoRetryCondition(const std::vector<RetryCondition> &conditions) {
    noRetryCondition_ = conditions;
    return *this;
  }

protected:
  bool retryable_ = false;
  std::vector<RetryCondition> retryCondition_;
  std::vector<RetryCondition> noRetryCondition_;
};

class RetryPolicyContext {
public:
  friend void to_json(Darabonba::Json &j, const RetryPolicyContext &obj) {
    DARABONBA_TO_JSON(retriesAttempted, retriesAttempted_);
    DARABONBA_PTR_TO_JSON(exception, exception_);
    DARABONBA_TO_JSON(lastRequest, lastRequest_);
    DARABONBA_TO_JSON(lastResponse, lastResponse_);
  }

  friend void from_json(const Darabonba::Json &j, RetryPolicyContext &obj) {
    DARABONBA_ANY_FROM_JSON(retriesAttempted, retriesAttempted_);
    DARABONBA_PTR_FROM_JSON(exception, exception_);
    DARABONBA_FROM_JSON(lastRequest, lastRequest_);
    DARABONBA_FROM_JSON(lastResponse, lastResponse_);
  }

  RetryPolicyContext() = default;
  RetryPolicyContext(const RetryPolicyContext &) = default;
  RetryPolicyContext(RetryPolicyContext &&) = default;
  RetryPolicyContext(const Darabonba::Json &obj) { from_json(obj, *this); };
  ~RetryPolicyContext() = default;

  RetryPolicyContext(int retriesAttempted,
                     std::shared_ptr<DaraException> httpException)
      : retriesAttempted_(retriesAttempted), exception_(httpException) {}

  RetryPolicyContext &operator=(const RetryPolicyContext &) = default;
  RetryPolicyContext &operator=(RetryPolicyContext &&) = default;

  int getRetriesAttempted() const { return retriesAttempted_; }
  RetryPolicyContext &setRetriesAttempted(int retries) {
    retriesAttempted_ = retries;
    return *this;
  }

  std::shared_ptr<DaraException> getException() const { return exception_; }
  RetryPolicyContext &setException(std::shared_ptr<DaraException> ex) {
    exception_ = ex;
    return *this;
  }

  std::shared_ptr<Darabonba::Http::MCurlResponse> getLastResponse() const {
    return lastResponse_;
  }
  RetryPolicyContext &
  setLastResponse(std::shared_ptr<Darabonba::Http::MCurlResponse> resp) {
    lastResponse_ = resp;
    return *this;
  }

  std::shared_ptr<Darabonba::Http::Request> getLastRequest() const {
    return lastRequest_;
  }
  RetryPolicyContext &
  setLastRequest(std::shared_ptr<Darabonba::Http::Request> resp) {
    lastRequest_ = resp;
    return *this;
  }

protected:
  int retriesAttempted_ = 0;             // 已尝试的重试次数
  std::shared_ptr<DaraException> exception_; // HTTP 异常
  std::shared_ptr<Darabonba::Http::MCurlResponse> lastResponse_;
  std::shared_ptr<Darabonba::Http::Request> lastRequest_;
};

bool shouldRetry(const RetryOptions &options, const RetryPolicyContext &ctx);
int getBackoffDelay(const RetryOptions &options, const RetryPolicyContext &ctx);
} // namespace Policy
} // namespace Darabonba

#endif // DARABONBA_RETRY_HPP
