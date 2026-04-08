#include <darabonba/Number.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

// ==================== parseInt ====================
TEST(Darabonba_Number, parseInt) {
  EXPECT_EQ(Number::parseInt("42"), 42);
  EXPECT_EQ(Number::parseInt("-100"), -100);
  EXPECT_EQ(Number::parseInt("0"), 0);
  EXPECT_EQ(Number::parseInt("2147483647"), 2147483647);
}

TEST(Darabonba_Number, parseIntInvalid) {
  EXPECT_THROW(Number::parseInt("abc"), std::invalid_argument);
  EXPECT_THROW(Number::parseInt(""), std::invalid_argument);
}

// ==================== parseLong ====================
TEST(Darabonba_Number, parseLong) {
  EXPECT_EQ(Number::parseLong("1234567890"), 1234567890L);
  EXPECT_EQ(Number::parseLong("-1234567890"), -1234567890L);
  EXPECT_EQ(Number::parseLong("0"), 0L);
}

TEST(Darabonba_Number, parseLongInvalid) {
  EXPECT_THROW(Number::parseLong("xyz"), std::invalid_argument);
}

// ==================== parseFloat ====================
TEST(Darabonba_Number, parseFloat) {
  EXPECT_FLOAT_EQ(Number::parseFloat("3.14"), 3.14f);
  EXPECT_FLOAT_EQ(Number::parseFloat("-1.5"), -1.5f);
  EXPECT_FLOAT_EQ(Number::parseFloat("0.0"), 0.0f);
}

TEST(Darabonba_Number, parseFloatInvalid) {
  EXPECT_THROW(Number::parseFloat("not_a_number"), std::invalid_argument);
}

// ==================== parseDouble ====================
TEST(Darabonba_Number, parseDouble) {
  EXPECT_DOUBLE_EQ(Number::parseDouble("3.141592653589793"), 3.141592653589793);
  EXPECT_DOUBLE_EQ(Number::parseDouble("-2.718281828"), -2.718281828);
  EXPECT_DOUBLE_EQ(Number::parseDouble("0.0"), 0.0);
}

TEST(Darabonba_Number, parseDoubleInvalid) {
  EXPECT_THROW(Number::parseDouble(""), std::invalid_argument);
}

// ==================== itol ====================
TEST(Darabonba_Number, itol) {
  EXPECT_EQ(Number::itol(42), 42L);
  EXPECT_EQ(Number::itol(-100), -100L);
  EXPECT_EQ(Number::itol(0), 0L);
}

// ==================== ltoi ====================
TEST(Darabonba_Number, ltoi) {
  EXPECT_EQ(Number::ltoi(42L), 42);
  EXPECT_EQ(Number::ltoi(-100L), -100);
  EXPECT_EQ(Number::ltoi(0L), 0);
}

// ==================== 边界值 ====================
TEST(Darabonba_Number, parseIntWithLeadingSpaces) {
  // std::stoi 会忽略前导空格
  EXPECT_EQ(Number::parseInt("  123"), 123);
}

TEST(Darabonba_Number, parseFloatScientificNotation) {
  EXPECT_FLOAT_EQ(Number::parseFloat("1.5e2"), 150.0f);
}

TEST(Darabonba_Number, parseDoubleScientificNotation) {
  EXPECT_DOUBLE_EQ(Number::parseDouble("1.5e10"), 1.5e10);
}

// ==================== int64_t 大数值测试 ====================
// 这些测试证明了使用 int64_t 和 std::stoll 而非 long 和 std::stol 的必要性
// 在 Windows 平台上，long 是 32 位的，无法正确处理大于 INT32_MAX 的值

// 测试 INT32_MAX (2147483647) - 在所有平台上都能正确处理
TEST(Darabonba_Number, parseLongInt32Max) {
  std::string val = "2147483647";  // INT32_MAX
  int64_t result = Number::parseLong(val);
  EXPECT_EQ(result, 2147483647LL);
}

// 测试 INT32_MAX + 1 - 在 32 位 long 平台（如 Windows）上会溢出
// 如果使用 std::stol，这个测试在 Windows 上会失败
TEST(Darabonba_Number, parseLongInt32MaxPlusOne) {
  std::string val = "2147483648";  // INT32_MAX + 1
  int64_t result = Number::parseLong(val);
  EXPECT_EQ(result, 2147483648LL);  // 需要 int64_t 才能正确存储
}

// 测试大数值 - 证明 int64_t 可以处理超出 32 位范围的值
TEST(Darabonba_Number, parseLongLargeValue) {
  std::string val = "9223372036854775807";  // INT64_MAX
  int64_t result = Number::parseLong(val);
  EXPECT_EQ(result, 9223372036854775807LL);
}

// 测试 INT32_MIN - 1 - 负数边界
TEST(Darabonba_Number, parseLongInt32MinMinusOne) {
  std::string val = "-2147483649";  // INT32_MIN - 1
  int64_t result = Number::parseLong(val);
  EXPECT_EQ(result, -2147483649LL);
}

// 测试一个大时间戳（毫秒）- 实际应用场景
// Unix 时间戳 2024-01-01 00:00:00 UTC 的毫秒值
TEST(Darabonba_Number, parseLongTimestamp) {
  std::string val = "1704067200000";  // 2024-01-01 00:00:00 UTC in milliseconds
  int64_t result = Number::parseLong(val);
  EXPECT_EQ(result, 1704067200000LL);
  // 这个值远超 INT32_MAX，使用 long (32-bit) 会溢出
  EXPECT_GT(result, 2147483647LL);  // 证明需要 int64_t
}

// 测试 itol 返回 int64_t
TEST(Darabonba_Number, itolReturnsInt64) {
  // 验证返回类型可以存储 int64_t 值
  auto result = Number::itol(42);
  static_assert(std::is_same<decltype(result), int64_t>::value,
                "itol should return int64_t");
  EXPECT_EQ(result, 42LL);
}

// 测试 ltoi 接受 int64_t
TEST(Darabonba_Number, ltoiAcceptsInt64) {
  // 验证可以接受 int64_t 参数
  int64_t largeVal = 2147483648LL;  // INT32_MAX + 1
  int result = Number::ltoi(largeVal);
  // 注意：这里会发生截断，但函数签名应该接受 int64_t
  EXPECT_EQ(result, static_cast<int>(largeVal));
}
