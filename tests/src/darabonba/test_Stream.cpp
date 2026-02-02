#include <darabonba/Exception.hpp>
#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <vector>

using namespace Darabonba;

class StreamTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

// ==================== readAsBytes 测试 ====================
TEST_F(StreamTest, ReadAsBytesWithValidStream) {
  std::string data = "Hello, World!";
  auto stream = Stream::toReadable(data);

  Bytes bytes = Stream::readAsBytes(stream);

  EXPECT_EQ(bytes.size(), data.size());
  std::string result(bytes.begin(), bytes.end());
  EXPECT_EQ(result, data);
}

TEST_F(StreamTest, ReadAsBytesWithEmptyStream) {
  std::string data = "";
  auto stream = Stream::toReadable(data);

  Bytes bytes = Stream::readAsBytes(stream);

  EXPECT_TRUE(bytes.empty());
}

TEST_F(StreamTest, ReadAsBytesWithNullStream) {
  Bytes bytes = Stream::readAsBytes(nullptr);
  EXPECT_TRUE(bytes.empty());
}

TEST_F(StreamTest, ReadAsBytesWithBinaryData) {
  std::vector<uint8_t> vec = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
  Bytes originalBytes(vec);
  auto stream = Stream::toReadable(originalBytes);

  Bytes bytes = Stream::readAsBytes(stream);

  EXPECT_EQ(bytes.size(), originalBytes.size());
  for (size_t i = 0; i < bytes.size(); ++i) {
    EXPECT_EQ(bytes[i], originalBytes[i]);
  }
}

// ==================== readAsString 测试 ====================
TEST_F(StreamTest, ReadAsStringWithValidStream) {
  std::string data = "Test string content";
  auto stream = Stream::toReadable(data);

  std::string result = Stream::readAsString(stream);

  EXPECT_EQ(result, data);
}

TEST_F(StreamTest, ReadAsStringWithEmptyStream) {
  std::string data = "";
  auto stream = Stream::toReadable(data);

  std::string result = Stream::readAsString(stream);

  EXPECT_TRUE(result.empty());
}

TEST_F(StreamTest, ReadAsStringWithNullStream) {
  std::string result = Stream::readAsString(nullptr);
  EXPECT_TRUE(result.empty());
}

TEST_F(StreamTest, ReadAsStringWithUTF8Content) {
  std::string utf8Data = "Hello, World!";
  auto stream = Stream::toReadable(utf8Data);

  std::string result = Stream::readAsString(stream);

  // cleanString 会移除非打印字符
  EXPECT_FALSE(result.empty());
}

// ==================== readAsJSON 测试 ====================
TEST_F(StreamTest, ReadAsJSONWithValidJSON) {
  std::string jsonStr = "{\"name\":\"test\",\"value\":123}";
  auto stream = Stream::toReadable(jsonStr);

  Json json = Stream::readAsJSON(stream);

  EXPECT_EQ(json["name"], "test");
  EXPECT_EQ(json["value"], 123);
}

TEST_F(StreamTest, ReadAsJSONWithArrayJSON) {
  std::string jsonStr = "[1, 2, 3, 4, 5]";
  auto stream = Stream::toReadable(jsonStr);

  Json json = Stream::readAsJSON(stream);

  EXPECT_TRUE(json.is_array());
  EXPECT_EQ(json.size(), 5u);
  EXPECT_EQ(json[0], 1);
  EXPECT_EQ(json[4], 5);
}

TEST_F(StreamTest, ReadAsJSONWithInvalidJSON) {
  std::string invalidJson = "{invalid json}";
  auto stream = Stream::toReadable(invalidJson);

  // 应该抛出异常
  EXPECT_THROW(Stream::readAsJSON(stream), Darabonba::Exception);
}

TEST_F(StreamTest, ReadAsJSONWithEmptyStream) {
  std::string data = "";
  auto stream = Stream::toReadable(data);

  // 空字符串应该返回空 JSON
  Json json = Stream::readAsJSON(stream);
  EXPECT_TRUE(json.is_null());
}

