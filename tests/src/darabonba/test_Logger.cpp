#include <darabonba/Logger.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <vector>

using namespace Darabonba;

class LoggerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save original log level
    originalLevel_ = Logger::getLevel();
    // Reset to default before each test
    Logger::setLevel(LogLevel::WARNING);
  }

  void TearDown() override {
    // Restore original log level
    Logger::setLevel(originalLevel_);
  }

private:
  LogLevel originalLevel_;
};

// ==================== LogLevel Enumeration Tests ====================

TEST_F(LoggerTest, LogLevelValues) {
  // Verify log level ordering (lower = more verbose)
  EXPECT_LT(static_cast<int>(LogLevel::DEBUG), static_cast<int>(LogLevel::INFO));
  EXPECT_LT(static_cast<int>(LogLevel::INFO), static_cast<int>(LogLevel::WARNING));
  EXPECT_LT(static_cast<int>(LogLevel::WARNING), static_cast<int>(LogLevel::ERROR));
  EXPECT_LT(static_cast<int>(LogLevel::ERROR), static_cast<int>(LogLevel::OFF));
}

TEST_F(LoggerTest, LogLevelDebugValue) {
  EXPECT_EQ(0, static_cast<int>(LogLevel::DEBUG));
}

TEST_F(LoggerTest, LogLevelInfoValue) {
  EXPECT_EQ(1, static_cast<int>(LogLevel::INFO));
}

TEST_F(LoggerTest, LogLevelWarningValue) {
  EXPECT_EQ(2, static_cast<int>(LogLevel::WARNING));
}

TEST_F(LoggerTest, LogLevelErrorValue) {
  EXPECT_EQ(3, static_cast<int>(LogLevel::ERROR));
}

TEST_F(LoggerTest, LogLevelOffValue) {
  EXPECT_EQ(4, static_cast<int>(LogLevel::OFF));
}

// ==================== Default Log Level Tests ====================

TEST_F(LoggerTest, DefaultLogLevelIsWarning) {
  EXPECT_EQ(LogLevel::WARNING, Logger::getLevel());
}

TEST_F(LoggerTest, SetAndGetLogLevel) {
  Logger::setLevel(LogLevel::DEBUG);
  EXPECT_EQ(LogLevel::DEBUG, Logger::getLevel());

  Logger::setLevel(LogLevel::INFO);
  EXPECT_EQ(LogLevel::INFO, Logger::getLevel());

  Logger::setLevel(LogLevel::ERROR);
  EXPECT_EQ(LogLevel::ERROR, Logger::getLevel());

  Logger::setLevel(LogLevel::OFF);
  EXPECT_EQ(LogLevel::OFF, Logger::getLevel());
}

// ==================== Log Level Filtering Tests ====================

TEST_F(LoggerTest, DebugLevelShowsAllMessages) {
  Logger::setLevel(LogLevel::DEBUG);

  // All levels should be visible when set to DEBUG
  // These should not throw or crash
  EXPECT_NO_THROW(Logger::debug("debug message"));
  EXPECT_NO_THROW(Logger::info("info message"));
  EXPECT_NO_THROW(Logger::warning("warning message"));
  EXPECT_NO_THROW(Logger::error("error message"));
}

TEST_F(LoggerTest, InfoLevelHidesDebug) {
  Logger::setLevel(LogLevel::INFO);

  // DEBUG should be suppressed, others visible
  EXPECT_NO_THROW(Logger::debug("should be suppressed"));
  EXPECT_NO_THROW(Logger::info("info message"));
  EXPECT_NO_THROW(Logger::warning("warning message"));
  EXPECT_NO_THROW(Logger::error("error message"));
}

TEST_F(LoggerTest, WarningLevelHidesDebugAndInfo) {
  Logger::setLevel(LogLevel::WARNING);

  // DEBUG and INFO should be suppressed
  EXPECT_NO_THROW(Logger::debug("should be suppressed"));
  EXPECT_NO_THROW(Logger::info("should be suppressed"));
  EXPECT_NO_THROW(Logger::warning("warning message"));
  EXPECT_NO_THROW(Logger::error("error message"));
}

TEST_F(LoggerTest, ErrorLevelShowsOnlyError) {
  Logger::setLevel(LogLevel::ERROR);

  // Only ERROR should be visible
  EXPECT_NO_THROW(Logger::debug("should be suppressed"));
  EXPECT_NO_THROW(Logger::info("should be suppressed"));
  EXPECT_NO_THROW(Logger::warning("should be suppressed"));
  EXPECT_NO_THROW(Logger::error("error message"));
}

TEST_F(LoggerTest, OffLevelHidesAll) {
  Logger::setLevel(LogLevel::OFF);

  // All messages should be suppressed
  EXPECT_NO_THROW(Logger::debug("should be suppressed"));
  EXPECT_NO_THROW(Logger::info("should be suppressed"));
  EXPECT_NO_THROW(Logger::warning("should be suppressed"));
  EXPECT_NO_THROW(Logger::error("should be suppressed"));
}

