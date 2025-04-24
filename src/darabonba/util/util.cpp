#include <chrono>
#include <ctime>
#include <darabonba/Core.hpp>
#include <darabonba/Model.hpp>
#include <darabonba/Util.hpp>
#include <iostream>
#include <map>
#include <thread>

using std::string;

namespace Darabonba {

string Util::getDateUTCString() {
  char buf[80];
  time_t t = time(nullptr);
  std::strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
  return buf;
}

static string osName() {
#if defined(_WIN64) || defined(__MINGW64__)
  return "Windows64";
#elif defined(_WIN32) || defined(__MINGW32__)
  return "Windows32";
#elif defined(__APPLE__) || defined(__MACH__)
  return "MacOS";
#elif defined(__linux__)
  return "Linux";
#elif defined(__FreeBSD__)
  return "FreeBSD";
#elif defined(__NetBSD__)
  return "NetBSD";
#elif defined(__unix) || defined(__unix__)
  return "Unix";
#elif defined(__ANDROID__)
  return "Android";
#else
  return "Other";
#endif
}

string Util::getUserAgent(const string &userAgent) {
  string defaultUserAgent = "Alibabacloud OS/" + osName() + " DaraDSL/1";
  if (userAgent.empty()) {
    return defaultUserAgent;
  }
  return defaultUserAgent + userAgent;
}

} // namespace Darabonba