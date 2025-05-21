#ifndef DARABONBA_RETRY_HPP
#define DARABONBA_RETRY_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

// 定义最大和最小延迟时间
const int MaxDelayTime = 120 * 1000;
const int MinDelayTime = 100;

namespace Darabonba {

class RetryPolicyContext; // 前向声明

// BackoffPolicy 基类
class BackoffPolicy {
public:
    explicit BackoffPolicy(const std::map<std::string, std::string>& option) : _policy(option.at("policy")) {}

    virtual int getDelayTime(const RetryPolicyContext& ctx) const = 0;
    static std::unique_ptr<BackoffPolicy> createBackoffPolicy(const std::map<std::string, std::string>& option);

protected:
    std::string _policy;
};

// 具体的 Backoff 策略
class FixedBackoffPolicy : public BackoffPolicy {
public:
    explicit FixedBackoffPolicy(const std::map<std::string, std::string>& option);
    int getDelayTime(const RetryPolicyContext& ctx) const override;

private:
    int _period;
};

class RandomBackoffPolicy : public BackoffPolicy {
public:
    explicit RandomBackoffPolicy(const std::map<std::string, std::string>& option);
    int getDelayTime(const RetryPolicyContext& ctx) const override;

private:
    int _period;
    int _cap;
};

// 定义其他策略如 ExponentialBackoffPolicy、EqualJitterBackoffPolicy、FullJitterBackoffPolicy...

class RetryCondition {
public:
    explicit RetryCondition(const std::map<std::string, std::string>& condition);

    int maxAttempts;
    std::unique_ptr<BackoffPolicy> backoff;
    std::vector<std::string> exception;
    std::vector<std::string> errorCode;
    int maxDelay;
};

class RetryOptions {
public:
    explicit RetryOptions(const std::map<std::string, std::string>& options);

    bool isValid() const;
    const std::vector<RetryCondition>& getRetryConditions() const { return _retryCondition; }
    const std::vector<RetryCondition>& getNoRetryConditions() const { return _noRetryCondition; }

private:
    bool _retryable;
    std::vector<RetryCondition> _retryCondition;
    std::vector<RetryCondition> _noRetryCondition;
};

class RetryPolicyContext {
public:
    RetryPolicyContext(int retriesAttempted, std::exception_ptr httpException);

    int retriesAttempted;
    std::exception_ptr exception;
};

// 功能函数
int getBackoffDelay(const RetryOptions& options, const RetryPolicyContext& ctx);

} // namespace Darabonba

#endif // DARABONBA_RETRY_HPP
