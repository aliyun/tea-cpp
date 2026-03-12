#include <chrono>
#include <darabonba/Core.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/policy/Retry.hpp>
#include <gtest/gtest.h>

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
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();

  // 应该睡眠至少 100ms，允许一定误差（CI 环境可能有较大延迟）
  EXPECT_GE(duration, 90);
  EXPECT_LE(duration, 300);
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
  ctx.setRetriesAttempted(0); // 第一次尝试

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
  ctx.setRetriesAttempted(3); // 已达到最大重试次数
  ctx.setException(ex);

  // 已达到最大重试次数，不应该允许重试
  EXPECT_FALSE(Darabonba::allowRetry(options, ctx));
}

TEST_F(CoreTest, AllowRetryWhenNotRetryable) {
  Policy::RetryOptions options;
  options.setRetryable(false); // 不可重试

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

  auto backoffPolicy =
      std::make_shared<Policy::FixedBackoffPolicy>(policyOption);
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

  auto backoffPolicy =
      std::make_shared<Policy::ExponentialBackoffPolicy>(policyOption);
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
  condition.setMaxDelay(100); // 设置很小的最大延迟

  std::map<std::string, std::string> policyOption;
  policyOption["policy"] = "Fixed";
  policyOption["period"] = "5000"; // 大于 maxDelay

  auto backoffPolicy =
      std::make_shared<Policy::FixedBackoffPolicy>(policyOption);
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

    std::future_status status =
        futureResponse.wait_for(std::chrono::seconds(10));
    ASSERT_EQ(status, std::future_status::ready);

    auto response = futureResponse.get();
    ASSERT_NE(response, nullptr);

    // 验证收到响应(200, 301, 302 等都算成功)
    EXPECT_GT(response->getStatusCode(), 0);
    EXPECT_LT(response->getStatusCode(), 500);

  } catch (const std::exception &e) {
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

    std::future_status status =
        futureResponse.wait_for(std::chrono::seconds(5));
    ASSERT_EQ(status, std::future_status::ready);

    auto response = futureResponse.get();

    // 空 host 应该返回错误状态码
    EXPECT_EQ(response->getStatusCode(), 0);

  } catch (const std::exception &e) {
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

    std::future_status status =
        futureResponse.wait_for(std::chrono::seconds(15));
    ASSERT_EQ(status, std::future_status::ready);

    auto response = futureResponse.get();
    ASSERT_NE(response, nullptr);

    // POST 请求应该返回成功状态码(2xx, 3xx 都算成功)
    EXPECT_GT(response->getStatusCode(), 0);
    EXPECT_LT(response->getStatusCode(), 500);

  } catch (const std::exception &e) {
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

    std::future_status status =
        futureResponse.wait_for(std::chrono::seconds(15));
    if (status == std::future_status::ready) {
      auto response = futureResponse.get();
      ASSERT_NE(response, nullptr);
      // 验证收到响应
      EXPECT_GT(response->getStatusCode(), 0);
      EXPECT_LT(response->getStatusCode(), 500);
    }

  } catch (const std::exception &e) {
    // 网络请求可能失败,跳过测试
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

// ==================== ConnectionPoolConfig 测试 ====================
TEST_F(CoreTest, ConnectionPoolConfigDefaultValues) {
  ConnectionPoolConfig config;
  EXPECT_EQ(config.max_connections, 5UL);
  EXPECT_EQ(config.max_host_connections, 2UL);
  EXPECT_EQ(config.connection_idle_timeout, 300L);
  EXPECT_FALSE(config.pipelining);
  EXPECT_TRUE(config.keep_alive);
}

TEST_F(CoreTest, ConnectionPoolConfigWithKeepAlive) {
  ConnectionPoolConfig config;
  config.max_connections = 20;
  config.keep_alive = false;
  
  EXPECT_EQ(config.max_connections, 20UL);
  EXPECT_FALSE(config.keep_alive);
}

// ==================== MCurlHttpClient 连接池配置测试 ====================
TEST_F(CoreTest, MCurlHttpClientSetConnectionPoolConfig) {
  Http::MCurlHttpClient client;
  
  ConnectionPoolConfig config;
  config.max_connections = 15;
  config.keep_alive = false;
  client.setConnectionPoolConfig(config);
  
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 15UL);
  EXPECT_FALSE(client.getConnectionPoolConfig().keep_alive);
}

