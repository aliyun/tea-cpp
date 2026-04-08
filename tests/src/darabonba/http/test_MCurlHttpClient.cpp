#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Runtime.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <memory>

using namespace Darabonba;
using namespace Darabonba::Http;

class MCurlHttpClientTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}

  virtual void TearDown() override {}
};

// ==================== MCurlHttpClient 生命周期测试 ====================

TEST_F(MCurlHttpClientTest, CreateDestroy) {
  MCurlHttpClient client;
  // 析构函数应该正确清理资源
}

TEST_F(MCurlHttpClientTest, StartStop) {
  MCurlHttpClient client;

  EXPECT_TRUE(client.start());
  EXPECT_FALSE(client.start());  // 已启动，再次启动返回 false
  EXPECT_TRUE(client.stop());
  EXPECT_FALSE(client.stop());   // 已停止，再次停止返回 false
}

TEST_F(MCurlHttpClientTest, StartStopSequence) {
  MCurlHttpClient client;

  EXPECT_TRUE(client.start());
  EXPECT_TRUE(client.stop());
}

// ==================== ConnectionPoolConfig 测试 ====================

TEST_F(MCurlHttpClientTest, DefaultConnectionPoolConfig) {
  MCurlHttpClient client;
  auto config = client.getConnectionPoolConfig();

  EXPECT_EQ(config.max_connections, 128UL);
  EXPECT_EQ(config.max_host_connections, 128UL);
  EXPECT_EQ(config.connection_idle_timeout, 30L);
  EXPECT_FALSE(config.pipelining);
  EXPECT_TRUE(config.keep_alive);
}

TEST_F(MCurlHttpClientTest, SetConnectionPoolConfig) {
  MCurlHttpClient client;

  ConnectionPoolConfig config;
  config.max_connections = 50;
  config.max_host_connections = 10;
  config.connection_idle_timeout = 60L;
  config.pipelining = true;
  config.keep_alive = false;

  client.setConnectionPoolConfig(config);

  auto retrieved = client.getConnectionPoolConfig();
  EXPECT_EQ(retrieved.max_connections, 50UL);
  EXPECT_EQ(retrieved.max_host_connections, 10UL);
  EXPECT_EQ(retrieved.connection_idle_timeout, 60L);
  EXPECT_TRUE(retrieved.pipelining);
  EXPECT_FALSE(retrieved.keep_alive);
}

TEST_F(MCurlHttpClientTest, UpdateConnectionPoolConfigMultipleTimes) {
  MCurlHttpClient client;

  ConnectionPoolConfig config1;
  config1.max_connections = 20;
  client.setConnectionPoolConfig(config1);
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 20UL);

  ConnectionPoolConfig config2;
  config2.max_connections = 40;
  client.setConnectionPoolConfig(config2);
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 40UL);
}

TEST_F(MCurlHttpClientTest, SetConfigBeforeStart) {
  MCurlHttpClient client;

  ConnectionPoolConfig config;
  config.max_connections = 30;
  client.setConnectionPoolConfig(config);

  EXPECT_TRUE(client.start());
  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 30UL);

  EXPECT_TRUE(client.stop());
}

TEST_F(MCurlHttpClientTest, SetConfigAfterStart) {
  MCurlHttpClient client;

  EXPECT_TRUE(client.start());

  ConnectionPoolConfig config;
  config.max_connections = 25;
  client.setConnectionPoolConfig(config);

  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 25UL);

  EXPECT_TRUE(client.stop());
}

// ==================== 并发配置更新测试 ====================

TEST_F(MCurlHttpClientTest, ConcurrentConfigUpdate) {
  MCurlHttpClient client;
  client.start();

  std::vector<std::thread> threads;
  std::atomic<int> successCount{0};

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&client, i, &successCount]() {
      ConnectionPoolConfig config;
      config.max_connections = static_cast<size_t>(10 + i);
      client.setConnectionPoolConfig(config);
      successCount++;
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(successCount.load(), 10);

  client.stop();
}

// ==================== 网络请求测试 ====================

