#include <darabonba/Core.hpp>
#include <darabonba/Exception.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/Request.hpp>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace Darabonba;
using namespace Darabonba::Http;

// ==================== Curl::setCurlIpv4Only 测试 ====================

TEST(Ipv4OnlyCurlOptionTest, NotSetWhenFalse) {
  CURL *curl = curl_easy_init();
  ASSERT_NE(curl, nullptr);
  EXPECT_FALSE(Curl::setCurlIpv4Only(curl, false));
  curl_easy_cleanup(curl);
}

TEST(Ipv4OnlyCurlOptionTest, SetWhenTrue) {
  CURL *curl = curl_easy_init();
  ASSERT_NE(curl, nullptr);
  EXPECT_TRUE(Curl::setCurlIpv4Only(curl, true));
  curl_easy_cleanup(curl);
}

TEST(Ipv4OnlyCurlOptionTest, NullHandle) {
  EXPECT_FALSE(Curl::setCurlIpv4Only(nullptr, true));
}

#ifndef _WIN32

namespace {

// Minimal keep-alive HTTP/1.1 server bound to a single loopback address.
class LoopbackHttpServer {
public:
  ~LoopbackHttpServer() { stop(); }

  bool start(int family) {
    fd_ = ::socket(family, SOCK_STREAM, 0);
    if (fd_ < 0) {
      return false;
    }
    int on = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    int rc;
    if (family == AF_INET6) {
      ::setsockopt(fd_, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
      sockaddr_in6 addr{};
      addr.sin6_family = AF_INET6;
      addr.sin6_addr = in6addr_loopback;
      rc = ::bind(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
      socklen_t len = sizeof(addr);
      if (rc == 0 &&
          ::getsockname(fd_, reinterpret_cast<sockaddr *>(&addr), &len) == 0) {
        port_ = ntohs(addr.sin6_port);
      }
    } else {
      sockaddr_in addr{};
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      rc = ::bind(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
      socklen_t len = sizeof(addr);
      if (rc == 0 &&
          ::getsockname(fd_, reinterpret_cast<sockaddr *>(&addr), &len) == 0) {
        port_ = ntohs(addr.sin_port);
      }
    }
    if (rc != 0 || port_ == 0 || ::listen(fd_, 16) != 0) {
      ::close(fd_);
      fd_ = -1;
      return false;
    }
    running_ = true;
    acceptThread_ = std::thread([this]() { acceptLoop(); });
    return true;
  }

  void stop() {
    if (!running_.exchange(false)) {
      return;
    }
    acceptThread_.join();
    {
      std::lock_guard<std::mutex> guard(mutex_);
      for (int fd : clientFds_) {
        ::shutdown(fd, SHUT_RDWR);
      }
    }
    for (auto &t : clientThreads_) {
      t.join();
    }
    ::close(fd_);
    fd_ = -1;
  }

  int port() const { return port_; }
  int acceptedConnections() const { return accepted_.load(); }

private:
  void acceptLoop() {
    while (running_) {
      pollfd pfd{fd_, POLLIN, 0};
      if (::poll(&pfd, 1, 50) <= 0) {
        continue;
      }
      int client = ::accept(fd_, nullptr, nullptr);
      if (client < 0) {
        continue;
      }
      ++accepted_;
      std::lock_guard<std::mutex> guard(mutex_);
      clientFds_.push_back(client);
      clientThreads_.emplace_back([this, client]() { serve(client); });
    }
  }

  void serve(int client) {
    static const std::string response = "HTTP/1.1 200 OK\r\n"
                                        "Content-Length: 2\r\n"
                                        "Connection: keep-alive\r\n\r\nok";
    std::string pending;
    char buf[4096];
    while (running_) {
      pollfd pfd{client, POLLIN, 0};
      if (::poll(&pfd, 1, 50) <= 0) {
        continue;
      }
      ssize_t n = ::recv(client, buf, sizeof(buf), 0);
      if (n <= 0) {
        break;
      }
      pending.append(buf, static_cast<size_t>(n));
      size_t pos;
      while ((pos = pending.find("\r\n\r\n")) != std::string::npos) {
        pending.erase(0, pos + 4);
        ::send(client, response.data(), response.size(), 0);
      }
    }
    ::close(client);
  }

  int fd_ = -1;
  int port_ = 0;
  std::atomic<bool> running_{false};
  std::atomic<int> accepted_{0};
  std::thread acceptThread_;
  std::mutex mutex_;
  std::vector<int> clientFds_;
  std::vector<std::thread> clientThreads_;
};

Json baseRuntime(bool ipv4Only) {
  Json runtime;
  runtime["connectTimeout"] = 2000;
  runtime["readTimeout"] = 5000;
  runtime["keepAlive"] = true;
  if (ipv4Only) {
    runtime["ipv4Only"] = true;
  }
  return runtime;
}

std::string localhostUrl(int port) {
  return "http://localhost:" + std::to_string(port) + "/";
}

// Returns true on HTTP 200, false on a network error.
bool waitForOk(std::future<std::shared_ptr<MCurlResponse>> future) {
  if (future.wait_for(std::chrono::seconds(10)) != std::future_status::ready) {
    ADD_FAILURE() << "request timed out";
    return false;
  }
  try {
    auto response = future.get();
    if (!response) {
      return false;
    }
    response->getBody()->waitForDone();
    // Let the perform thread return the connection to the cache.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return response->getStatusCode() == 200;
  } catch (const ResponseException &) {
    return false;
  }
}

bool doActionOk(const std::string &url, const Json &runtime) {
  Request request(url);
  return waitForOk(Core::doAction(request, runtime));
}

} // namespace

class Ipv4OnlyRequestTest : public ::testing::Test {
protected:
  void SetUp() override { Core::ClearAllHttpClients(); }
  void TearDown() override { Core::ClearAllHttpClients(); }

  // Server reachable only via ::1; skip on hosts without IPv6 loopback.
  bool startIpv6OnlyServer(LoopbackHttpServer &server) {
    return server.start(AF_INET6);
  }
};

TEST_F(Ipv4OnlyRequestTest, Ipv6OnlyTargetDualStackSucceedsIpv4OnlyFails) {
  LoopbackHttpServer server;
  if (!startIpv6OnlyServer(server)) {
    GTEST_SKIP() << "IPv6 loopback (::1) not available";
  }
  std::string url = localhostUrl(server.port());

  EXPECT_TRUE(doActionOk(url, baseRuntime(false)));
  EXPECT_FALSE(doActionOk(url, baseRuntime(true)));
  // Unset and explicit false keep the dual-stack behavior.
  Json explicitFalse = baseRuntime(false);
  explicitFalse["ipv4Only"] = false;
  EXPECT_TRUE(doActionOk(url, explicitFalse));
  EXPECT_EQ(Core::GetHttpClientCount(), 2UL);
}

TEST_F(Ipv4OnlyRequestTest, Ipv4TargetWithIpv4OnlySucceeds) {
  LoopbackHttpServer server;
  ASSERT_TRUE(server.start(AF_INET));
  std::string url = localhostUrl(server.port());

  EXPECT_TRUE(doActionOk(url, baseRuntime(true)));
  EXPECT_TRUE(doActionOk(url, baseRuntime(true)));
  // Keep-alive reuse still works inside the IPv4-only pool.
  EXPECT_EQ(server.acceptedConnections(), 1);
}

TEST_F(Ipv4OnlyRequestTest, PoolsNotSharedBetweenIpv4OnlyAndDualStack) {
  LoopbackHttpServer server;
  ASSERT_TRUE(server.start(AF_INET));
  std::string url = localhostUrl(server.port());

  EXPECT_TRUE(doActionOk(url, baseRuntime(false)));
  EXPECT_EQ(Core::GetHttpClientCount(), 1UL);
  EXPECT_TRUE(doActionOk(url, baseRuntime(true)));
  EXPECT_EQ(Core::GetHttpClientCount(), 2UL);
  // The IPv4-only request did not reuse the dual-stack connection.
  EXPECT_EQ(server.acceptedConnections(), 2);

  EXPECT_TRUE(doActionOk(url, baseRuntime(false)));
  EXPECT_TRUE(doActionOk(url, baseRuntime(true)));
  EXPECT_EQ(server.acceptedConnections(), 2);
}

TEST_F(Ipv4OnlyRequestTest, Ipv4OnlyNeverReusesIpv6ConnectionFromDualStack) {
  LoopbackHttpServer server;
  if (!startIpv6OnlyServer(server)) {
    GTEST_SKIP() << "IPv6 loopback (::1) not available";
  }
  std::string url = localhostUrl(server.port());

  // Warm a keep-alive connection over ::1 in the dual-stack pool.
  EXPECT_TRUE(doActionOk(url, baseRuntime(false)));
  EXPECT_FALSE(doActionOk(url, baseRuntime(true)));
  EXPECT_EQ(server.acceptedConnections(), 1);
}

TEST_F(Ipv4OnlyRequestTest, MCurlHttpClientPoolConfigIpv4Only) {
  LoopbackHttpServer server;
  if (!startIpv6OnlyServer(server)) {
    GTEST_SKIP() << "IPv6 loopback (::1) not available";
  }
  MCurlHttpClient client;
  ConnectionPoolConfig config;
  config.ipv4_only = true;
  client.setConnectionPoolConfig(config);
  ASSERT_TRUE(client.start());

  Request request(localhostUrl(server.port()));
  EXPECT_FALSE(waitForOk(client.makeRequest(request)));
  EXPECT_EQ(server.acceptedConnections(), 0);
  client.stop();
}

TEST_F(Ipv4OnlyRequestTest, MCurlHttpClientRequestOptionIpv4Only) {
  LoopbackHttpServer server;
  if (!startIpv6OnlyServer(server)) {
    GTEST_SKIP() << "IPv6 loopback (::1) not available";
  }
  MCurlHttpClient client;
  ASSERT_TRUE(client.start());
  Request request(localhostUrl(server.port()));

  EXPECT_TRUE(waitForOk(client.makeRequest(request, baseRuntime(false))));
  // Same multi handle: libcurl must not hand the cached ::1 connection to a
  // request restricted to IPv4.
  EXPECT_FALSE(waitForOk(client.makeRequest(request, baseRuntime(true))));
  EXPECT_EQ(server.acceptedConnections(), 1);
  client.stop();
}

#endif // _WIN32