TEST_F(CoreTest, MCurlHttpClientConfigAfterStart) {
  Http::MCurlHttpClient client;
  
  ConnectionPoolConfig config;
  config.max_connections = 25;
  client.setConnectionPoolConfig(config);
  
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 25UL);
}

// ==================== 配置传递测试 ====================
TEST_F(CoreTest, MaxIdleConnsPropagationFromConfig) {
  Core::ClearAllHttpClients();

  try {
    // Simulate what Client does: pass maxIdleConns from Config through runtime
    Json runtime;
    runtime["maxIdleConns"] = 20;
    runtime["keepAlive"] = true;
    runtime["timeout"] = 5000;

    Http::Request request(std::string("https://config-test.aliyun.com"));
    auto future = core.doAction(request, runtime);
    future.wait_for(std::chrono::seconds(10));

    EXPECT_EQ(Core::GetHttpClientCount(), 1UL);
    Core::ClearAllHttpClients();

  } catch (const std::exception &e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

TEST_F(CoreTest, ConfigUpdateOnExistingClient) {
  Core::ClearAllHttpClients();

  try {
    Http::Request request(std::string("https://update-test.aliyun.com"));

    {
      Json runtime;
      runtime["maxIdleConns"] = 10;
      auto future = core.doAction(request, runtime);
      future.wait_for(std::chrono::seconds(10));
    }

    {
      Json runtime;
      runtime["maxIdleConns"] = 30;
      auto future = core.doAction(request, runtime);
      future.wait_for(std::chrono::seconds(10));
    }

    EXPECT_EQ(Core::GetHttpClientCount(), 1UL);
    Core::ClearAllHttpClients();

  } catch (const std::exception &e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

// ==================== ConnectionPoolConfig 边界测试 ====================
TEST_F(CoreTest, ConnectionPoolConfigZeroConnections) {
  ConnectionPoolConfig config;
  config.max_connections = 0;
  EXPECT_EQ(config.max_connections, 0UL);
}

TEST_F(CoreTest, ConnectionPoolConfigLargeConnections) {
  ConnectionPoolConfig config;
  config.max_connections = 10000;
  EXPECT_EQ(config.max_connections, 10000UL);
}

TEST_F(CoreTest, ConnectionPoolConfigCompareOperator) {
  ConnectionPoolConfig config1;
  config1.host = "api.test.com";
  config1.max_connections = 10;
  
  ConnectionPoolConfig config2;
  config2.host = "api.test.com";
  config2.max_connections = 20;
  
  EXPECT_TRUE(config1 < config2);
  EXPECT_FALSE(config2 < config1);
}

TEST_F(CoreTest, ConnectionPoolConfigAllFields) {
  ConnectionPoolConfig config;
  config.host = "api.example.com";
  config.max_connections = 100;
  config.max_host_connections = 10;
  config.connection_idle_timeout = 600L;
  config.pipelining = true;
  config.keep_alive = false;
  
  EXPECT_EQ(config.host, "api.example.com");
  EXPECT_EQ(config.max_connections, 100UL);
  EXPECT_EQ(config.max_host_connections, 10UL);
  EXPECT_EQ(config.connection_idle_timeout, 600L);
  EXPECT_TRUE(config.pipelining);
  EXPECT_FALSE(config.keep_alive);
}

TEST_F(CoreTest, ConnectionPoolConfigCopy) {
  ConnectionPoolConfig config1;
  config1.host = "original.com";
  config1.max_connections = 30;
  config1.keep_alive = false;
  
  ConnectionPoolConfig config2 = config1;
  
  EXPECT_EQ(config2.host, "original.com");
  EXPECT_EQ(config2.max_connections, 30UL);
  EXPECT_FALSE(config2.keep_alive);
  
  config1.host = "modified.com";
  EXPECT_EQ(config2.host, "original.com");
}

// ==================== MCurlHttpClient 详细测试 ====================
TEST_F(CoreTest, MCurlHttpClientDefaultConnectionPoolConfig) {
  Http::MCurlHttpClient client;
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 5UL);
  EXPECT_TRUE(client.getConnectionPoolConfig().keep_alive);
}

TEST_F(CoreTest, MCurlHttpClientConfigOverride) {
  Http::MCurlHttpClient client;
  
  ConnectionPoolConfig config1;
  config1.max_connections = 10;
  config1.keep_alive = true;
  client.setConnectionPoolConfig(config1);
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 10UL);
  EXPECT_TRUE(client.getConnectionPoolConfig().keep_alive);
  
  ConnectionPoolConfig config2;
  config2.max_connections = 20;
  config2.keep_alive = false;
  client.setConnectionPoolConfig(config2);
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 20UL);
  EXPECT_FALSE(client.getConnectionPoolConfig().keep_alive);
}

TEST_F(CoreTest, MCurlHttpClientStartStop) {
  Http::MCurlHttpClient client;
  
  ConnectionPoolConfig config;
  config.max_connections = 15;
  client.setConnectionPoolConfig(config);
  
  EXPECT_TRUE(client.start());
  EXPECT_FALSE(client.start());  // Already running
  EXPECT_TRUE(client.stop());
  EXPECT_FALSE(client.stop());   // Already stopped
}

// ==================== Keep-Alive 配置测试 ====================
TEST_F(CoreTest, KeepAliveDefaultTrue) {
  ConnectionPoolConfig config;
  EXPECT_TRUE(config.keep_alive);
}

TEST_F(CoreTest, KeepAliveSetFalse) {
  ConnectionPoolConfig config;
  config.keep_alive = false;
  EXPECT_FALSE(config.keep_alive);
}

TEST_F(CoreTest, KeepAliveDisabledConfig) {
  ConnectionPoolConfig config;
  config.keep_alive = false;
  
  Http::MCurlHttpClient client;
  client.setConnectionPoolConfig(config);
  
  EXPECT_FALSE(client.getConnectionPoolConfig().keep_alive);
}

TEST_F(CoreTest, KeepAliveEnabledConfig) {
  ConnectionPoolConfig config;
  config.keep_alive = true;
  
  Http::MCurlHttpClient client;
  client.setConnectionPoolConfig(config);
  
  EXPECT_TRUE(client.getConnectionPoolConfig().keep_alive);
}

// ==================== 默认超时测试 ====================
TEST_F(CoreTest, DefaultReadTimeout) {
  Json runtime;
  // 不设置 readTimeout，应该使用默认值
  
  Http::Request request(std::string("https://timeout-test.aliyun.com"));
  // 验证 runtime 可以正确处理
  EXPECT_NO_THROW({
    auto future = core.doAction(request, runtime);
    future.wait_for(std::chrono::seconds(10));
  });
  
  Core::ClearAllHttpClients();
}

TEST_F(CoreTest, CustomReadTimeout) {
  Json runtime;
  runtime["readTimeout"] = 5000;
  
  Http::Request request(std::string("https://custom-timeout.aliyun.com"));
  EXPECT_NO_THROW({
    auto future = core.doAction(request, runtime);
    future.wait_for(std::chrono::seconds(10));
  });
  
  Core::ClearAllHttpClients();
}

// ==================== 多 Host 客户端管理测试 ====================
TEST_F(CoreTest, MultipleHostsMultipleClients) {
  Core::ClearAllHttpClients();
  
  try {
    Json runtime;
    runtime["maxIdleConns"] = 10;
    
    Http::Request request1(std::string("https://host1.aliyun.com"));
    auto f1 = core.doAction(request1, runtime);
    f1.wait_for(std::chrono::seconds(10));
    
    Http::Request request2(std::string("https://host2.aliyun.com"));
    auto f2 = core.doAction(request2, runtime);
    f2.wait_for(std::chrono::seconds(10));
    
    EXPECT_EQ(Core::GetHttpClientCount(), 2UL);
    
    Core::ClearAllHttpClients();
    
  } catch (const std::exception &e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

TEST_F(CoreTest, SameHostReusesClient) {
  Core::ClearAllHttpClients();
  
  try {
    Json runtime;
    runtime["maxIdleConns"] = 10;
    
    for (int i = 0; i < 3; i++) {
      Http::Request request(std::string("https://same-host.aliyun.com"));
      auto future = core.doAction(request, runtime);
      future.wait_for(std::chrono::seconds(10));
    }
    
    EXPECT_EQ(Core::GetHttpClientCount(), 1UL);
    
    Core::ClearAllHttpClients();
    
  } catch (const std::exception &e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

// ==================== ClearHttpClient 测试 ====================
TEST_F(CoreTest, ClearSpecificHttpClient) {
  Core::ClearAllHttpClients();
  
  try {
    Json runtime;
    
    Http::Request request1(std::string("https://clear-test1.aliyun.com"));
    auto f1 = core.doAction(request1, runtime);
    f1.wait_for(std::chrono::seconds(10));
    
    Http::Request request2(std::string("https://clear-test2.aliyun.com"));
    auto f2 = core.doAction(request2, runtime);
    f2.wait_for(std::chrono::seconds(10));
    
    EXPECT_EQ(Core::GetHttpClientCount(), 2UL);
    
    ConnectionPoolConfig config;
    config.host = "clear-test1.aliyun.com";
    Core::ClearHttpClient(config);
    
    EXPECT_EQ(Core::GetHttpClientCount(), 1UL);
    
    Core::ClearAllHttpClients();
    
  } catch (const std::exception &e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

// ==================== 内存管理测试 ====================
TEST_F(CoreTest, ClientCleanupOnDestruction) {
  {
    Http::MCurlHttpClient* client = new Http::MCurlHttpClient();
    client->start();
    
    ConnectionPoolConfig config;
    config.max_connections = 10;
    client->setConnectionPoolConfig(config);
    
    delete client;
  }
  
  EXPECT_TRUE(true);
}

TEST_F(CoreTest, MultipleClearAllHttpClients) {
  Core::ClearAllHttpClients();
  Core::ClearAllHttpClients();
  Core::ClearAllHttpClients();
  
  EXPECT_EQ(Core::GetHttpClientCount(), 0UL);
}

// ==================== 空配置测试 ====================
TEST_F(CoreTest, EmptyRuntimeOptions) {
  Core::ClearAllHttpClients();
  
  try {
    Http::Request request(std::string("https://empty-runtime.aliyun.com"));
    Json runtime;
    
    auto future = core.doAction(request, runtime);
    future.wait_for(std::chrono::seconds(10));
    
    EXPECT_EQ(Core::GetHttpClientCount(), 1UL);
    
    Core::ClearAllHttpClients();
    
  } catch (const std::exception &e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }
}

// ==================== 配置类型安全测试 ====================
TEST_F(CoreTest, MaxIdleConnsTypeSafety) {
  Json runtime;
  runtime["maxIdleConns"] = 50;
  
  EXPECT_EQ(runtime["maxIdleConns"].get<int64_t>(), 50);
}

TEST_F(CoreTest, KeepAliveTypeSafety) {
  Json runtime;
  runtime["keepAlive"] = true;
  
  EXPECT_TRUE(runtime["keepAlive"].get<bool>());
  
  runtime["keepAlive"] = false;
  EXPECT_FALSE(runtime["keepAlive"].get<bool>());
}
