#include <darabonba/Runtime.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace Darabonba;

class RuntimeOptionsTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}

  virtual void TearDown() override {}
};

// ==================== maxIdleConns 字段测试 ====================
// 测试新增的 maxIdleConns getter/setter

TEST_F(RuntimeOptionsTest, MaxIdleConnsDefaultEmpty) {
  RuntimeOptions options;
  // 默认值应该是空字符串（因为 maxIdleConns_ 初始化为 nullptr 的字符串）
  EXPECT_TRUE(options.getMaxIdleConns().empty());
}

TEST_F(RuntimeOptionsTest, MaxIdleConnsSetterGetter) {
  RuntimeOptions options;
  options.setMaxIdleConns("100");

  EXPECT_EQ(options.getMaxIdleConns(), "100");
}

TEST_F(RuntimeOptionsTest, MaxIdleConnsSetEmpty) {
  RuntimeOptions options;
  options.setMaxIdleConns("");
  EXPECT_TRUE(options.getMaxIdleConns().empty());
}

TEST_F(RuntimeOptionsTest, MaxIdleConnsOverwrite) {
  RuntimeOptions options;
  options.setMaxIdleConns("50");
  EXPECT_EQ(options.getMaxIdleConns(), "50");

  options.setMaxIdleConns("200");
  EXPECT_EQ(options.getMaxIdleConns(), "200");
}

TEST_F(RuntimeOptionsTest, MaxIdleConnsLargeValue) {
  RuntimeOptions options;
  options.setMaxIdleConns("10000");
  EXPECT_EQ(options.getMaxIdleConns(), "10000");
}

TEST_F(RuntimeOptionsTest, MaxIdleConnsStringValue) {
  RuntimeOptions options;
  // 测试字符串类型存储
  options.setMaxIdleConns("invalid_number");
  EXPECT_EQ(options.getMaxIdleConns(), "invalid_number");
}

// ==================== 其他 RuntimeOptions 字段测试 ====================

TEST_F(RuntimeOptionsTest, AllTimeoutFields) {
  RuntimeOptions options;
  options.setReadTimeout(5000);
  options.setConnectTimeout(3000);

  EXPECT_EQ(options.getReadTimeout(), 5000);
  EXPECT_EQ(options.getConnectTimeout(), 3000);
}

TEST_F(RuntimeOptionsTest, HttpProxySettings) {
  RuntimeOptions options;
  options.setHttpProxy("http://proxy.example.com:8080");
  options.setHttpsProxy("https://proxy.example.com:8443");
  options.setNoProxy("localhost,127.0.0.1");

  EXPECT_EQ(options.getHttpProxy(), "http://proxy.example.com:8080");
  EXPECT_EQ(options.getHttpsProxy(), "https://proxy.example.com:8443");
  EXPECT_EQ(options.getNoProxy(), "localhost,127.0.0.1");
}

TEST_F(RuntimeOptionsTest, Socks5ProxySettings) {
  RuntimeOptions options;
  options.setSocks5Proxy("socks5://proxy.example.com:1080");
  options.setSocks5NetWork("tcp");

  EXPECT_EQ(options.getSocks5Proxy(), "socks5://proxy.example.com:1080");
  EXPECT_EQ(options.getSocks5NetWork(), "tcp");
}

TEST_F(RuntimeOptionsTest, SslSettings) {
  RuntimeOptions options;
  options.setKey("/path/to/key.pem");
  options.setCert("/path/to/cert.pem");
  options.setCa("/path/to/ca.pem");
  options.setIgnoreSSL(true);

  EXPECT_EQ(options.getKey(), "/path/to/key.pem");
  EXPECT_EQ(options.getCert(), "/path/to/cert.pem");
  EXPECT_EQ(options.getCa(), "/path/to/ca.pem");
  EXPECT_TRUE(options.getIgnoreSSL());
}

TEST_F(RuntimeOptionsTest, ConnectionSettings) {
  RuntimeOptions options;
  options.setMaxConnections(100);
  options.setMaxHostConnections(10);
  options.setKeepAlive(true);
  options.setLocalAddr("192.168.1.100");

  EXPECT_EQ(options.getMaxConnections(), 100);
  EXPECT_EQ(options.getMaxHostConnections(), 10);
  EXPECT_TRUE(options.getKeepAlive());
  EXPECT_EQ(options.getLocalAddr(), "192.168.1.100");
}

TEST_F(RuntimeOptionsTest, BackoffSettings) {
  RuntimeOptions options;
  options.setMaxAttempts(5);
  options.setBackoffPolicy("Exponential");
  options.setBackoffPeriod(1000);

  EXPECT_EQ(options.getMaxAttempts(), 5);
  EXPECT_EQ(options.getBackoffPolicy(), "Exponential");
  EXPECT_EQ(options.getBackoffPeriod(), 1000);
}

TEST_F(RuntimeOptionsTest, AutoretrySetting) {
  RuntimeOptions options;
  options.setAutoretry(true);
  EXPECT_TRUE(options.getAutoretry());

  options.setAutoretry(false);
  EXPECT_FALSE(options.getAutoretry());
}

// ==================== ExtendsParameters 测试 ====================

