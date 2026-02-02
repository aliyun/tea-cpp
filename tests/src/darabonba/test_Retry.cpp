#include <darabonba/Exception.hpp>
#include <darabonba/policy/Retry.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace Darabonba;
using namespace Darabonba::Policy;

class RetryTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    // Setup code here, if needed.
  }

  virtual void TearDown() override {
    // Tear down code here.
  }
};

// Test FixedBackoffPolicy
TEST_F(RetryTest, FixedBackoffPolicyReturnsFixedDelay) {
  std::map<std::string, std::string> option;
  option["policy"] = "Fixed";
  option["period"] = "1000";

  FixedBackoffPolicy policy(option);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);

  EXPECT_EQ(policy.getDelayTime(ctx), 1000);

  ctx.setRetriesAttempted(5);
  EXPECT_EQ(policy.getDelayTime(ctx), 1000);
}

// Test RandomBackoffPolicy
TEST_F(RetryTest, RandomBackoffPolicyReturnsRandomDelay) {
  std::map<std::string, std::string> option;
  option["policy"] = "Random";
  option["period"] = "100";
  option["cap"] = "5000";

  RandomBackoffPolicy policy(option);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(10);

  int delay = policy.getDelayTime(ctx);
  EXPECT_GE(delay, 100);
  EXPECT_LE(delay, 5000);
}

// Test ExponentialBackoffPolicy
TEST_F(RetryTest, ExponentialBackoffPolicyReturnsExponentialDelay) {
  std::map<std::string, std::string> option;
  option["policy"] = "Exponential";
  option["period"] = "1";
  option["cap"] = "10000";

  ExponentialBackoffPolicy policy(option);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);

  int delay = policy.getDelayTime(ctx);
  EXPECT_EQ(delay, 2); // 2^(1*1) = 2

  ctx.setRetriesAttempted(2);
  delay = policy.getDelayTime(ctx);
  EXPECT_EQ(delay, 4); // 2^(2*1) = 4

  ctx.setRetriesAttempted(3);
  delay = policy.getDelayTime(ctx);
  EXPECT_EQ(delay, 8); // 2^(3*1) = 8
}

// Test EqualJitterBackoffPolicy
TEST_F(RetryTest, EqualJitterBackoffPolicyReturnsJitteredDelay) {
  std::map<std::string, std::string> option;
  option["policy"] = "EqualJitter";
  option["period"] = "1";
  option["cap"] = "10000";

  EqualJitterBackoffPolicy policy(option);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(3);

  int delay = policy.getDelayTime(ctx);
  // ceil = min(10000, 2^3) = 8
  // delay should be between ceil/2 and ceil = [4, 8]
  EXPECT_GE(delay, 4);
  EXPECT_LE(delay, 8);
}

// Test FullJitterBackoffPolicy
TEST_F(RetryTest, FullJitterBackoffPolicyReturnsFullJitteredDelay) {
  std::map<std::string, std::string> option;
  option["policy"] = "FullJitter";
  option["period"] = "1";
  option["cap"] = "10000";

  FullJitterBackoffPolicy policy(option);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(3);

  int delay = policy.getDelayTime(ctx);
  // ceil = min(10000, 2^3) = 8
  // delay should be between 0 and ceil = [0, 8]
  EXPECT_GE(delay, 0);
  EXPECT_LE(delay, 8);
}

// Test BackoffPolicy factory method
TEST_F(RetryTest, BackoffPolicyFactoryCreatesCorrectPolicy) {
  std::map<std::string, std::string> option;

  option["policy"] = "Fixed";
  option["period"] = "1000";
  auto fixedPolicy = BackoffPolicy::createBackoffPolicy(option);
  EXPECT_NE(fixedPolicy, nullptr);

  option["policy"] = "Random";
  option["cap"] = "5000";
  auto randomPolicy = BackoffPolicy::createBackoffPolicy(option);
  EXPECT_NE(randomPolicy, nullptr);

  option["policy"] = "Exponential";
  auto exponentialPolicy = BackoffPolicy::createBackoffPolicy(option);
  EXPECT_NE(exponentialPolicy, nullptr);

  option["policy"] = "EqualJitter";
  auto equalJitterPolicy = BackoffPolicy::createBackoffPolicy(option);
  EXPECT_NE(equalJitterPolicy, nullptr);

  option["policy"] = "FullJitter";
  auto fullJitterPolicy = BackoffPolicy::createBackoffPolicy(option);
  EXPECT_NE(fullJitterPolicy, nullptr);
}

// Test shouldRetry - first attempt
TEST_F(RetryTest, ShouldRetryReturnsTrueForFirstAttempt) {
  RetryOptions options;
  options.setRetryable(true);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(0);

  EXPECT_TRUE(shouldRetry(options, ctx));
}

// Test shouldRetry - not retryable
TEST_F(RetryTest, ShouldRetryReturnsFalseWhenNotRetryable) {
  RetryOptions options;
  options.setRetryable(false);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);

  EXPECT_FALSE(shouldRetry(options, ctx));
}

// Test shouldRetry - with retry condition
TEST_F(RetryTest, ShouldRetryWithRetryCondition) {
  RetryOptions options;
  options.setRetryable(true);

  RetryCondition condition;
  condition.setMaxAttempts(3);
  condition.setException({"ResponseException"});
  condition.setErrorCode({"Throttling"});

  std::vector<RetryCondition> conditions = {condition};
  options.setRetryCondition(conditions);

  auto ex = std::make_shared<ResponseException>();

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);
  ctx.setException(ex);

  // 应该重试，因为重试次数小于 maxAttempts
  EXPECT_TRUE(shouldRetry(options, ctx));

  ctx.setRetriesAttempted(3);
  // 不应该重试，因为重试次数已达到 maxAttempts
  EXPECT_FALSE(shouldRetry(options, ctx));
}

