#include <darabonba/Convert.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

// ==================== int8Val ====================
TEST(Darabonba_Convert, int8Val) {
  EXPECT_EQ(Convert::int8Val(Json(42)), 42);
  EXPECT_EQ(Convert::int8Val(Json(-128)), -128);
  EXPECT_EQ(Convert::int8Val(Json(127)), 127);
  EXPECT_EQ(Convert::int8Val(Json(0)), 0);
}

// ==================== int16Val ====================
TEST(Darabonba_Convert, int16Val) {
  EXPECT_EQ(Convert::int16Val(Json(1000)), 1000);
  EXPECT_EQ(Convert::int16Val(Json(-32768)), -32768);
  EXPECT_EQ(Convert::int16Val(Json(32767)), 32767);
  EXPECT_EQ(Convert::int16Val(Json(0)), 0);
}

// ==================== int32Val ====================
TEST(Darabonba_Convert, int32Val) {
  EXPECT_EQ(Convert::int32Val(Json(100000)), 100000);
  EXPECT_EQ(Convert::int32Val(Json(-100000)), -100000);
  EXPECT_EQ(Convert::int32Val(Json(0)), 0);
}

// ==================== int64Val ====================
TEST(Darabonba_Convert, int64Val) {
  EXPECT_EQ(Convert::int64Val(Json(1234567890123LL)), 1234567890123LL);
  EXPECT_EQ(Convert::int64Val(Json(-1234567890123LL)), -1234567890123LL);
  EXPECT_EQ(Convert::int64Val(Json(0)), 0);
}

// ==================== uint8Val ====================
TEST(Darabonba_Convert, uint8Val) {
  EXPECT_EQ(Convert::uint8Val(Json(255)), 255u);
  EXPECT_EQ(Convert::uint8Val(Json(0)), 0u);
  EXPECT_EQ(Convert::uint8Val(Json(128)), 128u);
}

// ==================== uint16Val ====================
TEST(Darabonba_Convert, uint16Val) {
  EXPECT_EQ(Convert::uint16Val(Json(65535)), 65535u);
  EXPECT_EQ(Convert::uint16Val(Json(0)), 0u);
}

// ==================== uint32Val ====================
TEST(Darabonba_Convert, uint32Val) {
  EXPECT_EQ(Convert::uint32Val(Json(4294967295u)), 4294967295u);
  EXPECT_EQ(Convert::uint32Val(Json(0)), 0u);
}

// ==================== uint64Val ====================
TEST(Darabonba_Convert, uint64Val) {
  EXPECT_EQ(Convert::uint64Val(Json(18446744073709551615ULL)),
            18446744073709551615ULL);
  EXPECT_EQ(Convert::uint64Val(Json(0)), 0u);
}

// ==================== floatVal ====================
TEST(Darabonba_Convert, floatVal) {
  EXPECT_FLOAT_EQ(Convert::floatVal(Json(3.14f)), 3.14f);
  EXPECT_FLOAT_EQ(Convert::floatVal(Json(0.0f)), 0.0f);
  EXPECT_FLOAT_EQ(Convert::floatVal(Json(-1.5f)), -1.5f);
}

// ==================== doubleVal ====================
TEST(Darabonba_Convert, doubleVal) {
  EXPECT_DOUBLE_EQ(Convert::doubleVal(Json(3.141592653589793)), 3.141592653589793);
  EXPECT_DOUBLE_EQ(Convert::doubleVal(Json(0.0)), 0.0);
  EXPECT_DOUBLE_EQ(Convert::doubleVal(Json(-2.718281828)), -2.718281828);
}

// ==================== stringVal ====================
TEST(Darabonba_Convert, stringVal) {
  EXPECT_EQ(Convert::stringVal(Json("hello")), "hello");
  EXPECT_EQ(Convert::stringVal(Json("")), "");
  EXPECT_EQ(Convert::stringVal(Json("中文测试")), "中文测试");
}

// ==================== boolVal ====================
TEST(Darabonba_Convert, boolVal) {
  EXPECT_TRUE(Convert::boolVal(Json(true)));
  EXPECT_FALSE(Convert::boolVal(Json(false)));
}

// ==================== toBytes ====================
TEST(Darabonba_Convert, toBytes) {
  auto bytes = Convert::toBytes(Json("hello"));
  EXPECT_EQ(bytes.size(), 5u);
  EXPECT_EQ(bytes[0], 'h');
  EXPECT_EQ(bytes[4], 'o');
}

TEST(Darabonba_Convert, toBytesEmpty) {
  auto bytes = Convert::toBytes(Json(""));
  EXPECT_TRUE(bytes.empty());
}

// ==================== 从整数 JSON 转换 floatVal ====================
TEST(Darabonba_Convert, floatValFromInt) {
  EXPECT_FLOAT_EQ(Convert::floatVal(Json(42)), 42.0f);
}

TEST(Darabonba_Convert, doubleValFromInt) {
  EXPECT_DOUBLE_EQ(Convert::doubleVal(Json(42)), 42.0);
}
