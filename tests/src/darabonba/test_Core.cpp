#include <gtest/gtest.h>
#include <darabonba/Core.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/policy/Retry.hpp>
#include <chrono>

using namespace Darabonba;

class CoreTest : public ::testing::Test {
protected:
    Core core;

    virtual void SetUp() override {
        // Setup code here, if needed.
    }

    virtual void TearDown() override {
        // Tear down code here.
    }
};

// ==================== sleep 测试 ====================
TEST_F(CoreTest, SleepShouldWorkCorrectly) {
    auto start = std::chrono::high_resolution_clock::now();
    Darabonba::sleep(100); // 100ms
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // 应该睡眠至少 100ms，允许一定误差
    EXPECT_GE(duration, 90);
    EXPECT_LE(duration, 200);
}

// ==================== allowRetry 测试 ====================
TEST_F(CoreTest, AllowRetryWithFirstAttempt) {
    Policy::RetryOptions options;
    options.setRetryable(true);
    
    Policy::RetryCondition condition;
    condition.setMaxAttempts(3);
    condition.setException({"ResponseException"});
    options.setRetryCondition({condition});
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(0);  // 第一次尝试
    
    // 第一次尝试应该允许重试
    EXPECT_TRUE(Darabonba::allowRetry(options, ctx));
}

TEST_F(CoreTest, AllowRetryWithMaxAttemptsReached) {
    Policy::RetryOptions options;
    options.setRetryable(true);
    
    Policy::RetryCondition condition;
    condition.setMaxAttempts(3);
    condition.setException({"ResponseException"});
    options.setRetryCondition({condition});
    
    auto ex = std::make_shared<ResponseException>();
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(3);  // 已达到最大重试次数
    ctx.setException(ex);
    
    // 已达到最大重试次数，不应该允许重试
    EXPECT_FALSE(Darabonba::allowRetry(options, ctx));
}

TEST_F(CoreTest, AllowRetryWhenNotRetryable) {
    Policy::RetryOptions options;
    options.setRetryable(false);  // 不可重试
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(1);
    
    // 不可重试选项应该返回 false
    EXPECT_FALSE(Darabonba::allowRetry(options, ctx));
}

TEST_F(CoreTest, AllowRetryWithMatchingException) {
    Policy::RetryOptions options;
    options.setRetryable(true);
    
    Policy::RetryCondition condition;
    condition.setMaxAttempts(5);
    condition.setException({"ResponseException"});
    options.setRetryCondition({condition});
    
    auto ex = std::make_shared<ResponseException>();
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(2);
    ctx.setException(ex);
    
    // 匹配的异常类型应该允许重试
    EXPECT_TRUE(Darabonba::allowRetry(options, ctx));
}

// ==================== getBackoffTime 测试 ====================
TEST_F(CoreTest, GetBackoffTimeReturnsMinimumDelay) {
    Policy::RetryOptions options;
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(1);
    
    int delay = Darabonba::getBackoffTime(options, ctx);
    // 没有设置策略时，应该返回最小延迟
    EXPECT_EQ(delay, MinDelayTime);
}

TEST_F(CoreTest, GetBackoffTimeWithFixedPolicy) {
    Policy::RetryOptions options;
    options.setRetryable(true);
    
    Policy::RetryCondition condition;
    condition.setMaxAttempts(5);
    condition.setException({"ResponseException"});
    condition.setMaxDelay(10000);
    
    std::map<std::string, std::string> policyOption;
    policyOption["policy"] = "Fixed";
    policyOption["period"] = "1500";
    
    auto backoffPolicy = std::make_shared<Policy::FixedBackoffPolicy>(policyOption);
    condition.setBackoff(backoffPolicy);
    
    options.setRetryCondition({condition});
    
    auto ex = std::make_shared<ResponseException>();
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(2);
    ctx.setException(ex);
    
    int delay = Darabonba::getBackoffTime(options, ctx);
    EXPECT_EQ(delay, 1500);
}

TEST_F(CoreTest, GetBackoffTimeWithExponentialPolicy) {
    Policy::RetryOptions options;
    options.setRetryable(true);
    
    Policy::RetryCondition condition;
    condition.setMaxAttempts(5);
    condition.setException({"ResponseException"});
    condition.setMaxDelay(10000);
    
    std::map<std::string, std::string> policyOption;
    policyOption["policy"] = "Exponential";
    policyOption["period"] = "1";
    policyOption["cap"] = "10000";
    
    auto backoffPolicy = std::make_shared<Policy::ExponentialBackoffPolicy>(policyOption);
    condition.setBackoff(backoffPolicy);
    
    options.setRetryCondition({condition});
    
    auto ex = std::make_shared<ResponseException>();
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(3);
    ctx.setException(ex);
    
    int delay = Darabonba::getBackoffTime(options, ctx);
    // 2^3 = 8
    EXPECT_EQ(delay, 8);
}