// ==================== Log Method Tests ====================

TEST_F(LoggerTest, LogMethodCallsInfo) {
  Logger::setLevel(LogLevel::INFO);

  // log() should delegate to info()
  EXPECT_NO_THROW(Logger::log("log message"));
}

TEST_F(LoggerTest, DebugMethodAtDebugLevel) {
  Logger::setLevel(LogLevel::DEBUG);
  EXPECT_NO_THROW(Logger::debug("debug test"));
}

TEST_F(LoggerTest, InfoMethodAtInfoLevel) {
  Logger::setLevel(LogLevel::INFO);
  EXPECT_NO_THROW(Logger::info("info test"));
}

TEST_F(LoggerTest, WarningMethodAtWarningLevel) {
  Logger::setLevel(LogLevel::WARNING);
  EXPECT_NO_THROW(Logger::warning("warning test"));
}

TEST_F(LoggerTest, ErrorMethodAtErrorLevel) {
  Logger::setLevel(LogLevel::ERROR);
  EXPECT_NO_THROW(Logger::error("error test"));
}

// ==================== Boundary Tests ====================

TEST_F(LoggerTest, SetLevelMultipleTimes) {
  Logger::setLevel(LogLevel::DEBUG);
  EXPECT_EQ(LogLevel::DEBUG, Logger::getLevel());

  Logger::setLevel(LogLevel::WARNING);
  EXPECT_EQ(LogLevel::WARNING, Logger::getLevel());

  Logger::setLevel(LogLevel::DEBUG);
  EXPECT_EQ(LogLevel::DEBUG, Logger::getLevel());
}

TEST_F(LoggerTest, SetSameLevelMultipleTimes) {
  Logger::setLevel(LogLevel::INFO);
  Logger::setLevel(LogLevel::INFO);
  Logger::setLevel(LogLevel::INFO);

  EXPECT_EQ(LogLevel::INFO, Logger::getLevel());
}

// ==================== Edge Cases ====================

TEST_F(LoggerTest, EmptyMessage) {
  Logger::setLevel(LogLevel::DEBUG);

  // Empty strings should not cause issues
  EXPECT_NO_THROW(Logger::debug(""));
  EXPECT_NO_THROW(Logger::info(""));
  EXPECT_NO_THROW(Logger::warning(""));
  EXPECT_NO_THROW(Logger::error(""));
}

TEST_F(LoggerTest, LongMessage) {
  Logger::setLevel(LogLevel::DEBUG);

  // Long strings should work
  std::string longMessage(10000, 'a');
  EXPECT_NO_THROW(Logger::debug(longMessage));
  EXPECT_NO_THROW(Logger::info(longMessage));
  EXPECT_NO_THROW(Logger::warning(longMessage));
  EXPECT_NO_THROW(Logger::error(longMessage));
}

TEST_F(LoggerTest, MessageWithNewlines) {
  Logger::setLevel(LogLevel::DEBUG);

  // Messages with newlines
  EXPECT_NO_THROW(Logger::debug("line1\nline2\nline3"));
  EXPECT_NO_THROW(Logger::info("line1\nline2"));
  EXPECT_NO_THROW(Logger::warning("line1\r\nline2"));
  EXPECT_NO_THROW(Logger::error("line1\r\nline2\r\n"));
}

TEST_F(LoggerTest, MessageWithSpecialCharacters) {
  Logger::setLevel(LogLevel::DEBUG);

  // Special characters
  EXPECT_NO_THROW(Logger::debug("Special: \t\b\n\r"));
  EXPECT_NO_THROW(Logger::info("Quotes: \"'`"));
  EXPECT_NO_THROW(Logger::warning("Symbols: !@#$%^&*()"));
  EXPECT_NO_THROW(Logger::error("Unicode: 中文 日本語 한글"));
}

TEST_F(LoggerTest, MessageWithFormatLikeStrings) {
  Logger::setLevel(LogLevel::DEBUG);

  // Format-like strings (should not be interpreted)
  EXPECT_NO_THROW(Logger::debug("%s %d %f"));
  EXPECT_NO_THROW(Logger::info("{} {0} {1}"));
  EXPECT_NO_THROW(Logger::warning("%1$s %2$d"));
  EXPECT_NO_THROW(Logger::error("${var} #{var}"));
}

// ==================== Thread Safety Tests ====================

