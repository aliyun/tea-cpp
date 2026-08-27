#include <darabonba/Exception.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/http/Request.hpp>
#include <curl/curl.h>
#include <gtest/gtest.h>

using namespace Darabonba;
using namespace Darabonba::Http;
using namespace Darabonba::Http::Curl;

class CurlProxyTest : public ::testing::Test {};

TEST_F(CurlProxyTest, ValidateSocks5NetworkWithoutProxyFailsFast) {
  EXPECT_THROW(validateProxyOptions("", "", "", "tcp"), ValidateException);
}

TEST_F(CurlProxyTest, ValidateInvalidSocks5NetworkFailsFast) {
  EXPECT_THROW(validateProxyOptions("", "", "socks5://127.0.0.1:1080", "icmp"),
               ValidateException);
}

TEST_F(CurlProxyTest, ValidateSocks5UdpFailsFast) {
  EXPECT_THROW(validateProxyOptions("", "", "socks5://127.0.0.1:1080", "udp"),
               ValidateException);
}

TEST_F(CurlProxyTest, ValidateInvalidSocks5ProxyFailsFast) {
  EXPECT_THROW(validateProxyOptions("", "", "# #%gfdf", ""), ValidateException);
}

TEST_F(CurlProxyTest, ValidateInvalidHttpProxyFailsFast) {
  EXPECT_THROW(validateProxyOptions("://bad", "", "", ""), ValidateException);
}

TEST_F(CurlProxyTest, ValidateInvalidHttpsProxyFailsFast) {
  EXPECT_THROW(validateProxyOptions("", "://bad", "", ""), ValidateException);
}

TEST_F(CurlProxyTest, ValidateValidSocks5ProxyPasses) {
  EXPECT_NO_THROW(validateProxyOptions("", "", "socks5://user:pass@127.0.0.1:1080",
                                       "tcp"));
}

TEST_F(CurlProxyTest, ValidateValidHttpProxyPasses) {
  EXPECT_NO_THROW(validateProxyOptions("http://127.0.0.1:8080", "", "", ""));
}

TEST_F(CurlProxyTest, ValidateValidHttpsProxyPasses) {
  EXPECT_NO_THROW(validateProxyOptions("", "https://127.0.0.1:8443", "", ""));
}

TEST_F(CurlProxyTest, ApplyCurlProxyOptionsHttpBranch) {
  CURL *curl = curl_easy_init();
  ASSERT_NE(curl, nullptr);
  EXPECT_NO_THROW(applyCurlProxyOptions(curl, "http://127.0.0.1:8080", "", "",
                                        "", "localhost"));
  curl_easy_cleanup(curl);
}

TEST_F(CurlProxyTest, ApplyCurlProxyOptionsHttpsBranch) {
  CURL *curl = curl_easy_init();
  ASSERT_NE(curl, nullptr);
  EXPECT_NO_THROW(applyCurlProxyOptions(curl, "", "https://127.0.0.1:8443", "",
                                        "", ""));
  curl_easy_cleanup(curl);
}

TEST_F(CurlProxyTest, ApplyCurlProxyOptionsSocks5hBranch) {
  CURL *curl = curl_easy_init();
  ASSERT_NE(curl, nullptr);
  EXPECT_NO_THROW(applyCurlProxyOptions(curl, "", "", "socks5h://127.0.0.1:1080",
                                        "tcp", ""));
  curl_easy_cleanup(curl);
}

TEST_F(CurlProxyTest, ApplyCurlProxyOptionsHttpPriorityOverSocks5) {
  CURL *curl = curl_easy_init();
  ASSERT_NE(curl, nullptr);
  EXPECT_NO_THROW(applyCurlProxyOptions(
      curl, "http://127.0.0.1:8080", "", "socks5://127.0.0.1:1080", "tcp", ""));
  curl_easy_cleanup(curl);
}

TEST_F(CurlProxyTest, MakeRequestAppliesSocks5OptionsBeforeNetworkIO) {
  MCurlHttpClient client;
  client.start();

  Request request("https://www.example.com");
  Darabonba::Json options;
  options["socks5Proxy"] = "socks5://127.0.0.1:1080";
  options["socks5NetWork"] = "tcp";
  options["noProxy"] = "localhost";
  options["connectTimeout"] = 1000L;
  options["readTimeout"] = 1000L;

  auto future = client.makeRequest(request, options);
  auto status = future.wait_for(std::chrono::seconds(5));
  ASSERT_EQ(status, std::future_status::ready);

  try {
    (void)future.get();
  } catch (const std::exception &) {
    // Connection failure against local proxy is acceptable; config was accepted.
  }

  client.stop();
}

TEST_F(CurlProxyTest, MakeRequestRejectsInvalidSocks5Configuration) {
  MCurlHttpClient client;
  client.start();

  Request request("https://www.example.com");
  Darabonba::Json options;
  options["socks5NetWork"] = "tcp";

  EXPECT_THROW(client.makeRequest(request, options), ValidateException);

  client.stop();
}

TEST_F(CurlProxyTest, MakeRequestAppliesHttpProxyBeforeNetworkIO) {
  MCurlHttpClient client;
  client.start();

  Request request("https://www.example.com");
  Darabonba::Json options;
  options["httpProxy"] = "http://127.0.0.1:8080";
  options["connectTimeout"] = 1000L;
  options["readTimeout"] = 1000L;

  auto future = client.makeRequest(request, options);
  auto status = future.wait_for(std::chrono::seconds(5));
  ASSERT_EQ(status, std::future_status::ready);

  try {
    (void)future.get();
  } catch (const std::exception &) {
  }

  client.stop();
}
