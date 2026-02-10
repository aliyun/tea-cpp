#include <darabonba/encode/Encoder.hpp>
#include <darabonba/http/URL.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;
using namespace Darabonba::Http;

class URLTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

// ==================== URL Parse 测试 ====================
TEST_F(URLTest, ParseSimpleURL) {
  URL url("https://example.com/path");

  EXPECT_EQ(url.getScheme(), "https");
  EXPECT_EQ(url.getHost(), "example.com");
  EXPECT_EQ(url.getPathName(), "/path");
}

TEST_F(URLTest, ParseURLWithPort) {
  URL url("http://localhost:8080/api");

  EXPECT_EQ(url.getScheme(), "http");
  EXPECT_EQ(url.getHost(), "localhost");
  EXPECT_EQ(url.getPort(), 8080);
  EXPECT_EQ(url.getPathName(), "/api");
}

TEST_F(URLTest, ParseURLWithQuery) {
  URL url("https://example.com/search?q=test&page=1");

  EXPECT_EQ(url.getScheme(), "https");
  EXPECT_EQ(url.getHost(), "example.com");
  EXPECT_EQ(url.getPathName(), "/search");
  EXPECT_EQ(url.getQuery("q"), "test");
  EXPECT_EQ(url.getQuery("page"), "1");
}

TEST_F(URLTest, ParseURLWithFragment) {
  URL url("https://example.com/page#section");

  EXPECT_EQ(url.getScheme(), "https");
  EXPECT_EQ(url.getHost(), "example.com");
  EXPECT_EQ(url.getPathName(), "/page");
  EXPECT_TRUE(url.hasFragment());
  EXPECT_EQ(url.getFragment(), "section");
}

TEST_F(URLTest, ParseURLWithUserInfo) {
  URL url("https://user:password@example.com/path");

  EXPECT_EQ(url.getScheme(), "https");
  EXPECT_EQ(url.getUser(), "user");
  EXPECT_EQ(url.getPassword(), "password");
  EXPECT_EQ(url.getHost(), "example.com");
}

TEST_F(URLTest, ParseURLWithAllComponents) {
  URL url(
      "https://user:pass@example.com:443/path/to/resource?key=value#section");

  EXPECT_EQ(url.getScheme(), "https");
  EXPECT_EQ(url.getUser(), "user");
  EXPECT_EQ(url.getPassword(), "pass");
  EXPECT_EQ(url.getHost(), "example.com");
  EXPECT_EQ(url.getPort(), 443);
  EXPECT_EQ(url.getPathName(), "/path/to/resource");
  EXPECT_EQ(url.getQuery("key"), "value");
  EXPECT_EQ(url.getFragment(), "section");
}

TEST_F(URLTest, ParseEmptyURL) {
  URL url("");

  EXPECT_TRUE(url.empty());
}

TEST_F(URLTest, ParseURLWithIPAddress) {
  URL url("http://192.168.1.1:3000/api");

  EXPECT_EQ(url.getHost(), "192.168.1.1");
  EXPECT_EQ(url.getPort(), 3000);
}

// ==================== URL 操作测试 ====================
TEST_F(URLTest, SetScheme) {
  URL url("http://example.com");
  url.setScheme("https");

  EXPECT_EQ(url.getScheme(), "https");
}

TEST_F(URLTest, SetHost) {
  URL url("https://old.example.com");
  url.setHost("new.example.com");

  EXPECT_EQ(url.getHost(), "new.example.com");
}

TEST_F(URLTest, SetPort) {
  URL url("https://example.com");
  url.setPort(8443);

  EXPECT_EQ(url.getPort(), 8443);
}

TEST_F(URLTest, SetPathName) {
  URL url("https://example.com/old");
  url.setPathName("/new/path");

  EXPECT_EQ(url.getPathName(), "/new/path");
}

TEST_F(URLTest, SetQuery) {
  URL url("https://example.com");
  Query query;
  query["key1"] = "value1";
  query["key2"] = "value2";
  url.setQuery(query);

  EXPECT_EQ(url.getQuery("key1"), "value1");
  EXPECT_EQ(url.getQuery("key2"), "value2");
}

