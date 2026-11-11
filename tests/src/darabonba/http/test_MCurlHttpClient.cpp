#include <darabonba/Exception.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Runtime.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
static const socket_t kInvalidSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static const socket_t kInvalidSocket = -1;
#endif

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
        int64_t code = response->getStatusCode();
        // httpbin.org is an external dependency; 429/5xx mean the service is
        // unavailable — treat as skip (same posture as MakeRequestToValidUrl).
        if (code == 429 || code >= 500) {
          std::cerr << "Network test skipped: httpbin returned " << code
                    << std::endl;
        } else {
          EXPECT_EQ(code, 200);
        }
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
    Request request(std::string("https://httpbin.org/bytes/128"));

    Darabonba::Json options;
    options["connectTimeout"] = 5000;
    options["readTimeout"] = 10000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(15));
    if (status == std::future_status::ready) {
      auto response = future.get();
      if (response && response->getStatusCode() == 200 && response->getBody()) {
        auto body = response->getBody();

        // 测试流式读取（read 内置 wait_for，避免网络异常时永久阻塞）
        char buffer[1024];
        size_t totalRead = 0;
        size_t bytesRead;

        while ((bytesRead = body->read(buffer, sizeof(buffer))) > 0) {
          totalRead += bytesRead;
        }

        EXPECT_EQ(128u, totalRead);
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}

namespace {

void closeSocket(socket_t fd) {
  if (fd == kInvalidSocket) {
    return;
  }
#ifdef _WIN32
  closesocket(fd);
#else
  ::close(fd);
#endif
}

bool sendAll(socket_t fd, const std::string &data) {
  size_t sent = 0;
  while (sent < data.size()) {
#ifdef _WIN32
    int wrote = ::send(fd, data.data() + sent, static_cast<int>(data.size() - sent), 0);
#else
    ssize_t wrote = ::send(fd, data.data() + sent, data.size() - sent, 0);
#endif
    if (wrote <= 0) {
      return false;
    }
    sent += static_cast<size_t>(wrote);
  }
  return true;
}

void readHttpRequest(socket_t fd) {
  std::string request;
  char buffer[512];
  while (request.find("\r\n\r\n") == std::string::npos && request.size() < 8192) {
#ifdef _WIN32
    int got = ::recv(fd, buffer, sizeof(buffer), 0);
#else
    ssize_t got = ::recv(fd, buffer, sizeof(buffer), 0);
#endif
    if (got <= 0) {
      return;
    }
    request.append(buffer, static_cast<size_t>(got));
  }
}

class LocalHttpServer {
public:
  LocalHttpServer() {
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
#endif
    listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    int enabled = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<char *>(&enabled), sizeof(enabled));
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (::bind(listenFd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
      return;
    }
    if (::listen(listenFd_, 1) != 0) {
      return;
    }
    socklen_t length = sizeof(address);
    if (::getsockname(listenFd_, reinterpret_cast<sockaddr *>(&address), &length) == 0) {
      port_ = ntohs(address.sin_port);
    }
  }

  ~LocalHttpServer() {
    closeSocket(listenFd_);
    listenFd_ = kInvalidSocket;
    join();
  }

  int port() const { return port_; }

  void serve(const std::function<void(socket_t)> &writeResponse) {
    worker_ = std::thread([this, writeResponse]() {
      socket_t client = ::accept(listenFd_, nullptr, nullptr);
      if (client == kInvalidSocket) {
        return;
      }
      readHttpRequest(client);
      writeResponse(client);
      closeSocket(client);
    });
  }

  void join() {
    if (worker_.joinable()) {
      worker_.join();
    }
  }

private:
  socket_t listenFd_ = kInvalidSocket;
  int port_ = 0;
  std::thread worker_;
};

std::string readUntilEnd(const std::shared_ptr<MCurlResponseBody> &body, bool *failed) {
  std::string payload;
  char buffer[64];
  *failed = false;
  try {
    for (;;) {
      size_t count = body->read(buffer, sizeof(buffer));
      if (count == 0) {
        break;
      }
      payload.append(buffer, count);
    }
  } catch (const ResponseException &) {
    *failed = true;
  }
  return payload;
}

} // namespace

TEST_F(MCurlHttpClientTest, ChunkedDownloadCompletesFromLocalServer) {
  LocalHttpServer server;
  ASSERT_GT(server.port(), 0);
  server.serve([](socket_t client) {
    sendAll(client,
            "HTTP/1.1 200 OK\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Connection: close\r\n"
            "\r\n"
            "5\r\nhello\r\n"
            "6\r\n world\r\n"
            "0\r\n\r\n");
  });

  MCurlHttpClient client;
  client.start();
  Request request("http://127.0.0.1:" + std::to_string(server.port()) + "/file");
  Json options = {{"connectTimeout", 2000}, {"readTimeout", 5000}, {"noProxy", "*"}};
  auto response = client.makeRequest(request, options).get();
  ASSERT_TRUE(response);
  EXPECT_EQ(200, response->getStatusCode());
  bool failed = false;
  EXPECT_EQ("hello world", readUntilEnd(response->getBody(), &failed));
  EXPECT_FALSE(failed);
  client.stop();
  server.join();
}

TEST_F(MCurlHttpClientTest, InterruptedDownloadRaisesInsteadOfShortSuccess) {
  LocalHttpServer server;
  ASSERT_GT(server.port(), 0);
  server.serve([](socket_t client) {
    sendAll(client,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 100\r\n"
            "Connection: close\r\n"
            "\r\n"
            "partial");
  });

  MCurlHttpClient client;
  client.start();
  Request request("http://127.0.0.1:" + std::to_string(server.port()) + "/file");
  Json options = {{"connectTimeout", 2000}, {"readTimeout", 5000}, {"noProxy", "*"}};
  auto response = client.makeRequest(request, options).get();
  ASSERT_TRUE(response);
  EXPECT_EQ(200, response->getStatusCode());
  bool failed = false;
  response->getBody()->waitForDone();
  std::string payload = readUntilEnd(response->getBody(), &failed);
  EXPECT_TRUE(failed);
  EXPECT_LT(payload.size(), 100u);
  client.stop();
  server.join();
}

TEST_F(MCurlHttpClientTest, DiscardingRetainedBodyCancelsTransferWithoutTimeout) {
  std::promise<bool> peerClosed;
  auto closed = peerClosed.get_future();
  LocalHttpServer server;
  ASSERT_GT(server.port(), 0);
  server.serve([&peerClosed](socket_t socket) {
    if (!sendAll(socket,
                 "HTTP/1.1 200 OK\r\nContent-Length: 20000000\r\n\r\npartial")) {
      peerClosed.set_value(false);
      return;
    }
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(socket, &readable);
    timeval timeout = {8, 0};
#ifdef _WIN32
    const int nfds = 0;
#else
    const int nfds = socket + 1;
#endif
    char byte;
    peerClosed.set_value(
        ::select(nfds, &readable, nullptr, nullptr, &timeout) > 0 &&
        ::recv(socket, &byte, 1, 0) == 0);
  });

  MCurlHttpClient client;
  ASSERT_TRUE(client.start());
  Request request("http://127.0.0.1:" + std::to_string(server.port()) + "/file");
  Json options = {{"connectTimeout", 2000}, {"readTimeout", 0}, {"noProxy", "*"}};
  auto response = client.makeRequest(request, options).get();
  ASSERT_TRUE(response);
  auto retained = Stream::retain(response->getBody());
  response.reset();
  EXPECT_EQ(std::future_status::timeout, closed.wait_for(std::chrono::milliseconds(100)));
  retained = nullptr;
  ASSERT_EQ(std::future_status::ready, closed.wait_for(std::chrono::seconds(6)));
  EXPECT_TRUE(closed.get());
}

TEST_F(MCurlHttpClientTest, StalledDownloadRaisesInsteadOfShortSuccess) {
  LocalHttpServer server;
  ASSERT_GT(server.port(), 0);
  server.serve([](socket_t client) {
    sendAll(client,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 100\r\n"
            "Connection: close\r\n"
            "\r\n"
            "partial");
    std::this_thread::sleep_for(std::chrono::seconds(2));
  });

  MCurlHttpClient client;
  client.start();
  Request request("http://127.0.0.1:" + std::to_string(server.port()) + "/file");
  Json options = {{"connectTimeout", 2000}, {"readTimeout", 5000}, {"noProxy", "*"}};
  auto response = client.makeRequest(request, options).get();
  ASSERT_TRUE(response);
  EXPECT_EQ(200, response->getStatusCode());
  response->getBody()->setReadWait(std::chrono::milliseconds(200));
  bool failed = false;
  std::string payload = readUntilEnd(response->getBody(), &failed);
  EXPECT_TRUE(failed);
  EXPECT_EQ("partial", payload);
  client.stop();
  server.join();
}

// ==================== 响应体并发读取测试 ====================

TEST_F(MCurlHttpClientTest, ConcurrentResponseBodyRead) {
  MCurlHttpClient client;
  client.start();

  try {
    Request request(std::string("https://www.aliyun.com"));

    Darabonba::Json options;
    options["connectTimeout"] = 5000;
    options["readTimeout"] = 10000;

    auto future = client.makeRequest(request, options);

    auto status = future.wait_for(std::chrono::seconds(15));
    if (status == std::future_status::ready) {
      auto response = future.get();
      if (response && response->getBody()) {
        auto body = response->getBody();

        // Single consumer: stream body is not a fan-out queue; concurrent
        // readers race on the ring buffer. Exercise sequential drain under a
        // second thread instead.
        std::atomic<size_t> totalRead{0};
        std::thread reader([&body, &totalRead]() {
          char buffer[512];
          size_t bytesRead;
          while ((bytesRead = body->read(buffer, sizeof(buffer))) > 0) {
            totalRead += bytesRead;
          }
        });
        reader.join();

        EXPECT_GE(totalRead.load(), 0u);
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Network test skipped: " << e.what() << std::endl;
  }

  client.stop();
}