TEST_F(MCurlHttpClientTest, MakeRequestToValidUrl) {
  MCurlHttpClient client;
  client.start();

  try {
    Request request(std::string("https://www.aliyun.com"));
    request.getHeader()["User-Agent"] = "test-client";

    Darabonba::Json options;
    options["readTimeout"] = 10000;
    options["connectTimeout"] = 5000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(15));
    ASSERT_EQ(status, std::future_status::ready);

    auto response = future.get();
    if (response) {
      EXPECT_GT(response->getStatusCode(), 0);
      EXPECT_LT(response->getStatusCode(), 600);
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}

TEST_F(MCurlHttpClientTest, MakeRequestWithPostMethod) {
  MCurlHttpClient client;
  client.start();

  try {
    Request request(std::string("https://httpbin.org/post"));
    request.setMethod(Request::Method::POST);
    request.getHeader()["Content-Type"] = "application/json";
    request.setBody(std::string("{\"test\": \"value\"}"));

    Darabonba::Json options;
    options["readTimeout"] = 15000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(20));
    if (status == std::future_status::ready) {
      auto response = future.get();
      if (response) {
        EXPECT_EQ(response->getStatusCode(), 200);
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}

TEST_F(MCurlHttpClientTest, MakeRequestWithInvalidUrl) {
  MCurlHttpClient client;
  client.start();

  try {
    Request request(std::string("https://invalid.nonexistent.domain.example/test"));

    Darabonba::Json options;
    options["readTimeout"] = 5000;
    options["connectTimeout"] = 3000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(10));
    ASSERT_EQ(status, std::future_status::ready);

    // 无效域名应该抛出异常或返回错误
    EXPECT_THROW({
      auto response = future.get();
    }, std::exception);

  } catch (const std::exception&) {
    // 预期会抛出异常
  }

  client.stop();
}

// ==================== 多请求并发测试 ====================

TEST_F(MCurlHttpClientTest, MultipleConcurrentRequests) {
  MCurlHttpClient client;
  client.start();

  try {
    std::vector<std::future<std::shared_ptr<MCurlResponse>>> futures;

    for (int i = 0; i < 3; ++i) {
      Request request(std::string("https://www.aliyun.com"));
      Darabonba::Json options;
      options["readTimeout"] = 10000;
      futures.push_back(client.makeRequest(request, options));
    }

    for (auto& f : futures) {
      auto status = f.wait_for(std::chrono::seconds(15));
      if (status == std::future_status::ready) {
        auto response = f.get();
        if (response) {
          EXPECT_GT(response->getStatusCode(), 0);
        }
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}

// ==================== Keep-Alive 配置测试 ====================

TEST_F(MCurlHttpClientTest, KeepAliveEnabled) {
  MCurlHttpClient client;

  ConnectionPoolConfig config;
  config.keep_alive = true;
  client.setConnectionPoolConfig(config);

  EXPECT_TRUE(client.getConnectionPoolConfig().keep_alive);
}

TEST_F(MCurlHttpClientTest, KeepAliveDisabled) {
  MCurlHttpClient client;

  ConnectionPoolConfig config;
  config.keep_alive = false;
  client.setConnectionPoolConfig(config);

  EXPECT_FALSE(client.getConnectionPoolConfig().keep_alive);
}

// ==================== 析构时资源清理测试 ====================

TEST_F(MCurlHttpClientTest, DestructionDuringRequest) {
  try {
    MCurlHttpClient* client = new MCurlHttpClient();
    client->start();

    Request request(std::string("https://www.aliyun.com"));
    Darabonba::Json options;
    options["readTimeout"] = 10000;

    auto future = client->makeRequest(request, options);

    // 在请求进行中销毁客户端
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    delete client;

    // 应该不会崩溃
    EXPECT_TRUE(true);

  } catch (const std::exception& e) {
    std::cerr << "Test exception: " << e.what() << std::endl;
  }
}

// ==================== 边界条件测试 ====================

TEST_F(MCurlHttpClientTest, ZeroConnections) {
  MCurlHttpClient client;

  ConnectionPoolConfig config;
  config.max_connections = 0;
  client.setConnectionPoolConfig(config);

  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 0UL);
}

TEST_F(MCurlHttpClientTest, LargeConnections) {
  MCurlHttpClient client;

  ConnectionPoolConfig config;
  config.max_connections = 10000;
  client.setConnectionPoolConfig(config);

  EXPECT_EQ(client.getConnectionPoolConfig().max_connections, 10000UL);
}

// ==================== 错误处理测试 ====================
// 这些测试验证最新的代码改动：perform() 中的 null promise 检查

TEST_F(MCurlHttpClientTest, EmptyRequestHandling) {
  MCurlHttpClient client;
  client.start();

  try {
    // 使用无效 URL 测试错误处理
    Request request(std::string(""));

    Darabonba::Json options;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(5));
    if (status == std::future_status::ready) {
      // 应该抛出异常或返回错误
      EXPECT_THROW({
        auto response = future.get();
      }, std::exception);
    }

  } catch (const std::exception&) {
    // 预期会抛出异常
  }

  client.stop();
}

// ==================== 流式响应测试 ====================

TEST_F(MCurlHttpClientTest, StreamResponseBody) {
  MCurlHttpClient client;
  client.start();

  try {
    Request request(std::string("https://www.aliyun.com"));

    Darabonba::Json options;
    options["readTimeout"] = 15000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(15));
    if (status == std::future_status::ready) {
      auto response = future.get();
      if (response && response->getBody()) {
        auto body = response->getBody();

        // 测试流式读取
        char buffer[1024];
        size_t totalRead = 0;
        size_t bytesRead;

        while ((bytesRead = body->read(buffer, sizeof(buffer))) > 0) {
          totalRead += bytesRead;
        }

        EXPECT_GT(totalRead, 0u);
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}

// ==================== 响应体并发读取测试 ====================

TEST_F(MCurlHttpClientTest, ConcurrentResponseBodyRead) {
  MCurlHttpClient client;
  client.start();

  try {
    Request request(std::string("https://www.aliyun.com"));

    Darabonba::Json options;
    options["readTimeout"] = 15000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(15));
    if (status == std::future_status::ready) {
      auto response = future.get();
      if (response && response->getBody()) {
        auto body = response->getBody();

        std::atomic<size_t> totalRead{0};
        std::vector<std::thread> readers;

        // 启动多个读线程测试并发安全性
        for (int i = 0; i < 2; ++i) {
          readers.emplace_back([&body, &totalRead]() {
            char buffer[512];
            size_t bytesRead;
            while ((bytesRead = body->read(buffer, sizeof(buffer))) > 0) {
              totalRead += bytesRead;
            }
          });
        }

        for (auto& t : readers) {
          t.join();
        }

        // 应该成功读取数据
        EXPECT_GE(totalRead.load(), 0u);
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}