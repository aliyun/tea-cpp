#include <darabonba/Time.hpp>
#include <chrono>
#include <gtest/gtest.h>
#include <regex>

using namespace Darabonba;

// ==================== unix ====================
TEST(Darabonba_Time, unixReturnsTimestamp) {
  std::string ts = Time::unix();
  EXPECT_FALSE(ts.empty());
  // 应该是一个正整数
  long val = std::stol(ts);
  EXPECT_GT(val, 1000000000L); // 大于 2001-09-09
}

TEST(Darabonba_Time, unixIsReasonable) {
  auto now = std::chrono::system_clock::now();
  auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
                   now.time_since_epoch())
                   .count();
  long ts = std::stol(Time::unix());
  // 应该跟当前时间相差不超过 2 秒
  EXPECT_LE(std::abs(ts - epoch), 2);
}

// ==================== UTC ====================
TEST(Darabonba_Time, UTCFormat) {
  std::string utc = Time::UTC();
  EXPECT_FALSE(utc.empty());
  // 格式: "2023-12-31 23:59:59.000000 +0000 UTC"
  EXPECT_NE(utc.find("+0000 UTC"), std::string::npos);
}

TEST(Darabonba_Time, UTCContainsDate) {
  std::string utc = Time::UTC();
  // 应该包含年份
  std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
  EXPECT_TRUE(std::regex_search(utc, pattern));
}

// ==================== format ====================
TEST(Darabonba_Time, formatYearMonthDay) {
  std::string formatted = Time::format("yyyy-MM-dd");
  // 应该匹配日期格式
  std::regex pattern(R"(\d{4}-\d{2}-\d{2})");
  EXPECT_TRUE(std::regex_match(formatted, pattern));
}

TEST(Darabonba_Time, formatHourMinute) {
  std::string formatted = Time::format("hh:mm");
  // 应该匹配时间格式
  std::regex pattern(R"(\d{2}:\d{2})");
  EXPECT_TRUE(std::regex_match(formatted, pattern));
}

TEST(Darabonba_Time, formatWithDayOfWeek) {
  std::string formatted = Time::format("EEEE");
  // 应该是一个星期几的全名
  EXPECT_FALSE(formatted.empty());
}

TEST(Darabonba_Time, formatWithAmPm) {
  std::string formatted = Time::format("a");
  // AM 或 PM
  EXPECT_TRUE(formatted == "AM" || formatted == "PM");
}

// ==================== sleep ====================
TEST(Darabonba_Time, sleepDoesNotCrash) {
  // Time::sleep 使用毫秒
  Time::sleep(1); // 1ms
  // 不崩溃就算通过
}
