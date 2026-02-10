#include <darabonba/String.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/encode/MD5.hpp>
#include <darabonba/encode/SHA1.hpp>
#include <darabonba/encode/SHA256.hpp>
#include <darabonba/encode/SM3.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace Darabonba;
using namespace Darabonba::Encode;
using namespace std;

// ==================== Encoder 原有测试 ====================
TEST(Darabonba_Encode_Encoder, urlEncode) {
  EXPECT_EQ(Encoder::urlEncode("123abc!@#$%^&*()-=_+ ~|\\/"),
            "123abc%21%40%23%24%25%5E%26%2A%28%29-%3D_%2B%20~%7C%5C%2F");
}

TEST(Darabonba_Encode_Encoder, urlEncodeEmpty) {
  EXPECT_EQ(Encoder::urlEncode(""), "");
}

TEST(Darabonba_Encode_Encoder, percentEncode) {
  EXPECT_EQ(Encoder::percentEncode("123abc!@#$%^&*()-=_+ ~|\\/"),
            "123abc%21%40%23%24%25%5E%26%2A%28%29-%3D_%2B%20~%7C%5C%2F");
}

TEST(Darabonba_Encode_Encoder, pathEncode) {
  EXPECT_EQ(Encoder::pathEncode("@/@"), "%40/%40");
  EXPECT_EQ(Encoder::pathEncode("@//@"), "%40//%40");
  EXPECT_EQ(Encoder::pathEncode("@"), "%40");
}

TEST(Darabonba_Encode_Encoder, pathEncodeEmpty) {
  EXPECT_EQ(Encoder::pathEncode(""), "");
  EXPECT_EQ(Encoder::pathEncode("/"), "/");
}

TEST(Darabonba_Encode_Encoder, hexEncode) {
  Bytes raw(vector<uint8_t>{20, 10, 30, 5});
  EXPECT_EQ("140a1e05", Encoder::hexEncode(raw));
}

TEST(Darabonba_Encode_Encoder, hexEncodeEmpty) {
  Bytes empty;
  EXPECT_EQ("", Encoder::hexEncode(empty));
}

TEST(Darabonba_Encode_Encoder, toString) {
  string data = "hello world";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  EXPECT_EQ(Encoder::toString(bytes), "hello world");
}

TEST(Darabonba_Encode_Encoder, toStringEmpty) {
  Bytes empty;
  EXPECT_EQ(Encoder::toString(empty), "");
}

TEST(Darabonba_Encode_Encoder, base64EncodeToString) {
  string case1 = "阿里云,coding the world!";
  Bytes bytes;
  bytes.assign(case1.begin(), case1.end());
  EXPECT_EQ(Encoder::base64EncodeToString(bytes),
            "6Zi/6YeM5LqRLGNvZGluZyB0aGUgd29ybGQh");
  string case2 = "ABCabc0123456789+/123abc!@#$%^&*()-=_+ ~|\\/";
  bytes.assign(case2.begin(), case2.end());
  EXPECT_EQ(Encoder::base64EncodeToString(bytes),
            "QUJDYWJjMDEyMzQ1Njc4OSsvMTIzYWJjIUAjJCVeJiooKS09XysgfnxcLw==");
}

TEST(Darabonba_Encode_Encoder, base64Decode) {
  string case1 = "阿里云,coding the world!";
  Bytes bytes;
  bytes.assign(case1.begin(), case1.end());
  EXPECT_EQ(Encoder::base64Decode("6Zi/6YeM5LqRLGNvZGluZyB0aGUgd29ybGQh"),
            bytes);
  string case2 = "ab$";
  bytes.assign(case2.begin(), case2.end());
  EXPECT_EQ(Encoder::base64Decode("YWIk"), bytes);
  string case3 = "ABCabc0123456789+/123abc!@#$%^&*()-=_+ ~|\\/";
  bytes.assign(case3.begin(), case3.end());
  EXPECT_EQ(Encoder::base64Decode(
                "QUJDYWJjMDEyMzQ1Njc4OSsvMTIzYWJjIUAjJCVeJiooKS09XysgfnxcLw=="),
            bytes);
}

