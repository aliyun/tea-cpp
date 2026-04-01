#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>
#include <darabonba/Core.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

using namespace Darabonba;
using namespace Darabonba::Http;

// 测试用的子类，暴露 protected 成员用于测试
class TestableMCurlResponseBody : public MCurlResponseBody {
public:
  using MCurlResponseBody::done_;
  using MCurlResponseBody::streamCV_;
  using MCurlResponseBody::doneCV_;
  using MCurlResponseBody::ready_;
};

class MCurlResponseBodyTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}

  virtual void TearDown() override {}
};

// ==================== MCurlResponseBody 基础测试 ====================

TEST_F(MCurlResponseBodyTest, InitialState) {
  TestableMCurlResponseBody body;

  EXPECT_FALSE(body.getDone());
  EXPECT_FALSE(body.getReady());
  EXPECT_EQ(body.getReadableSize(), 0u);
}

TEST_F(MCurlResponseBodyTest, WriteIncreasesReadableSize) {
  TestableMCurlResponseBody body;

  char data[] = "Hello, World!";
  size_t written = body.write(data, strlen(data));

  EXPECT_EQ(written, strlen(data));
  EXPECT_EQ(body.getReadableSize(), strlen(data));
}

TEST_F(MCurlResponseBodyTest, WriteMultipleTimes) {
  TestableMCurlResponseBody body;

  char data1[] = "Hello";
  char data2[] = ", ";
  char data3[] = "World!";

  body.write(data1, strlen(data1));
  EXPECT_EQ(body.getReadableSize(), 5u);

  body.write(data2, strlen(data2));
  EXPECT_EQ(body.getReadableSize(), 7u);

  body.write(data3, strlen(data3));
  EXPECT_EQ(body.getReadableSize(), 13u);
}

TEST_F(MCurlResponseBodyTest, ReadFromEmptyBody) {
  TestableMCurlResponseBody body;
  body.done_ = true;  // 标记为完成

  char buffer[100];
  size_t bytesRead = body.read(buffer, sizeof(buffer));

  EXPECT_EQ(bytesRead, 0u);
}

TEST_F(MCurlResponseBodyTest, WriteAndRead) {
  TestableMCurlResponseBody body;

  char data[] = "Test Data";
  body.write(data, strlen(data));

  char buffer[100] = {0};
  size_t bytesRead = body.read(buffer, sizeof(buffer));

  EXPECT_EQ(bytesRead, strlen(data));
  EXPECT_EQ(std::string(buffer, bytesRead), "Test Data");
  EXPECT_EQ(body.getReadableSize(), 0u);
}

TEST_F(MCurlResponseBodyTest, ReadPartialData) {
  TestableMCurlResponseBody body;

  char data[] = "Hello, World!";
  body.write(data, strlen(data));

  char buffer[5] = {0};
  size_t bytesRead = body.read(buffer, sizeof(buffer));

  EXPECT_EQ(bytesRead, 5u);
  EXPECT_EQ(std::string(buffer, bytesRead), "Hello");
  EXPECT_EQ(body.getReadableSize(), 8u);  // 剩余数据
}

TEST_F(MCurlResponseBodyTest, IsFinished) {
  TestableMCurlResponseBody body;

  EXPECT_FALSE(body.isFinished());

  body.done_ = true;
  EXPECT_TRUE(body.isFinished());

  char data[] = "data";
  body.write(data, strlen(data));
  EXPECT_FALSE(body.isFinished());  // 还有数据未读

  char buffer[100];
  body.read(buffer, sizeof(buffer));
  EXPECT_TRUE(body.isFinished());  // 数据已读完且 done_ 为 true
}

// ==================== 并发测试 ====================

// 测试 write() 方法正确通知 streamCV_
TEST_F(MCurlResponseBodyTest, ConcurrentWriteAndRead) {
  TestableMCurlResponseBody body;
  std::atomic<bool> readCompleted{false};
  std::atomic<bool> writeCompleted{false};

  // 读线程 - 等待数据
  std::thread readThread([&]() {
    char buffer[100] = {0};
    size_t bytesRead = body.read(buffer, sizeof(buffer));
    if (bytesRead > 0) {
      readCompleted = true;
    }
  });

  // 稍微延迟后写入数据
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  char data[] = "Async Data";
  body.write(data, strlen(data));
  writeCompleted = true;

  readThread.join();

  EXPECT_TRUE(writeCompleted);
  EXPECT_TRUE(readCompleted);
}

// 测试多次 write 正确更新 readableSize
TEST_F(MCurlResponseBodyTest, ConcurrentMultipleWrites) {
  TestableMCurlResponseBody body;
  const int numWriters = 3;
  const int writesPerThread = 10;
  std::vector<std::thread> writers;
  std::atomic<int> totalWritten{0};

  for (int i = 0; i < numWriters; ++i) {
    writers.emplace_back([&body, i, &totalWritten]() {
      for (int j = 0; j < writesPerThread; ++j) {
        char data[2] = {static_cast<char>('A' + i), static_cast<char>('0' + j)};
        body.write(data, 2);
        totalWritten += 2;
      }
    });
  }

  for (auto& t : writers) {
    t.join();
  }

  EXPECT_EQ(body.getReadableSize(), static_cast<size_t>(numWriters * writesPerThread * 2));
}

// 测试 done_ 标志和通知机制
TEST_F(MCurlResponseBodyTest, DoneFlagWithConcurrentRead) {
  TestableMCurlResponseBody body;
  std::atomic<bool> readReturned{false};

  // 读线程 - 应该在 done_ 设置后返回
  std::thread readThread([&]() {
    char buffer[100] = {0};
    body.read(buffer, sizeof(buffer));
    readReturned = true;
  });

  // 稍微延迟后设置 done_
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  body.done_ = true;
  body.streamCV_.notify_one();

  readThread.join();

  EXPECT_TRUE(readReturned);
}

