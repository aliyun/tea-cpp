#include <darabonba/policy/Retry.hpp>
#include <random>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>

namespace Darabonba {
namespace Policy {

// Helper function for generating random integers - 使用静态变量避免重复初始化
static int getRandomInt(int cap) {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    if (cap <= 0) return 0;
    std::uniform_int_distribution<int> dist(0, cap);
    return dist(mt);
}

// BackoffPolicy factory method
std::unique_ptr<BackoffPolicy> BackoffPolicy::createBackoffPolicy(const std::map<std::string, std::string>& option) {
    std::string policy = option.at("policy");
    if (policy == "Fixed") {
        return std::unique_ptr<BackoffPolicy>(new FixedBackoffPolicy(option));
    } else if (policy == "Random") {
        return std::unique_ptr<BackoffPolicy>(new RandomBackoffPolicy(option));
    } else if (policy == "Exponential") {
        return std::unique_ptr<BackoffPolicy>(new ExponentialBackoffPolicy(option));
    } else if (policy == "EqualJitter" || policy == "ExponentialWithEqualJitter") {
        return std::unique_ptr<BackoffPolicy>(new EqualJitterBackoffPolicy(option));
    } else if (policy == "FullJitter" || policy == "ExponentialWithFullJitter") {
        return std::unique_ptr<BackoffPolicy>(new FullJitterBackoffPolicy(option));
    }
    throw std::invalid_argument("Unknown policy: " + policy);
}

// ExponentialBackoffPolicy implementation
int ExponentialBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
    int randomTime = static_cast<int>(std::pow(2, ctx.getRetriesAttempted() * period_));
    return std::min(randomTime, cap_);
}

// EqualJitterBackoffPolicy implementation
int EqualJitterBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
    int ceil = std::min(cap_, static_cast<int>(std::pow(2, ctx.getRetriesAttempted() * period_)));
    int jitter = getRandomInt(ceil / 2);
    return ceil / 2 + jitter;
}

// FullJitterBackoffPolicy implementation
int FullJitterBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
    int ceil = std::min(cap_, static_cast<int>(std::pow(2, ctx.getRetriesAttempted() * period_)));
    return getRandomInt(ceil);
}

// shouldRetry function implementation
bool shouldRetry(const RetryOptions& options, const RetryPolicyContext& ctx) {
    // 第一次尝试，直接返回 true
    if (ctx.getRetriesAttempted() == 0) {
        return true;
    }

    // 检查是否可重试
    if (!options.isRetryable()) {
        return false;
    }

    auto ex = ctx.getException();
    if (!ex) {
        return false;
    }

    const std::string& exceptionName = ex->getName();
    const std::string& exceptionCode = ex->getCode();
    int retriesAttempted = ctx.getRetriesAttempted();

    // 检查 NoRetryCondition
    const auto& noRetryConditions = options.getNoRetryCondition();
    for (const auto& condition : noRetryConditions) {
        const auto& exceptions = condition.getException();
        const auto& errorCodes = condition.getErrorCode();
        
        if (std::find(exceptions.begin(), exceptions.end(), exceptionName) != exceptions.end() ||
            std::find(errorCodes.begin(), errorCodes.end(), exceptionCode) != errorCodes.end()) {
            return false;
        }
    }

    // 检查 RetryCondition
    const auto& retryConditions = options.getRetryCondition();
    for (const auto& condition : retryConditions) {
        const auto& exceptions = condition.getException();
        const auto& errorCodes = condition.getErrorCode();
        
        if (std::find(exceptions.begin(), exceptions.end(), exceptionName) == exceptions.end() &&
            std::find(errorCodes.begin(), errorCodes.end(), exceptionCode) == errorCodes.end()) {
            continue;
        }

        if (retriesAttempted >= condition.getMaxAttempts()) {
            return false;
        }
        return true;
    }

    return false;
}

// getBackoffDelay function implementation
int getBackoffDelay(const RetryOptions& options, const RetryPolicyContext& ctx) {
    auto ex = ctx.getException();
    if (!ex) {
        return MinDelayTime;
    }

    const std::string& exceptionName = ex->getName();
    const std::string& exceptionCode = ex->getCode();

    const auto& conditions = options.getRetryCondition();
    for (const auto& condition : conditions) {
        const auto& exceptions = condition.getException();
        const auto& errorCodes = condition.getErrorCode();
        
        if (std::find(exceptions.begin(), exceptions.end(), exceptionName) == exceptions.end() &&
            std::find(errorCodes.begin(), errorCodes.end(), exceptionCode) == errorCodes.end()) {
            continue;
        }

        int maxDelay = condition.getMaxDelay() > 0 ? condition.getMaxDelay() : MaxDelayTime;
        
        // 检查是否有 retryAfter - 尝试转换为 ResponseException
        auto responseEx = std::dynamic_pointer_cast<ResponseException>(ex);
        if (responseEx) {
            int64_t retryAfter = responseEx->getRetryAfter();
            if (retryAfter > 0) {
                return std::min(static_cast<int>(retryAfter), maxDelay);
            }
        }

        // 如果没有 backoff 策略，返回最小延迟时间
        auto backoff = condition.getBackoff();
        if (!backoff) {
            return MinDelayTime;
        }

        return std::min(backoff->getDelayTime(ctx), maxDelay);
    }
    
    return MinDelayTime;
}

} // namespace Policy
} // namespace Darabonba
