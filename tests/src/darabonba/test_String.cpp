#include <darabonba/String.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Darabonba;

TEST(Darabonba_String, split) {
  EXPECT_THAT(String::split("1#2", "#"), testing::ElementsAre("1", "2"));
  EXPECT_THAT(String::split("1#2#", "#"), testing::ElementsAre("1", "2", ""));
  EXPECT_THAT(String::split("1#2#", "#", 2), testing::ElementsAre("1", "2#"));
  EXPECT_THAT(String::split("#1#2", "#"), testing::ElementsAre("", "1", "2"));
  EXPECT_THAT(String::split("1#2#3", "#", 2), testing::ElementsAre("1", "2#3"));
  EXPECT_THAT(String::split("1#2#3", "#", 3), testing::ElementsAre("1", "2", "3"));
  EXPECT_THAT(String::split("1#2#3", "#", 4), testing::ElementsAre("1", "2", "3"));
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
}
