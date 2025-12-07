#include <darabonba/Date.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

TEST(Darabonba_Date, Format) {
  Date date("2023-09-12 17:47:31");
  EXPECT_EQ(date.format("%Y-%m-%d %H:%M:%S"), "2023-09-12 17:47:31");
}

TEST(Darabonba_Date, UnixTimestamp) {
  Date date("1970-01-01 00:00:00");
  EXPECT_EQ(date.unix(), 0);
  Date dateLater("2023-09-12 17:47:31");
  EXPECT_GT(dateLater.unix(), 0);
}

TEST(Darabonba_Date, AddAndSubDays) {
  Date date("2023-09-12 17:47:31");
  Date tomorrow = date.add("day", 1);
  EXPECT_EQ(tomorrow.format("%Y-%m-%d"), "2023-09-13");

  Date yesterday = date.sub("day", 1);
  EXPECT_EQ(yesterday.format("%Y-%m-%d"), "2023-09-11");
}

TEST(Darabonba_Date, DiffDays) {
  Date date1("2023-09-12 17:47:31");
  Date date2("2023-09-13 17:47:31");
  EXPECT_EQ(date2.diff("day", date1), 1);
  EXPECT_EQ(date1.diff("day", date2), -1);
}

// TEST(Darabonba_Date, TimeFields) {
//     Date date("2023-09-12 17:47:31");
//     EXPECT_EQ(date.hour(), 17);
//     EXPECT_EQ(date.minute(), 47);
//     EXPECT_EQ(date.second(), 31);
//     EXPECT_EQ(date.dayOfMonth(), 12);
//     EXPECT_EQ(date.dayOfWeek(), 2); // Tuesday (0 = Sunday)
//     EXPECT_EQ(date.month(), 9);
//     EXPECT_EQ(date.year(), 2023);
// }

// TEST(Darabonba_Date, UTCConversion) {
//     Date date("2023-09-12 17:47:31");
//     Date utcDate = date.UTC();
//     // Since the input date string doesn't include timezone details,
//     // only the structural conversion is being tested here.
//     EXPECT_EQ(utcDate.format("%Y-%m-%d %H:%M:%S"), date.format("%Y-%m-%d
//     %H:%M:%S"));
// }

TEST(Darabonba_Date, WeekOfYearCalculation) {
  setenv("TZ", "UTC", 1);
  tzset();
  Date date("2023-01-01 00:00:00");
  EXPECT_EQ(date.weekOfYear(),
            1); // Might vary depending on the week-start conventions
}