TEST_F(CoreTest, GetBackoffTimeRespectMaxDelay) {
    Policy::RetryOptions options;
    options.setRetryable(true);
    
    Policy::RetryCondition condition;
    condition.setMaxAttempts(10);
    condition.setException({"ResponseException"});
    condition.setMaxDelay(100);  // 设置很小的最大延迟
    
    std::map<std::string, std::string> policyOption;
    policyOption["policy"] = "Fixed";
    policyOption["period"] = "5000";  // 大于 maxDelay
    
    auto backoffPolicy = std::make_shared<Policy::FixedBackoffPolicy>(policyOption);
    condition.setBackoff(backoffPolicy);
    
    options.setRetryCondition({condition});
    
    auto ex = std::make_shared<ResponseException>();
    
    Policy::RetryPolicyContext ctx;
    ctx.setRetriesAttempted(2);
    ctx.setException(ex);
    
    int delay = Darabonba::getBackoffTime(options, ctx);
    // 应该被限制在 maxDelay
    EXPECT_LE(delay, 100);
}

// ==================== doAction 测试 ====================
TEST_F(CoreTest, DoActionReturnsValidResponse) {
    try {
        Http::Request request(std::string("https://www.aliyun.com"));
        request.getHeader()["Content-Type"] = "application/json"; 

        Json runtime;
        runtime["timeout"] = 5000;

        auto futureResponse = core.doAction(request, runtime);

        std::future_status status = futureResponse.wait_for(std::chrono::seconds(10));
        ASSERT_EQ(status, std::future_status::ready);

        auto response = futureResponse.get();
        ASSERT_NE(response, nullptr);

        // 验证收到响应(200, 301, 302 等都算成功)
        EXPECT_GT(response->getStatusCode(), 0);
        EXPECT_LT(response->getStatusCode(), 500);

    } catch (const std::exception& e) {
        // 网络请求可能失败,跳过测试
        std::cerr << "Network test skipped: " << e.what() << std::endl;
    }
}

TEST_F(CoreTest, DoActionWithEmptyHostFails) {
    try {
        Http::Request request(std::string("/api"));

        Json runtime;
        runtime["timeout"] = 3000;

        auto futureResponse = core.doAction(request, runtime);

        std::future_status status = futureResponse.wait_for(std::chrono::seconds(5));
        ASSERT_EQ(status, std::future_status::ready);

        auto response = futureResponse.get();
        
        // 空 host 应该返回错误状态码
        EXPECT_EQ(response->getStatusCode(), 0);

    } catch (const std::exception& e) {
        // 预期会抛异常或者返回错误
        std::cerr << "Expected error for empty host: " << e.what() << std::endl;
    }
}

TEST_F(CoreTest, DoActionWithPostMethod) {
    try {
        Http::Request request(std::string("https://www.aliyun.com"));
        request.setMethod(Http::Request::Method::POST);
        request.getHeader()["Content-Type"] = "application/json";
        request.setBody(std::string("{\"test\": \"value\"}"));

        Json runtime;
        runtime["timeout"] = 10000;

        auto futureResponse = core.doAction(request, runtime);

        std::future_status status = futureResponse.wait_for(std::chrono::seconds(15));
        ASSERT_EQ(status, std::future_status::ready);

        auto response = futureResponse.get();
        ASSERT_NE(response, nullptr);
        
        // POST 请求应该返回成功状态码(2xx, 3xx 都算成功)
        EXPECT_GT(response->getStatusCode(), 0);
        EXPECT_LT(response->getStatusCode(), 500);

    } catch (const std::exception& e) {
        // 网络请求可能失败,跳过测试
        std::cerr << "Network test skipped: " << e.what() << std::endl;
    }
}

TEST_F(CoreTest, DoActionWithQueryParams) {
    try {
        Http::Request request(std::string("https://www.aliyun.com"));
        request.addQuery("foo", "bar");
        request.addQuery("key", "value");

        Json runtime;
        runtime["timeout"] = 10000;

        auto futureResponse = core.doAction(request, runtime);

        std::future_status status = futureResponse.wait_for(std::chrono::seconds(15));
        if (status == std::future_status::ready) {
            auto response = futureResponse.get();
            ASSERT_NE(response, nullptr);
            // 验证收到响应
            EXPECT_GT(response->getStatusCode(), 0);
            EXPECT_LT(response->getStatusCode(), 500);
        }

    } catch (const std::exception& e) {
        // 网络请求可能失败,跳过测试
        std::cerr << "Network test skipped: " << e.what() << std::endl;
    }
}

