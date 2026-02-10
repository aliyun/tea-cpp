#include <darabonba/String.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace Darabonba;

TEST(Darabonba_String, split) {
  EXPECT_THAT(String::split("1#2", "#"), testing::ElementsAre("1", "2"));
  EXPECT_THAT(String::split("1#2#", "#"), testing::ElementsAre("1", "2", ""));
  EXPECT_THAT(String::split("1#2#", "#", 2), testing::ElementsAre("1", "2#"));
  EXPECT_THAT(String::split("#1#2", "#"), testing::ElementsAre("", "1", "2"));
  EXPECT_THAT(String::split("1#2#3", "#", 2), testing::ElementsAre("1", "2#3"));
  EXPECT_THAT(String::split("1#2#3", "#", 3),
              testing::ElementsAre("1", "2", "3"));
  EXPECT_THAT(String::split("1#2#3", "#", 4),
              testing::ElementsAre("1", "2", "3"));
}

TEST(Darabonba_String, replace) {
  EXPECT_EQ(String::replace("1#2", "#", "@"), "1@2");
  EXPECT_EQ(String::replace("1#2#", "#", "@"), "1@2@");
  EXPECT_EQ(String::replace("#1#2", "#", "@"), "@1@2");
  EXPECT_EQ(String::replace("#1#2", "#", "@", 1), "@1#2");
  EXPECT_EQ(String::replace("#1#2", "#", "", 1), "1#2");
  EXPECT_EQ(String::replace("####", "#", ""), "");
}

TEST(Darabonba_String, contains) {
  EXPECT_EQ(String::contains("####", "#"), true);
  EXPECT_EQ(String::contains("####", "a"), false);
  EXPECT_EQ(String::contains("####", ""), true);
}

TEST(Darabonba_String, count) {
  EXPECT_EQ(String::count("####", "#"), 4);
  EXPECT_EQ(String::count("####", "##"), 2);
  EXPECT_EQ(String::count("####", ""), 0);
}

TEST(Darabonba_String, hasPrefix) {
  EXPECT_EQ(String::hasPrefix("####", "#"), true);
  EXPECT_EQ(String::hasPrefix("####", "##"), true);
  EXPECT_EQ(String::hasPrefix("####", ""), true);
  EXPECT_EQ(String::hasPrefix("####", "a"), false);
  EXPECT_EQ(String::hasPrefix("####", "######"), false);
}

TEST(Darabonba_String, hasSuffix) {
  EXPECT_EQ(String::hasSuffix("####", "#"), true);
  EXPECT_EQ(String::hasSuffix("####", "##"), true);
  EXPECT_EQ(String::hasSuffix("####", ""), true);
  EXPECT_EQ(String::hasSuffix("####", "a"), false);
  EXPECT_EQ(String::hasSuffix("####", "######"), false);
}

TEST(Darabonba_String, index) {
  EXPECT_EQ(String::index("####", "#"), 0);
  EXPECT_EQ(String::index("####", "##"), 0);
  EXPECT_EQ(String::index("####", ""), 0);
  EXPECT_EQ(String::index("###a", "a"), 3);
  EXPECT_EQ(String::index("####", "######"), -1);
}

TEST(Darabonba_String, toLower) {
  EXPECT_EQ(String::toLower("#Aa#"), "#aa#");
  EXPECT_EQ(String::toLower(""), "");
}

TEST(Darabonba_String, toUpper) {
  EXPECT_EQ(String::toUpper("#Aa#"), "#AA#");
  EXPECT_EQ(String::toUpper(""), "");
}

TEST(Darabonba_String, subString) {
  EXPECT_EQ(String::subString("#Aa#", 0), "#Aa#");
  EXPECT_EQ(String::subString("abcd", 1, 2), "b");
  EXPECT_EQ(String::subString("abcd", 1), "bcd");
}

TEST(Darabonba_String, equals) {
  EXPECT_EQ(String::equals("#Aa#", "#Aa#"), true);
  EXPECT_EQ(String::equals("abcd", "b"), false);
  EXPECT_EQ(String::equals("abcd", ""), false);
}

TEST(Darabonba_String, trim) {
  EXPECT_EQ(String::trim("  # Aa #  "), "# Aa #");
  EXPECT_EQ(String::trim(""), "");
  EXPECT_EQ(String::trim("   "), "");
  EXPECT_EQ(String::trim("noSpaces"), "noSpaces");
  EXPECT_EQ(String::trim("\t\n hello \r\n"), "hello");
}

// ==================== 边界场景补充 ====================
TEST(Darabonba_String, splitWithEmptyDelimiter) {
  EXPECT_THAT(String::split("hello", ""),
              testing::ElementsAre("hello"));
}

TEST(Darabonba_String, replaceNoMatch) {
  EXPECT_EQ(String::replace("hello", "xyz", "abc"), "hello");
}

TEST(Darabonba_String, containsSubstring) {
  EXPECT_TRUE(String::contains("hello world", "world"));
  EXPECT_FALSE(String::contains("hello world", "xyz"));
}

TEST(Darabonba_String, indexNotFound) {
  EXPECT_EQ(String::index("hello", "xyz"), -1);
}

TEST(Darabonba_String, toLowerMixed) {
  EXPECT_EQ(String::toLower("Hello WORLD 123"), "hello world 123");
}

TEST(Darabonba_String, toUpperMixed) {
  EXPECT_EQ(String::toUpper("Hello world 123"), "HELLO WORLD 123");
}

TEST(Darabonba_String, subStringBoundary) {
  EXPECT_EQ(String::subString("abcdef", 0, 6), "abcdef");
  EXPECT_EQ(String::subString("abcdef", 5), "f");
  EXPECT_EQ(String::subString("abcdef", 6), "");
}

TEST(Darabonba_String, equalsEmpty) {
  EXPECT_TRUE(String::equals("", ""));
  EXPECT_FALSE(String::equals("a", ""));
}

TEST(Darabonba_String, hasPrefixExactMatch) {
  EXPECT_TRUE(String::hasPrefix("hello", "hello"));
}

TEST(Darabonba_String, hasSuffixExactMatch) {
  EXPECT_TRUE(String::hasSuffix("hello", "hello"));
}

TEST(Darabonba_String, countNoMatch) {
  EXPECT_EQ(String::count("hello", "xyz"), 0);
}
