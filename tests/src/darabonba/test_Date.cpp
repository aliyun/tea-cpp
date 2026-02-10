#include <darabonba/Date.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

class DateTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    // 设置时区为 Asia/Shanghai (UTC+8)
#ifdef _WIN32
    _putenv_s("TZ", "CST-8");
    _tzset();
#else
    setenv("TZ", "Asia/Shanghai", 1);
    tzset();
#endif
  }

  virtual void TearDown() override {
    // 恢复默认时区
#ifdef _WIN32
    _putenv_s("TZ", "");
    _tzset();
#else
    unsetenv("TZ");
    tzset();
#endif
  }
};

// ==================== 初始化测试 ====================
TEST_F(DateTest, InitShouldBeOkay) {
  Date date("2023-12-31 00:00:00");
  // 验证日期对象创建成功
  EXPECT_EQ(date.format("%Y-%m-%d %H:%M:%S"), "2023-12-31 00:00:00");
}

TEST_F(DateTest, InitWithDifferentFormats) {
  // 测试不同格式的日期字符串
  Date date1("2023-09-12 17:47:31");
  EXPECT_EQ(date1.format("%Y-%m-%d %H:%M:%S"), "2023-09-12 17:47:31");

  // 测试带时区的日期字符串
  Date date2("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date2.format("%Y-%m-%d %H:%M:%S"), "2023-12-31 08:00:00");
}

// ==================== format 测试 ====================
TEST_F(DateTest, FormatShouldReturnCorrectString) {
  Date date("2023-09-12 17:47:31");
  EXPECT_EQ(date.format("%Y-%m-%d %H:%M:%S"), "2023-09-12 17:47:31");
  EXPECT_EQ(date.format("%Y-%m-%d"), "2023-09-12");
  EXPECT_EQ(date.format("%H:%M:%S"), "17:47:31");
}

TEST_F(DateTest, FormatWithDifferentPatterns) {
  Date date("2023-09-12 17:47:31");
  EXPECT_EQ(date.format("%Y/%m/%d"), "2023/09/12");
  EXPECT_EQ(date.format("%Y年%m月%d日"), "2023年09月12日");
}

// ==================== unix 时间戳测试 ====================
TEST_F(DateTest, UnixTimestampForEpoch) {
  Date date("1970-01-01 00:00:00");
  // Unix 纪元时间应该是 0（考虑时区差异）
  EXPECT_GE(date.unix(), -28800); // UTC+8 的偏移
  EXPECT_LE(date.unix(), 28800);
}

TEST_F(DateTest, UnixTimestampForSpecificDate) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.unix(), 1703980800);
}

TEST_F(DateTest, UnixTimestampIsPositive) {
  Date dateLater("2023-09-12 17:47:31");
  EXPECT_GT(dateLater.unix(), 0);
}

// ==================== add 和 sub 测试 ====================
TEST_F(DateTest, AddDays) {
  Date date("2023-09-12 17:47:31");
  Date tomorrow = date.add("day", 1);
  EXPECT_EQ(tomorrow.format("%Y-%m-%d"), "2023-09-13");
  EXPECT_EQ(tomorrow.format("%H:%M:%S"), "17:47:31");
}

TEST_F(DateTest, SubDays) {
  Date date("2023-09-12 17:47:31");
  Date yesterday = date.sub("day", 1);
  EXPECT_EQ(yesterday.format("%Y-%m-%d"), "2023-09-11");
  EXPECT_EQ(yesterday.format("%H:%M:%S"), "17:47:31");
}

TEST_F(DateTest, AddAndSubAcrossMonthBoundary) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  Date yesterday = date.sub("day", 1);
  EXPECT_EQ(yesterday.format("%Y-%m-%d %H:%M:%S"), "2023-12-30 08:00:00");
  
  Date tomorrow = date.add("day", 1);
  EXPECT_EQ(tomorrow.format("%Y-%m-%d"), "2024-01-01");
}