// ==================== toReadable 测试 ====================
TEST_F(StreamTest, ToReadableFromString) {
  std::string data = "readable content";
  auto stream = Stream::toReadable(data);

  EXPECT_NE(stream, nullptr);

  char buffer[100];
  size_t bytesRead = stream->read(buffer, sizeof(buffer));

  EXPECT_GT(bytesRead, 0u);
  std::string result(buffer, bytesRead);
  EXPECT_EQ(result, data);
}

TEST_F(StreamTest, ToReadableFromBytes) {
  std::vector<uint8_t> vec = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
  Bytes data(vec);
  auto stream = Stream::toReadable(data);

  EXPECT_NE(stream, nullptr);

  char buffer[100];
  size_t bytesRead = stream->read(buffer, sizeof(buffer));

  EXPECT_EQ(bytesRead, data.size());
}

// ==================== toWritable 测试 ====================
TEST_F(StreamTest, ToWritableFromString) {
  std::string data = "";
  auto stream = Stream::toWritable(data);

  EXPECT_NE(stream, nullptr);
}

// ==================== readFromFilePath 测试 ====================
TEST_F(StreamTest, ReadFromFilePathWithNonExistentFile) {
  auto stream = Stream::readFromFilePath("/non/existent/path/file.txt");

  // 文件不存在，但流对象应该被创建
  EXPECT_NE(stream, nullptr);
}

// ==================== readFromBytes 测试 ====================
TEST_F(StreamTest, ReadFromBytesCreatesValidStream) {
  std::vector<uint8_t> vec = {0x01, 0x02, 0x03, 0x04, 0x05};
  Bytes data(vec);
  auto stream = Stream::readFromBytes(data);

  EXPECT_NE(stream, nullptr);
}

// ==================== readFromString 测试 ====================
TEST_F(StreamTest, ReadFromStringCreatesValidStream) {
  std::string data = "test string";
  auto stream = Stream::readFromString(data);

  EXPECT_NE(stream, nullptr);

  char buffer[100];
  size_t bytesRead = stream->read(buffer, sizeof(buffer));

  EXPECT_GT(bytesRead, 0u);
}

// ==================== ISStream 测试 ====================
TEST_F(StreamTest, ISStreamReadWorks) {
  std::string content = "Hello ISStream";
  ISStream stream(content);

  char buffer[100];
  size_t bytesRead = stream.read(buffer, sizeof(buffer));

  EXPECT_GT(bytesRead, 0u);
  std::string result(buffer, bytesRead);
  EXPECT_EQ(result, "Hello ISStream");
}

TEST_F(StreamTest, ISStreamReadWithEOF) {
  std::string content = "short";
  ISStream stream(content);

  char buffer[100];
  stream.read(buffer, sizeof(buffer)); // 读取全部

  // EOF 后再读应返回 0
  size_t bytesRead = stream.read(buffer, sizeof(buffer));
  EXPECT_EQ(bytesRead, 0u);
}

// ==================== OSStream 测试 ====================
TEST_F(StreamTest, OSStreamWriteWorks) {
  std::ostringstream oss;
  OSStream stream(std::move(oss));

  char data[] = "Test data";
  size_t written = stream.write(data, strlen(data));

  EXPECT_EQ(written, strlen(data));
}

// ==================== 边界条件测试 ====================
TEST_F(StreamTest, LargeDataStream) {
  // 创建大量数据
  std::string largeData(10000, 'A');
  auto stream = Stream::toReadable(largeData);

  Bytes bytes = Stream::readAsBytes(stream);

  EXPECT_EQ(bytes.size(), 10000u);
}

TEST_F(StreamTest, SpecialCharactersInStream) {
  std::string data = "Line1\nLine2\tTab\rCarriage";
  auto stream = Stream::toReadable(data);

  std::string result = Stream::readAsString(stream);

  // cleanString 会保留打印字符
  EXPECT_FALSE(result.empty());
}