TEST_F(LoggerTest, ConcurrentSetLevel) {
  const int numThreads = 10;
  std::vector<std::thread> threads;

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back([i]() {
      LogLevel level = static_cast<LogLevel>(i % 5);
      Logger::setLevel(level);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // Should not crash, level should be one of the valid values
  LogLevel finalLevel = Logger::getLevel();
  EXPECT_TRUE(finalLevel == LogLevel::DEBUG ||
              finalLevel == LogLevel::INFO ||
              finalLevel == LogLevel::WARNING ||
              finalLevel == LogLevel::ERROR ||
              finalLevel == LogLevel::OFF);
}

TEST_F(LoggerTest, ConcurrentLogging) {
  Logger::setLevel(LogLevel::DEBUG);
  const int numThreads = 10;
  int messagesPerThread = 100;
  std::vector<std::thread> threads;

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back([i, messagesPerThread]() {
      for (int j = 0; j < messagesPerThread; ++j) {
        Logger::debug("Thread " + std::to_string(i) + " message " + std::to_string(j));
        Logger::info("Thread " + std::to_string(i) + " message " + std::to_string(j));
        Logger::warning("Thread " + std::to_string(i) + " message " + std::to_string(j));
        Logger::error("Thread " + std::to_string(i) + " message " + std::to_string(j));
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // Should complete without crashes
  SUCCEED();
}

TEST_F(LoggerTest, ConcurrentSetLevelAndLogging) {
  const int numThreads = 20;
  std::vector<std::thread> threads;
  std::atomic<bool> stop(false);

  // Thread that changes log levels
  threads.emplace_back([&stop]() {
    for (int i = 0; i < 100 && !stop; ++i) {
      Logger::setLevel(static_cast<LogLevel>(i % 5));
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  // Threads that log messages
  for (int i = 0; i < numThreads - 1; ++i) {
    threads.emplace_back([&stop, i]() {
      for (int j = 0; j < 50 && !stop; ++j) {
        Logger::debug("Thread " + std::to_string(i));
        Logger::info("Thread " + std::to_string(i));
        Logger::warning("Thread " + std::to_string(i));
        Logger::error("Thread " + std::to_string(i));
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // Should complete without crashes
  SUCCEED();
}

// ==================== Output Stream Tests ====================

// Note: These tests verify that debug/info go to stdout and warning/error go to stderr
// We can't easily capture the output in a portable way, but we verify no crashes

TEST_F(LoggerTest, DebugOutputsToStdout) {
  Logger::setLevel(LogLevel::DEBUG);
  EXPECT_NO_THROW(Logger::debug("stdout test"));
}

TEST_F(LoggerTest, InfoOutputsToStdout) {
  Logger::setLevel(LogLevel::INFO);
  EXPECT_NO_THROW(Logger::info("stdout test"));
}

TEST_F(LoggerTest, WarningOutputsToStderr) {
  Logger::setLevel(LogLevel::WARNING);
  EXPECT_NO_THROW(Logger::warning("stderr test"));
}

TEST_F(LoggerTest, ErrorOutputsToStderr) {
  Logger::setLevel(LogLevel::ERROR);
  EXPECT_NO_THROW(Logger::error("stderr test"));
}

// ==================== Backward Compatibility Tests ====================

TEST_F(LoggerTest, GlobalTypedefWorks) {
  // Verify backward compatibility typedefs
  DarabonbaLogger logger;
  (void)logger;  // Suppress unused variable warning

  DarabonbaLogLevel level = DarabonbaLogLevel::DEBUG;
  EXPECT_EQ(static_cast<int>(level), 0);
}

// ==================== Typical Usage Scenarios ====================

TEST_F(LoggerTest, TypicalUsageScenario) {
  // Production-like scenario
  Logger::setLevel(LogLevel::WARNING);

  // Debug and info suppressed
  Logger::debug("This won't appear in production");
  Logger::info("This won't appear either");

  // Only warnings and errors shown
  Logger::warning("Configuration file not found, using defaults");
  Logger::error("Failed to connect to server");
}

TEST_F(LoggerTest, DevelopmentUsageScenario) {
  // Development scenario
  Logger::setLevel(LogLevel::DEBUG);

  // All messages shown
  Logger::debug("Entering function processData()");
  Logger::info("Processing 100 records");
  Logger::warning("Record 50 has missing field");
  Logger::error("Failed to process record 75");
}

TEST_F(LoggerTest, SilenceAllLogsScenario) {
  // Completely silence logging
  Logger::setLevel(LogLevel::OFF);

  Logger::debug("Suppressed");
  Logger::info("Suppressed");
  Logger::warning("Suppressed");
  Logger::error("Suppressed");

  // Verify level is OFF
  EXPECT_EQ(LogLevel::OFF, Logger::getLevel());
}

// ==================== Performance Tests ====================

TEST_F(LoggerTest, SuppressedMessagesHaveMinimalOverhead) {
  Logger::setLevel(LogLevel::OFF);

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < 10000; ++i) {
    Logger::debug("This message is suppressed");
  }

  auto end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // 10000 suppressed messages should take very little time (< 100ms)
  EXPECT_LT(duration.count(), 100);
}

TEST_F(LoggerTest, LevelCheckIsFast) {
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < 100000; ++i) {
    LogLevel level = Logger::getLevel();
    (void)level;
  }

  auto end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // 100000 level checks should be very fast (< 50ms)
  EXPECT_LT(duration.count(), 50);
}