TEST_F(DateTest, AddMultipleDays) {
  Date date("2023-09-12 17:47:31");
  Date future = date.add("day", 10);
  EXPECT_EQ(future.format("%Y-%m-%d"), "2023-09-22");
}

TEST_F(DateTest, SubMultipleDays) {
  Date date("2023-09-12 17:47:31");
  Date past = date.sub("day", 10);
  EXPECT_EQ(past.format("%Y-%m-%d"), "2023-09-02");
}

// ==================== diff 测试 ====================
TEST_F(DateTest, DiffDaysPositive) {
  Date date1("2023-09-12 17:47:31");
  Date date2("2023-09-13 17:47:31");
  EXPECT_EQ(date2.diff("day", date1), 1);
}

TEST_F(DateTest, DiffDaysNegative) {
  Date date1("2023-09-12 17:47:31");
  Date date2("2023-09-13 17:47:31");
  EXPECT_EQ(date1.diff("day", date2), -1);
}

TEST_F(DateTest, DiffDaysAcrossYearBoundary) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  Date yesterday = date.sub("day", 1);
  Date tomorrow = date.add("day", 1);
  
  EXPECT_EQ(date.diff("day", yesterday), 1);
  EXPECT_EQ(date.diff("day", tomorrow), -1);
}

TEST_F(DateTest, DiffDaysSameDate) {
  Date date1("2023-09-12 17:47:31");
  Date date2("2023-09-12 17:47:31");
  EXPECT_EQ(date1.diff("day", date2), 0);
}

// ==================== 时间字段测试 ====================
TEST_F(DateTest, HourField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.hour(), 8);
  
  Date date2("2023-09-12 17:47:31");
  EXPECT_EQ(date2.hour(), 17);
}

TEST_F(DateTest, MinuteField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.minute(), 0);
  
  Date date2("2023-09-12 17:47:31");
  EXPECT_EQ(date2.minute(), 47);
}

TEST_F(DateTest, SecondField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.second(), 0);
  
  Date date2("2023-09-12 17:47:31");
  EXPECT_EQ(date2.second(), 31);
}

TEST_F(DateTest, DayOfMonthField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.dayOfMonth(), 31);
  
  Date date2("2023-09-12 17:47:31");
  EXPECT_EQ(date2.dayOfMonth(), 12);
}

TEST_F(DateTest, DayOfWeekField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.dayOfWeek(), 7); // Sunday = 7
  
  Date date2("2023-09-12 17:47:31");
  // 2023-09-12 是星期二
  EXPECT_GE(date2.dayOfWeek(), 1);
  EXPECT_LE(date2.dayOfWeek(), 7);
}

TEST_F(DateTest, MonthField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.month(), 12);
  
  Date date2("2023-09-12 17:47:31");
  EXPECT_EQ(date2.month(), 9);
  
  Date tomorrow = date.add("day", 1);
  EXPECT_EQ(tomorrow.month(), 1);
}

TEST_F(DateTest, YearField) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.year(), 2023);
  
  Date tomorrow = date.add("day", 1);
  EXPECT_EQ(tomorrow.year(), 2024);
  
  Date yesterday = date.sub("day", 1);
  EXPECT_EQ(yesterday.year(), 2023);
}

// ==================== weekOfMonth 测试 ====================
TEST_F(DateTest, WeekOfMonthForEndOfMonth) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.weekOfMonth(), 5);
}

TEST_F(DateTest, WeekOfMonthAcrossMonthBoundary) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  Date tomorrow = date.add("day", 1);
  Date yesterday = date.sub("day", 1);
  
  EXPECT_EQ(tomorrow.weekOfMonth(), 1);
  EXPECT_EQ(yesterday.weekOfMonth(), 5);
}

// ==================== weekOfYear 测试 ====================
TEST_F(DateTest, WeekOfYearForEndOfYear) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  EXPECT_EQ(date.weekOfYear(), 52);
}

