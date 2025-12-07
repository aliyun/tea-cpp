#ifndef DATE_HPP
#define DATE_HPP

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Darabonba {

class Date {
public:
  // Constructor from string
  explicit Date(const std::string &dateStr) { this->parse(dateStr); }

  // Format the date to a given format
  std::string format(const std::string &fmt) const {
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, fmt.c_str());
    return oss.str();
  }

  // Return UNIX timestamp
  int32_t unix() const {
    std::tm utcTimeInfo = timeInfo;
    // Here we should set `tm` to UTC manually
    utcTimeInfo.tm_isdst = 0;
    std::time_t utcTime = timegm(&utcTimeInfo); // Use `timegm` if available
    return static_cast<int32_t>(utcTime);
  }

  // Subtract time period
  Date sub(const std::string &unit, int value) const {
    std::tm modTimeInfo = timeInfo;
    if (unit == "day") {
      modTimeInfo.tm_mday -= value;
    }
    std::mktime(&modTimeInfo);
    return Date(modTimeInfo);
  }

  // Add time period
  Date add(const std::string &unit, int value) const {
    std::tm modTimeInfo = timeInfo;
    if (unit == "day") {
      modTimeInfo.tm_mday += value;
    }
    std::mktime(&modTimeInfo);
    return Date(modTimeInfo);
  }

  // Difference in time period
  int32_t diff(const std::string &unit, const Date &otherDate) const {
    std::tm thisTimeCopy = timeInfo;
    std::time_t thisTime = std::mktime(&thisTimeCopy);
    std::tm otherTimeCopy = otherDate.timeInfo;
    std::time_t otherTime = std::mktime(&otherTimeCopy);

    double difference = std::difftime(thisTime, otherTime);

    if (unit == "day") {
      return static_cast<int32_t>(difference / (60 * 60 * 24));
    }
    return 0;
  }

  // Get specific time fields
  int32_t hour() const { return timeInfo.tm_hour; }

  int32_t minute() const { return timeInfo.tm_min; }

  int32_t second() const { return timeInfo.tm_sec; }

  int32_t dayOfMonth() const { return timeInfo.tm_mday; }

  int32_t dayOfWeek() const { return timeInfo.tm_wday; }

  int32_t weekOfYear() const { return (timeInfo.tm_yday + 7) / 7; }

  int32_t month() const { return timeInfo.tm_mon + 1; }

  int32_t year() const { return timeInfo.tm_year + 1900; }

  // Convert to UTC
  Date UTC() const {
    std::tm utcTimeInfo = timeInfo;
    std::time_t utcTime = std::mktime(&utcTimeInfo);
    utcTimeInfo = *std::gmtime(&utcTime);
    return Date(utcTimeInfo);
  }

private:
  std::tm timeInfo;

  explicit Date(const std::tm &tmStruct) : timeInfo(tmStruct) {}

  // Parse date string to tm structure
  void parse(const std::string &dateStr) {
    std::istringstream ss(dateStr);
    ss >> std::get_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
      throw std::invalid_argument("Invalid date format");
    }
    timeInfo.tm_isdst = -1;
    std::mktime(&timeInfo);
  }
};

} // namespace Darabonba

#endif