// Test shouldRetry - with no retry condition
TEST_F(RetryTest, ShouldRetryWithNoRetryCondition) {
  RetryOptions options;
  options.setRetryable(true);

  RetryCondition retryCondition;
  retryCondition.setMaxAttempts(3);
  retryCondition.setException({"ResponseException"});

  RetryCondition noRetryCondition;
  noRetryCondition.setException({"ValidateException"});

  std::vector<RetryCondition> retryConditions = {retryCondition};
  std::vector<RetryCondition> noRetryConditions = {noRetryCondition};

  options.setRetryCondition(retryConditions);
  options.setNoRetryCondition(noRetryConditions);

  auto ex = std::make_shared<Exception>("Validation failed");

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);
  ctx.setException(ex);

  // 不应该重试，因为异常在 noRetryCondition 中
  EXPECT_FALSE(shouldRetry(options, ctx));
}

// Test getBackoffDelay - minimum delay
TEST_F(RetryTest, GetBackoffDelayReturnsMinimumDelay) {
  RetryOptions options;

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);

  int delay = getBackoffDelay(options, ctx);
  EXPECT_EQ(delay, MinDelayTime);
}

// Test getBackoffDelay - with backoff policy
TEST_F(RetryTest, GetBackoffDelayWithBackoffPolicy) {
  RetryOptions options;
  options.setRetryable(true);

  RetryCondition condition;
  condition.setMaxAttempts(3);
  condition.setException({"ResponseException"});
  condition.setMaxDelay(10000);

  std::map<std::string, std::string> policyOption;
  policyOption["policy"] = "Fixed";
  policyOption["period"] = "2000";

  auto backoffPolicy = std::make_shared<FixedBackoffPolicy>(policyOption);
  condition.setBackoff(backoffPolicy);

  std::vector<RetryCondition> conditions = {condition};
  options.setRetryCondition(conditions);

  auto ex = std::make_shared<ResponseException>();

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);
  ctx.setException(ex);

  int delay = getBackoffDelay(options, ctx);
  EXPECT_EQ(delay, 2000);
}

// Test getBackoffDelay - with retryAfter
TEST_F(RetryTest, GetBackoffDelayWithRetryAfter) {
  RetryOptions options;
  options.setRetryable(true);

  RetryCondition condition;
  condition.setMaxAttempts(3);
  condition.setException({"ResponseException"});
  condition.setMaxDelay(10000);

  std::vector<RetryCondition> conditions = {condition};
  options.setRetryCondition(conditions);

  // Note: 我们需要创建一个有 retryAfter 的异常
  // 由于 ResponseException 通过 JSON 构造，我们创建一个包含 retryAfter 的 JSON
  Json exceptionJson;
  exceptionJson["code"] = "Throttling";
  exceptionJson["message"] = "Request throttled";
  exceptionJson["retryAfter"] = 5000;

  auto ex = std::make_shared<ResponseException>(exceptionJson);

  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(1);
  ctx.setException(ex);

  int delay = getBackoffDelay(options, ctx);
  EXPECT_EQ(delay, 5000);
}

// Test RetryCondition JSON serialization
TEST_F(RetryTest, RetryConditionJsonSerialization) {
  RetryCondition condition;
  condition.setMaxAttempts(5);
  condition.setException({"ResponseException", "ServerException"});
  condition.setErrorCode({"Throttling", "ServiceUnavailable"});
  condition.setMaxDelay(30000);

  Json j;
  to_json(j, condition);

  EXPECT_EQ(j["maxAttempts"], 5);
  EXPECT_EQ(j["maxDelay"], 30000);
  EXPECT_EQ(j["exception"].size(), 2u);
  EXPECT_EQ(j["errorCode"].size(), 2u);

  RetryCondition condition2;
  from_json(j, condition2);

  EXPECT_EQ(condition2.getMaxAttempts(), 5);
  EXPECT_EQ(condition2.getMaxDelay(), 30000);
  EXPECT_EQ(condition2.getException().size(), 2u);
  EXPECT_EQ(condition2.getErrorCode().size(), 2u);
}

// Test RetryOptions JSON serialization
TEST_F(RetryTest, RetryOptionsJsonSerialization) {
  RetryOptions options;
  options.setRetryable(true);

  RetryCondition condition;
  condition.setMaxAttempts(3);
  condition.setException({"ResponseException"});

  std::vector<RetryCondition> conditions = {condition};
  options.setRetryCondition(conditions);

  Json j;
  to_json(j, options);

  EXPECT_EQ(j["retryCondition"].size(), 1u);

  RetryOptions options2;
  from_json(j, options2);

  EXPECT_EQ(options2.getRetryCondition().size(), 1u);
}

// Test RetryPolicyContext JSON serialization
TEST_F(RetryTest, RetryPolicyContextJsonSerialization) {
  RetryPolicyContext ctx;
  ctx.setRetriesAttempted(3);

  auto ex = std::make_shared<Exception>("Test exception");
  ctx.setException(ex);

  Json j;
  to_json(j, ctx);

  EXPECT_EQ(j["retriesAttempted"], 3);

  RetryPolicyContext ctx2;
  from_json(j, ctx2);

  EXPECT_EQ(ctx2.getRetriesAttempted(), 3);
}
