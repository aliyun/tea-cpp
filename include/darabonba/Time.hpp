#ifndef DARABONBA_TIME_H_
#define DARABONBA_TIME_H_

#include <chrono>
#include <ctime>
#include <darabonba/String.hpp>
#include <string>
#include <thread>

namespace Darabonba {

class Time {
public:
  static std::string unix() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch())
                  .count();
    return std::to_string(ms / 1000);
  }

  static std::string UTC() {
    time_t now = time(nullptr);
    char buf[100];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.000000 +0000 UTC",
             gmtime(&now));
    return buf;
  }

  static void sleep(long milliseconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(milliseconds));
  }

  static std::string format(std::string layout) {
    layout = String::replace(layout, "yyyy", "%Y");
    layout = String::replace(layout, "MM", "%m");
    layout = String::replace(layout, "dd", "%d");
    layout = String::replace(layout, "hh", "%H");
    layout = String::replace(layout, "mm", "%M");
    layout = String::replace(layout, "a", "%p");
    layout = String::replace(layout, "EEEE", "%A");
    time_t now = time(nullptr);
    char buf[512];
    strftime(buf, sizeof(buf), layout.c_str(), gmtime(&now));
    return buf;
  }
};
} // namespace Darabonba
#endif