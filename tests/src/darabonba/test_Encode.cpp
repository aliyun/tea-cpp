#include <darabonba/String.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/Util.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace Darabonba;
using namespace Darabonba::Encode;
using namespace std;

TEST(Darabonba_Encode_Encoder, urlEncode) {
  EXPECT_EQ(Encoder::urlEncode("123abc!@#$%^&*()-=_+ ~|\\/"),
            "123abc%21%40%23%24%25%5E%26%2A%28%29-%3D_%2B%20~%7C%5C%2F");
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

TEST(Darabonba_Encode_Encoder, hexEncode) {
  Bytes raw(vector<uint8_t>{20, 10, 30, 5});
  EXPECT_EQ("140a1e05", Encoder::hexEncode(raw));
}

TEST(Darabonba_Encode_Encoder, hash) {
  Encoder::hash(Darabonba::Util::toBytes("hello"), "SHA-256");
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