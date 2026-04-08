#ifndef DARABONBA_LOGGER_H_
#define DARABONBA_LOGGER_H_

#include <iostream>
#include <string>
#include <atomic>

// Windows SDK defines ERROR, WARNING, DEBUG, INFO as macros which conflict
// with our enum values. Temporarily undef them during this header, then restore.
#ifdef _WIN32
  #pragma push_macro("ERROR")
  #pragma push_macro("WARNING")
  #pragma push_macro("DEBUG")
  #pragma push_macro("INFO")
  #undef ERROR
  #undef WARNING
  #undef DEBUG
  #undef INFO
#endif

namespace Darabonba {

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
  DEBUG = 0,
  INFO = 1,
  WARNING = 2,
  ERROR = 3,
  OFF = 4  // Disable all logging
};

/**
 * @brief Simple logger with configurable log level
 *
 * Usage:
 *   // Set log level (default is WARNING)
 *   Darabonba::Logger::setLevel(Darabonba::LogLevel::DEBUG);
 *
 *   // Log messages
 *   Darabonba::Logger::debug("Debug message");
 *   Darabonba::Logger::warning("Warning message");
 *
 *   // Disable logging
 *   Darabonba::Logger::setLevel(Darabonba::LogLevel::OFF);
 */
class Logger {
public:
  /**
   * @brief Set the global log level
   * @param newLevel Messages below this level will be suppressed
   */
  static void setLevel(LogLevel newLevel) {
    level().store(newLevel);
  }

  /**
   * @brief Get the current log level
   */
  static LogLevel getLevel() {
    return level().load();
  }

  static void log(const std::string &val) {
    info(val);
  }

  static void info(const std::string &val) {
    if (level().load() <= LogLevel::INFO) {
      std::cout << "[INFO] " << val << std::endl;
    }
  }

  static void warning(const std::string &val) {
    if (level().load() <= LogLevel::WARNING) {
      std::cerr << "[WARNING] " << val << std::endl;
    }
  }

  static void debug(const std::string &val) {
    if (level().load() <= LogLevel::DEBUG) {
      std::cout << "[DEBUG] " << val << std::endl;
    }
  }

  static void error(const std::string &val) {
    if (level().load() <= LogLevel::ERROR) {
      std::cerr << "[ERROR] " << val << std::endl;
    }
  }

private:
  static std::atomic<LogLevel>& level() {
    static std::atomic<LogLevel> instance(LogLevel::WARNING);
    return instance;
  }
};

} // namespace Darabonba

// Backward compatibility: allow global access without namespace
using DarabonbaLogger = Darabonba::Logger;
using DarabonbaLogLevel = Darabonba::LogLevel;

// Restore Windows macros
#ifdef _WIN32
  #pragma pop_macro("INFO")
  #pragma pop_macro("DEBUG")
  #pragma pop_macro("WARNING")
  #pragma pop_macro("ERROR")
#endif

#endif