#include <darabonba/policy/Retry.hpp>
#include <random>
#include <algorithm>

namespace Darabonba {
namespace Policy {

// Helper function for generating random integers
static int getRandomInt(int cap) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, cap);
    return dist(mt);
}

// BackoffPolicy factory method
std::unique_ptr<BackoffPolicy> BackoffPolicy::createBackoffPolicy(const std::map<std::string, std::string>& option) {
    std::string policy = option.at("policy");
    if (policy == "Fixed") {
        // return std::make_unique<FixedBackoffPolicy>(option);
    } else if (policy == "Random") {
        // return std::make_unique<RandomBackoffPolicy>(option);
    }
    throw std::invalid_argument("Unknown policy: " + policy);
}

// FixedBackoffPolicy definition
// FixedBackoffPolicy::FixedBackoffPolicy(const std::map<std::string, std::string>& option)
// : period_(std::stoi(option.at("period"))) {}

// int FixedBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
//     return period_;
// }

// RandomBackoffPolicy definition
// RandomBackoffPolicy::RandomBackoffPolicy(const std::map<std::string, std::string>& option)
// : period_(std::stoi(option.at("period"))), cap_(std::stoi(option.at("cap"))) {}

// int RandomBackoffPolicy::getDelayTime(const RetryPolicyContext& ctx) const {
//     return std::min(getRandomInt(ctx.retriesAttempted() * period_), cap_);
// }

// RetryCondition implemented as needed...

// RetryOptions implemented as needed...

// RetryPolicyContext constructor
// RetryPolicyContext::RetryPolicyContext(int retriesAttempted, Darabonba::Exception exception)
// : retriesAttempted_(retriesAttempted), exception_(exception) {}

// getBackoffDelay function implementation
int getBackoffDelay(const RetryOptions& options, const RetryPolicyContext& ctx) {
    for (const auto& condition : options.getRetryCondition()) {
        // int maxDelay = condition.maxDelay ? *condition.maxDelay() : MaxDelayTime;
        // if (!condition.backoff()) {
        //     return MinDelayTime;
        // }
        // return std::min(condition.backoff()->getDelayTime(ctx), maxDelay);
    }
    return MinDelayTime;
}

} // namespace Policy
} // namespace Darabonba