TEST_F(RuntimeOptionsTest, ExtendsParametersSetGet) {
  RuntimeOptions options;
  ExtendsParameters params;

  std::map<std::string, std::string> headers;
  headers["X-Custom-Header"] = "value";
  params.setHeaders(headers);

  std::map<std::string, std::string> queries;
  queries["param"] = "query_value";
  params.setQueries(queries);

  options.setExtendsParameters(params);

  EXPECT_TRUE(options.hasExtendsParameters());
  EXPECT_EQ(options.getExtendsParameters().getHeaders().at("X-Custom-Header"), "value");
  EXPECT_EQ(options.getExtendsParameters().getQueries().at("param"), "query_value");
}

TEST_F(RuntimeOptionsTest, ExtendsParametersDelete) {
  RuntimeOptions options;
  ExtendsParameters params;
  params.setHeaders({{"X-Test", "value"}});
  options.setExtendsParameters(params);

  EXPECT_TRUE(options.hasExtendsParameters());
  options.deleteExtendsParameters();
  EXPECT_FALSE(options.hasExtendsParameters());
}

// ==================== RetryOptions 测试 ====================

TEST_F(RuntimeOptionsTest, RetryOptionsSetGet) {
  RuntimeOptions options;
  Policy::RetryOptions retryOptions;
  retryOptions.setRetryable(true);

  options.setRetryOptions(retryOptions);

  EXPECT_TRUE(options.hasRetryOptions());
  EXPECT_TRUE(options.getRetryOptions().isRetryable());
}

TEST_F(RuntimeOptionsTest, RetryOptionsDelete) {
  RuntimeOptions options;
  Policy::RetryOptions retryOptions;
  retryOptions.setRetryable(true);
  options.setRetryOptions(retryOptions);

  EXPECT_TRUE(options.hasRetryOptions());
  options.deleteRetryOptions();
  EXPECT_FALSE(options.hasRetryOptions());
}

// ==================== JSON 序列化测试 ====================

TEST_F(RuntimeOptionsTest, JsonSerializationBasicFields) {
  RuntimeOptions options;
  options.setReadTimeout(5000);
  options.setConnectTimeout(3000);
  options.setIgnoreSSL(true);
  options.setHttpProxy("http://proxy:8080");

  Json j;
  to_json(j, options);

  EXPECT_EQ(j["readTimeout"], 5000);
  EXPECT_EQ(j["connectTimeout"], 3000);
  EXPECT_EQ(j["ignoreSSL"], true);
  EXPECT_EQ(j["httpProxy"], "http://proxy:8080");
}

TEST_F(RuntimeOptionsTest, JsonDeserializationBasicFields) {
  Json j;
  j["readTimeout"] = 5000;
  j["connectTimeout"] = 3000;
  j["ignoreSSL"] = true;
  j["httpProxy"] = "http://proxy:8080";

  RuntimeOptions options;
  from_json(j, options);

  EXPECT_EQ(options.getReadTimeout(), 5000);
  EXPECT_EQ(options.getConnectTimeout(), 3000);
  EXPECT_TRUE(options.getIgnoreSSL());
  EXPECT_EQ(options.getHttpProxy(), "http://proxy:8080");
}

TEST_F(RuntimeOptionsTest, JsonRoundTrip) {
  RuntimeOptions original;
  original.setReadTimeout(5000);
  original.setConnectTimeout(3000);
  original.setIgnoreSSL(true);
  original.setMaxConnections(100);
  original.setKeepAlive(true);

  Json j;
  to_json(j, original);

  RuntimeOptions restored;
  from_json(j, restored);

  EXPECT_EQ(restored.getReadTimeout(), original.getReadTimeout());
  EXPECT_EQ(restored.getConnectTimeout(), original.getConnectTimeout());
  EXPECT_EQ(restored.getIgnoreSSL(), original.getIgnoreSSL());
  EXPECT_EQ(restored.getMaxConnections(), original.getMaxConnections());
  EXPECT_EQ(restored.getKeepAlive(), original.getKeepAlive());
}

// ==================== empty() 方法测试 ====================

TEST_F(RuntimeOptionsTest, EmptyWhenDefault) {
  RuntimeOptions options;
  EXPECT_TRUE(options.empty());
}

TEST_F(RuntimeOptionsTest, NotEmptyWhenFieldSet) {
  RuntimeOptions options;
  options.setReadTimeout(5000);
  EXPECT_FALSE(options.empty());
}

TEST_F(RuntimeOptionsTest, EmptyAfterClear) {
  RuntimeOptions options;
  options.setReadTimeout(5000);
  options.deleteReadTimeout();
  EXPECT_TRUE(options.empty());
}

// ==================== 边界情况测试 ====================

TEST_F(RuntimeOptionsTest, ZeroTimeoutValues) {
  RuntimeOptions options;
  options.setReadTimeout(0);
  options.setConnectTimeout(0);

  EXPECT_EQ(options.getReadTimeout(), 0);
  EXPECT_EQ(options.getConnectTimeout(), 0);
}

TEST_F(RuntimeOptionsTest, LargeTimeoutValues) {
  RuntimeOptions options;
  options.setReadTimeout(INT64_MAX);
  options.setConnectTimeout(INT64_MAX);

  EXPECT_EQ(options.getReadTimeout(), INT64_MAX);
  EXPECT_EQ(options.getConnectTimeout(), INT64_MAX);
}

TEST_F(RuntimeOptionsTest, NegativeValues) {
  RuntimeOptions options;
  // 虽然 API 可能不接受负值，但类型应该能存储
  options.setReadTimeout(-1);
  EXPECT_EQ(options.getReadTimeout(), -1);
}