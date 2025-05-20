#include <darabonba/Bytes.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Darabonba;

TEST(Darabonba_Bytes, toBytes) {
    std::string str = "hello world";
    Bytes bytes = Bytes::toBytes(str);
    EXPECT_EQ(bytes.size(), 11);
}

TEST(Darabonba_Bytes, toHex) {
    Bytes bytes = Bytes::toBytes("hello");
    EXPECT_EQ(bytes.toHex(), "68656c6c6f");
}

TEST(Darabonba_Bytes, toBase64) {
    Bytes bytes = Bytes::toBytes("hello");
    EXPECT_EQ(bytes.toBase64(), "aGVsbG8=");
}

TEST(Darabonba_Bytes, size) {
    Bytes bytes = Bytes::toBytes("hello");
    EXPECT_EQ(bytes.size(), 5);
}

TEST(Darabonba_Bytes, toJSON) {
    Bytes bytes = Bytes::toBytes("hello");
    EXPECT_EQ(bytes.toJSON(), "\"68656c6c6f\"");
}

TEST(Darabonba_Bytes, fromBase64) {
    Bytes bytes = Bytes::from("aGVsbG8=", "base64");
    EXPECT_EQ(bytes.toHex(), "68656c6c6f");
}

TEST(Darabonba_Bytes, equalityCheck) {
    Bytes bytes1 = Bytes::toBytes("hello");
    Bytes bytes2 = Bytes::from("aGVsbG8=", "base64");
    EXPECT_TRUE(bytes1.toHex() == bytes2.toHex());
}