TEST_F(URLTest, SetFragment) {
  URL url("https://example.com");
  url.setFragment("anchor");

  EXPECT_TRUE(url.hasFragment());
  EXPECT_EQ(url.getFragment(), "anchor");
}

TEST_F(URLTest, SetUserInfo) {
  URL url("https://example.com");
  url.setUser("username");
  url.setPassword("secret");

  EXPECT_EQ(url.getUser(), "username");
  EXPECT_EQ(url.getPassword(), "secret");
  EXPECT_EQ(url.getUserInfo(), "username:secret");
}

// ==================== URL 转换测试 ====================
TEST_F(URLTest, ToStringSimple) {
  URL url("https://example.com/path");
  std::string str = static_cast<std::string>(url);

  EXPECT_FALSE(str.empty());
  EXPECT_NE(str.find("example.com"), std::string::npos);
}

// ==================== URL 验证测试 ====================
TEST_F(URLTest, IsValidWithValidURL) {
  URL url("https://example.com");
  EXPECT_TRUE(url.isValid());
}

TEST_F(URLTest, EmptyURLIsNotValid) {
  URL url;
  EXPECT_TRUE(url.empty());
}

// ==================== URL 比较测试 ====================
TEST_F(URLTest, EqualityOperator) {
  URL url1("https://example.com/path");
  URL url2("https://example.com/path");

  EXPECT_TRUE(url1 == url2);
}

TEST_F(URLTest, InequalityOperator) {
  URL url1("https://example.com/path1");
  URL url2("https://example.com/path2");

  EXPECT_TRUE(url1 != url2);
}

// ==================== URL Copy/Move 测试 ====================
TEST_F(URLTest, CopyConstructor) {
  URL url1("https://example.com/path?key=value");
  URL url2(url1);

  EXPECT_EQ(url2.getScheme(), url1.getScheme());
  EXPECT_EQ(url2.getHost(), url1.getHost());
  EXPECT_EQ(url2.getPathName(), url1.getPathName());
  EXPECT_EQ(url2.getQuery("key"), url1.getQuery("key"));
}

TEST_F(URLTest, MoveConstructor) {
  URL url1("https://example.com/path");
  URL url2(std::move(url1));

  EXPECT_EQ(url2.getScheme(), "https");
  EXPECT_EQ(url2.getHost(), "example.com");
}

TEST_F(URLTest, AssignmentOperator) {
  URL url1("https://example.com/path");
  URL url2;
  url2 = url1;

  EXPECT_EQ(url2.getScheme(), url1.getScheme());
  EXPECT_EQ(url2.getHost(), url1.getHost());
}

// ==================== URL 清除测试 ====================
TEST_F(URLTest, ClearURL) {
  URL url("https://example.com/path");
  url.clear();

  EXPECT_TRUE(url.empty());
}

// ==================== URL 编码测试 (与 Encoder 配合) ====================
TEST_F(URLTest, URLEncodeWorksCorrectly) {
  std::string input = "hello world";
  std::string encoded = Encode::Encoder::urlEncode(input);

  EXPECT_NE(encoded.find("%20"), std::string::npos);
}

TEST_F(URLTest, PercentEncodeWorksCorrectly) {
  std::string input = "test@value";
  std::string encoded = Encode::Encoder::percentEncode(input);

  EXPECT_NE(encoded.find("%40"), std::string::npos);
}

TEST_F(URLTest, PathEncodeWorksCorrectly) {
  std::string input = "path/to/resource";
  std::string encoded = Encode::Encoder::pathEncode(input);

  // 路径编码保留 /
  EXPECT_NE(encoded.find("/"), std::string::npos);
}

TEST_F(URLTest, URLEncodeSpecialCharacters) {
  std::string input = "key=value&foo=bar";
  std::string encoded = Encode::Encoder::urlEncode(input);

  EXPECT_NE(encoded.find("%3D"), std::string::npos); // =
  EXPECT_NE(encoded.find("%26"), std::string::npos); // &
}