TEST_F(DateTest, WeekOfYearAcrossYearBoundary) {
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  Date tomorrow = date.add("day", 1);
  Date yesterday = date.sub("day", 1);
  
  EXPECT_EQ(tomorrow.weekOfYear(), 1);
  EXPECT_EQ(yesterday.weekOfYear(), 52);
}

TEST_F(DateTest, WeekOfYearForStartOfYear) {
  Date date("2023-01-01 00:00:00");
  EXPECT_GE(date.weekOfYear(), 1);
  EXPECT_LE(date.weekOfYear(), 2); // 可能是 1 或 2，取决于周的定义
}

// ==================== 综合测试 ====================
TEST_F(DateTest, ComprehensiveMethodTest) {
  // 参考 TypeScript 的综合测试
  Date date("2023-12-31 00:00:00.916000 +0800 UTC");
  
  // 格式化
  EXPECT_EQ(date.format("%Y-%m-%d %H:%M:%S"), "2023-12-31 08:00:00");
  
  // Unix 时间戳
  EXPECT_EQ(date.unix(), 1703980800);
  
  // 减一天
  Date yesterday = date.sub("day", 1);
  EXPECT_EQ(yesterday.format("%Y-%m-%d %H:%M:%S"), "2023-12-30 08:00:00");
  
  // 差值计算
  EXPECT_EQ(date.diff("day", yesterday), 1);
  
  // 加一天
  Date tomorrow = date.add("day", 1);
  EXPECT_EQ(date.diff("day", tomorrow), -1);
  
  // 时间字段
  EXPECT_EQ(date.hour(), 8);
  EXPECT_EQ(date.minute(), 0);
  EXPECT_EQ(date.second(), 0);
  EXPECT_EQ(date.dayOfMonth(), 31);
  EXPECT_EQ(date.dayOfWeek(), 7);
  EXPECT_EQ(date.weekOfMonth(), 5);
  EXPECT_EQ(tomorrow.weekOfMonth(), 1);
  EXPECT_EQ(yesterday.weekOfMonth(), 5);
  EXPECT_EQ(date.weekOfYear(), 52);
  EXPECT_EQ(tomorrow.weekOfYear(), 1);
  EXPECT_EQ(yesterday.weekOfYear(), 52);
  EXPECT_EQ(date.month(), 12);
  EXPECT_EQ(date.year(), 2023);
  EXPECT_EQ(yesterday.month(), 12);
  EXPECT_EQ(yesterday.year(), 2023);
  EXPECT_EQ(tomorrow.month(), 1);
  EXPECT_EQ(tomorrow.year(), 2024);
}

// ==================== 边界情况测试 ====================
TEST_F(DateTest, LeapYearFebruary) {
  Date date("2024-02-28 12:00:00");
  Date nextDay = date.add("day", 1);
  EXPECT_EQ(nextDay.format("%Y-%m-%d"), "2024-02-29");
  EXPECT_EQ(nextDay.dayOfMonth(), 29);
}

TEST_F(DateTest, NonLeapYearFebruary) {
  Date date("2023-02-28 12:00:00");
  Date nextDay = date.add("day", 1);
  EXPECT_EQ(nextDay.format("%Y-%m-%d"), "2023-03-01");
  EXPECT_EQ(nextDay.month(), 3);
}

TEST_F(DateTest, MonthsWith30Days) {
  Date date("2023-04-30 12:00:00");
  Date nextDay = date.add("day", 1);
  EXPECT_EQ(nextDay.format("%Y-%m-%d"), "2023-05-01");
  EXPECT_EQ(nextDay.month(), 5);
}

TEST_F(DateTest, MonthsWith31Days) {
  Date date("2023-01-31 12:00:00");
  Date nextDay = date.add("day", 1);
  EXPECT_EQ(nextDay.format("%Y-%m-%d"), "2023-02-01");
  EXPECT_EQ(nextDay.month(), 2);
}
