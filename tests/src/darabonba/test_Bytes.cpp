#include <darabonba/Bytes.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

TEST(Darabonba_BytesUtil, toBytes) {
  std::string str = "hello world";
  Bytes bytes = BytesUtil::toBytes(str);
  EXPECT_EQ(bytes.size(), 11u);
  EXPECT_EQ(bytes[0], static_cast<uint8_t>('h'));
  EXPECT_EQ(bytes[10], static_cast<uint8_t>('d'));
}

TEST(Darabonba_BytesUtil, toBytesEmpty) {
  Bytes bytes = BytesUtil::toBytes("");
  EXPECT_TRUE(bytes.empty());
}

TEST(Darabonba_BytesUtil, toHex) {
  BytesUtil bu("hello");
  EXPECT_EQ(bu.toHex(), "68656c6c6f");
}

TEST(Darabonba_BytesUtil, toHexEmpty) {
  BytesUtil bu("");
  EXPECT_EQ(bu.toHex(), "");
}

TEST(Darabonba_BytesUtil, toBase64) {
  BytesUtil bu("hello");
  EXPECT_EQ(bu.toBase64(), "aGVsbG8=");
}

TEST(Darabonba_BytesUtil, toBase64Binary) {
  BytesUtil bu("Man");
  EXPECT_EQ(bu.toBase64(), "TWFu");
}

TEST(Darabonba_BytesUtil, size) {
  BytesUtil bu("hello");
  EXPECT_EQ(bu.size(), 5);
}

TEST(Darabonba_BytesUtil, sizeEmpty) {
  BytesUtil bu("");
  EXPECT_EQ(bu.size(), 0);
}

TEST(Darabonba_BytesUtil, toJSON) {
  BytesUtil bu("hello");
  EXPECT_EQ(bu.toJSON(), "\"68656c6c6f\"");
}

TEST(Darabonba_BytesUtil, fromBase64) {
  Bytes bytes = BytesUtil::from("aGVsbG8=", "base64");
  BytesUtil bu(std::vector<unsigned char>(bytes.begin(), bytes.end()));
  EXPECT_EQ(bu.toHex(), "68656c6c6f");
}

TEST(Darabonba_BytesUtil, fromUtf8) {
  Bytes bytes = BytesUtil::from("hello", "utf8");
  EXPECT_EQ(bytes.size(), 5u);
  EXPECT_EQ(bytes[0], static_cast<uint8_t>('h'));
}

TEST(Darabonba_BytesUtil, fromUtf8Dash) {
  Bytes bytes = BytesUtil::from("hello", "utf-8");
  EXPECT_EQ(bytes.size(), 5u);
}

TEST(Darabonba_BytesUtil, fromHex) {
  Bytes bytes = BytesUtil::from("68656c6c6f", "hex");
  EXPECT_EQ(bytes.size(), 5u);
  EXPECT_EQ(bytes[0], static_cast<uint8_t>('h'));
  EXPECT_EQ(bytes[4], static_cast<uint8_t>('o'));
}

TEST(Darabonba_BytesUtil, fromHexOddLength) {
  EXPECT_THROW(BytesUtil::from("abc", "hex"), std::invalid_argument);
}

TEST(Darabonba_BytesUtil, fromUnsupportedFormat) {
  EXPECT_THROW(BytesUtil::from("abc", "unknown"), std::invalid_argument);
}

TEST(Darabonba_BytesUtil, equalityCheck) {
  BytesUtil bytes1("hello");
  Bytes bytes2 = BytesUtil::from("aGVsbG8=", "base64");
  BytesUtil bu2(std::vector<unsigned char>(bytes2.begin(), bytes2.end()));
  EXPECT_EQ(bytes1.toHex(), bu2.toHex());
}

TEST(Darabonba_BytesUtil, constructFromVector) {
  std::vector<unsigned char> vec = {0x48, 0x49};
  BytesUtil bu(vec);
  EXPECT_EQ(bu.size(), 2);
  EXPECT_EQ(bu.toHex(), "4849");
}
