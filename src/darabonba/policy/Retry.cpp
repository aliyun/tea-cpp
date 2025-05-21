#include <darabonba/policy/Retry.hpp>
#include <random>
#include <cmath>
#include <algorithm>

namespace Darabonba {

// 获取随机整数
static int getRandomInt(int cap) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, cap);
    return dist(mt);
}

// BackoffPolicy 创建工厂方法
std::unique_ptr<BackoffPolicy> BackoffPolicy::createBackoffPolicy(const std::map<std::string, std::string>& option) {
    std::string policy = option.at("policy");
    if (policy == "Fixed") {
        // return std::make_unique<FixedBackoffPolicy>(option);
    } else if (policy == "Random") {
        // return std::make_unique<RandomBackoffPolicy>(option);
    }
    // 继续为其他策略增加实现...
    throw std::invalid_argument("Unknown policy: " + policy);
}

// FixedBackoffPolicy 实现
FixedBackoffPolicy::FixedBackoffPolicy(const std::map<std::string, std::string>& option)
: BackoffPolicy(option), _period(std::stoi(option.at("period"))) {}

int FixedBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
    return _period;
}

// RandomBackoffPolicy 实现
RandomBackoffPolicy::RandomBackoffPolicy(const std::map<std::string, std::string>& option)
: BackoffPolicy(option), _period(std::stoi(option.at("period"))), _cap(std::stoi(option.at("cap"))) {}

int RandomBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
    int randomTime = getRandomInt(ctx.retriesAttempted * _period);
    return std::min(randomTime, _cap);
}

// RetryCondition 实现
RetryCondition::RetryCondition(const std::map<std::string, std::string>& condition) {
    maxAttempts = std::stoi(condition.at("maxAttempts"));
    backoff = BackoffPolicy::createBackoffPolicy(condition); // Assuming backoff object here
    // Initialize exception and errorCode vectors
    // maxDelay = logic for maxDelay
}

// RetryOptions 实现
RetryOptions::RetryOptions(const std::map<std::string, std::string>& options) {
    _retryable = options.at("retryable") == "true";
    // Populate _retryCondition and _noRetryCondition vectors
}

bool RetryOptions::isValid() const {
    if (!_retryable && _retryCondition.empty() && _noRetryCondition.empty()) {
        throw std::invalid_argument("Invalid retry options configuration.");
    }
    return true;
}

// RetryPolicyContext 实现
RetryPolicyContext::RetryPolicyContext(int retriesAttempted, std::exception_ptr httpException)
: retriesAttempted(retriesAttempted), exception(httpException) {}

// getBackoffDelay 函数实现
int getBackoffDelay(const RetryOptions& options, const RetryPolicyContext& ctx) {
    // Here ctx.exception needs to be converted to something meaningful like name, code, etc.
    for (const auto& condition : options.getRetryConditions()) {
        int maxDelay = condition.maxDelay ? condition.maxDelay : MaxDelayTime;
        if (!condition.backoff) {
            return MinDelayTime;
        }
        return std::min(condition.backoff->getDelayTime(ctx), maxDelay);
    }

    return MinDelayTime;
}

} // namespace Darabonba
