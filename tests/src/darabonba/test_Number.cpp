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