// 测试 waitForDone 方法 - 需要有效的 client 和 easyHandle
// 简化测试：只测试 done_ 标志和条件变量
TEST_F(MCurlResponseBodyTest, DoneFlagCanBeSet) {
  TestableMCurlResponseBody body;
  EXPECT_FALSE(body.getDone());
  body.done_ = true;
  EXPECT_TRUE(body.getDone());
}

// ==================== 边界条件测试 ====================

TEST_F(MCurlResponseBodyTest, WriteEmptyData) {
  TestableMCurlResponseBody body;

  char data[] = "";
  size_t written = body.write(data, 0);

  EXPECT_EQ(written, 0u);
  EXPECT_EQ(body.getReadableSize(), 0u);
}

TEST_F(MCurlResponseBodyTest, WriteLargeData) {
  TestableMCurlResponseBody body;

  std::vector<char> largeData(1024 * 1024, 'X');  // 1MB
  size_t written = body.write(largeData.data(), largeData.size());

  EXPECT_EQ(written, largeData.size());
  EXPECT_EQ(body.getReadableSize(), largeData.size());
}

TEST_F(MCurlResponseBodyTest, MaxSizeDefault) {
  TestableMCurlResponseBody body;
  EXPECT_EQ(body.getMaxSize(), 10u * 1024u * 1024u);  // 10MB
}

TEST_F(MCurlResponseBodyTest, ReadAfterMultipleWrites) {
  TestableMCurlResponseBody body;

  char data1[] = "Part1";
  char data2[] = "Part2";
  char data3[] = "Part3";

  body.write(data1, strlen(data1));
  body.write(data2, strlen(data2));
  body.write(data3, strlen(data3));

  char buffer[100] = {0};
  size_t bytesRead = body.read(buffer, sizeof(buffer));

  EXPECT_EQ(bytesRead, 15u);
  EXPECT_EQ(std::string(buffer, bytesRead), "Part1Part2Part3");
}

// ==================== fetch 方法测试 ====================

TEST_F(MCurlResponseBodyTest, FetchWithoutClient) {
  TestableMCurlResponseBody body;
  // 没有 client 和 easyHandle 时应该返回 false
  EXPECT_FALSE(body.fetch());
}

// ==================== getReady/setReady 测试 ====================

TEST_F(MCurlResponseBodyTest, ReadyFlagDefaultFalse) {
  TestableMCurlResponseBody body;
  EXPECT_FALSE(body.getReady());
}

// ==================== 并发读写压力测试 ====================

TEST_F(MCurlResponseBodyTest, StressTestReadWrite) {
  TestableMCurlResponseBody body;
  const int iterations = 100;
  std::atomic<int> totalRead{0};
  std::atomic<int> totalWritten{0};
  std::atomic<bool> doneWriting{false};

  // 写线程
  std::thread writeThread([&]() {
    for (int i = 0; i < iterations; ++i) {
      char data[10];
      snprintf(data, sizeof(data), "msg%03d", i);
      body.write(data, strlen(data));
      totalWritten += strlen(data);
    }
    doneWriting = true;
    body.done_ = true;
    body.streamCV_.notify_one();
  });

  // 读线程
  std::thread readThread([&]() {
    while (true) {
      char buffer[100];
      size_t bytesRead = body.read(buffer, sizeof(buffer));
      if (bytesRead == 0 && body.getDone()) {
        break;
      }
      totalRead += bytesRead;
    }
  });

  writeThread.join();
  readThread.join();

  EXPECT_EQ(totalRead.load(), totalWritten.load());
}

// ==================== 通知机制验证测试 ====================
// 这些测试验证最新的代码改动：write() 正确通知 streamCV_

TEST_F(MCurlResponseBodyTest, WriteNotifiesStreamCV) {
  TestableMCurlResponseBody body;
  std::atomic<bool> dataAvailable{false};
  std::atomic<bool> readStarted{false};

  // 读线程 - 应该在 write 后被通知
  std::thread readThread([&]() {
    readStarted = true;
    char buffer[100];
    // read() 会等待 streamCV_ 通知
    size_t bytesRead = body.read(buffer, sizeof(buffer));
    if (bytesRead > 0) {
      dataAvailable = true;
    }
  });

  // 等待读线程启动
  while (!readStarted.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // 稍微延迟后写入数据 - write() 应该通知 streamCV_
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  char data[] = "test";
  body.write(data, strlen(data));

  readThread.join();

  EXPECT_TRUE(dataAvailable);
}

TEST_F(MCurlResponseBodyTest, MultipleWritesNotifyMultipleReads) {
  TestableMCurlResponseBody body;
  const int numOperations = 5;
  std::atomic<int> readsCompleted{0};

  // 写入初始数据
  char initialData[] = "initial";
  body.write(initialData, strlen(initialData));

  // 读线程
  std::thread readThread([&]() {
    for (int i = 0; i < numOperations; ++i) {
      char buffer[100];
      size_t bytesRead = body.read(buffer, sizeof(buffer));
      if (bytesRead > 0) {
        readsCompleted++;
      }
    }
  });

  // 主线程继续写入
  for (int i = 0; i < numOperations - 1; ++i) {
    char data[20];
    snprintf(data, sizeof(data), "data%d", i);
    body.write(data, strlen(data));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // 标记完成
  body.done_ = true;
  body.streamCV_.notify_one();

  readThread.join();

  // 应该成功读取多次
  EXPECT_GT(readsCompleted.load(), 0);
}