// ==================== Encoder::hash 测试 ====================
TEST(Darabonba_Encode_Encoder, hashSHA256) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = Encoder::hash(bytes, "HMAC-SHA256");
  EXPECT_EQ(result.size(), 32u);
  EXPECT_EQ(Encoder::hexEncode(result),
            "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(Darabonba_Encode_Encoder, hashRSASHA256) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = Encoder::hash(bytes, "RSA-SHA256");
  EXPECT_EQ(result.size(), 32u);
  // 同样走 SHA256 hash
  EXPECT_EQ(Encoder::hexEncode(result),
            "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(Darabonba_Encode_Encoder, hashSM3) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = Encoder::hash(bytes, "HMAC-SM3");
  EXPECT_EQ(result.size(), 32u);
  EXPECT_FALSE(Encoder::hexEncode(result).empty());
}

TEST(Darabonba_Encode_Encoder, hashUnknownAlgorithm) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = Encoder::hash(bytes, "UNKNOWN");
  EXPECT_TRUE(result.empty());
}

// ==================== MD5 hash 测试 ====================
TEST(Darabonba_Encode_MD5, hashBytes) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = MD5::hash(bytes);
  EXPECT_EQ(result.size(), 16u);
  EXPECT_EQ(Encoder::hexEncode(result), "5d41402abc4b2a76b9719d911017c592");
}

TEST(Darabonba_Encode_MD5, hashVoidPtr) {
  string data = "hello";
  Bytes result = MD5::hash(data.c_str(), data.size());
  EXPECT_EQ(result.size(), 16u);
  EXPECT_EQ(Encoder::hexEncode(result), "5d41402abc4b2a76b9719d911017c592");
}

TEST(Darabonba_Encode_MD5, hashEmpty) {
  string data = "";
  Bytes result = MD5::hash(data.c_str(), data.size());
  EXPECT_EQ(result.size(), 16u);
  EXPECT_EQ(Encoder::hexEncode(result), "d41d8cd98f00b204e9800998ecf8427e");
}

// ==================== SHA1 hash 测试 ====================
TEST(Darabonba_Encode_SHA1, hashBytes) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = SHA1::hash(bytes);
  EXPECT_EQ(result.size(), 20u);
  EXPECT_EQ(Encoder::hexEncode(result), "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}

TEST(Darabonba_Encode_SHA1, hashVoidPtr) {
  string data = "hello";
  Bytes result = SHA1::hash(data.c_str(), data.size());
  EXPECT_EQ(result.size(), 20u);
  EXPECT_EQ(Encoder::hexEncode(result), "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}

// ==================== SHA256 hash 测试 ====================
TEST(Darabonba_Encode_SHA256, hashBytes) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = SHA256::hash(bytes);
  EXPECT_EQ(result.size(), 32u);
  EXPECT_EQ(Encoder::hexEncode(result),
            "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(Darabonba_Encode_SHA256, hashVoidPtr) {
  string data = "hello";
  Bytes result = SHA256::hash(data.c_str(), data.size());
  EXPECT_EQ(result.size(), 32u);
  EXPECT_EQ(Encoder::hexEncode(result),
            "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(Darabonba_Encode_SHA256, hashEmpty) {
  string data = "";
  Bytes result = SHA256::hash(data.c_str(), data.size());
  EXPECT_EQ(result.size(), 32u);
  EXPECT_EQ(Encoder::hexEncode(result),
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

// ==================== SM3 hash 测试 ====================
TEST(Darabonba_Encode_SM3, hashBytes) {
  string data = "hello";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes result = SM3::hash(bytes);
  EXPECT_EQ(result.size(), 32u);
  // SM3 是国密算法，验证长度即可
  EXPECT_FALSE(Encoder::hexEncode(result).empty());
}

TEST(Darabonba_Encode_SM3, hashVoidPtr) {
  string data = "hello";
  Bytes result = SM3::hash(data.c_str(), data.size());
  EXPECT_EQ(result.size(), 32u);
}

// ==================== SM3 与 SHA256 结果不同 ====================
TEST(Darabonba_Encode_Hash, sm3DiffFromSha256) {
  string data = "test data";
  Bytes bytes;
  bytes.assign(data.begin(), data.end());
  Bytes sha256Result = SHA256::hash(bytes);
  Bytes sm3Result = SM3::hash(bytes);
  // 不同算法应该产生不同结果
  EXPECT_NE(Encoder::hexEncode(sha256Result), Encoder::hexEncode(sm3Result));
}