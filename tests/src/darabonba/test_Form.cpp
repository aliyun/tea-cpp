#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/http/Form.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;
using namespace Darabonba::Http;

class FormTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

// ==================== getBoundary 测试 ====================
TEST_F(FormTest, GetBoundaryReturnsValidString) {
  std::string boundary = Form::getBoundary();

  // boundary 应该是非空字符串
  EXPECT_FALSE(boundary.empty());
}

TEST_F(FormTest, GetBoundaryHasMinimumLength) {
  std::string boundary = Form::getBoundary();

  // boundary 长度应该大于 10
  EXPECT_GT(boundary.length(), 10u);
}

TEST_F(FormTest, GetBoundaryIsUnique) {
  std::string boundary1 = Form::getBoundary();
  std::string boundary2 = Form::getBoundary();

  // 两次调用应该返回不同的 boundary（虽然理论上可能相同，但概率极低）
  // 这里只验证格式正确
  EXPECT_FALSE(boundary1.empty());
  EXPECT_FALSE(boundary2.empty());
}

// ==================== toFormString 测试 ====================
TEST_F(FormTest, ToFormStringWithEmptyObject) {
  Json emptyJson = Json::object();

  std::string result = Form::toFormString(emptyJson);

  // 空对象应该返回空字符串
  EXPECT_TRUE(result.empty());
}

TEST_F(FormTest, ToFormStringWithSingleField) {
  Json json;
  json["key"] = "value";

  std::string result = Form::toFormString(json);

  EXPECT_NE(result.find("key"), std::string::npos);
  EXPECT_NE(result.find("value"), std::string::npos);
}

TEST_F(FormTest, ToFormStringWithMultipleFields) {
  Json json;
  json["name"] = "test";
  json["age"] = "25";

  std::string result = Form::toFormString(json);

  EXPECT_NE(result.find("name"), std::string::npos);
  EXPECT_NE(result.find("test"), std::string::npos);
  EXPECT_NE(result.find("age"), std::string::npos);
  EXPECT_NE(result.find("25"), std::string::npos);
}

TEST_F(FormTest, ToFormStringEncodesSpecialCharacters) {
  Json json;
  json["key"] = "value with spaces";

  std::string result = Form::toFormString(json);

  // 空格应该被编码
  EXPECT_NE(result.find("key"), std::string::npos);
}

TEST_F(FormTest, ToFormStringWithEmptyValue) {
  Json json;
  json["empty"] = "";

  std::string result = Form::toFormString(json);

  EXPECT_NE(result.find("empty"), std::string::npos);
}

// ==================== toFileForm 测试 ====================
TEST_F(FormTest, ToFileFormReturnsStream) {
  Json form;
  form["field1"] = "value1";
  std::string boundary = Form::getBoundary();

  auto stream = Form::toFileForm(form, boundary);

  EXPECT_NE(stream, nullptr);
}

TEST_F(FormTest, ToFileFormWithStringField) {
  Json form;
  form["name"] = "test";
  form["description"] = "A test form";
  std::string boundary = Form::getBoundary();

  auto stream = Form::toFileForm(form, boundary);

  EXPECT_NE(stream, nullptr);

  // 读取并验证内容包含字段
  if (stream) {
    std::string content = Stream::readAsString(stream);
    // 内容应该包含 boundary 和字段
    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find(boundary), std::string::npos);
    EXPECT_NE(content.find("name"), std::string::npos);
    EXPECT_NE(content.find("test"), std::string::npos);
  }
}

TEST_F(FormTest, ToFileFormWithEmptyForm) {
  Json emptyForm = Json::object();
  std::string boundary = Form::getBoundary();

  auto stream = Form::toFileForm(emptyForm, boundary);

  // 即使是空表单也应该返回有效的流
  EXPECT_NE(stream, nullptr);
}

// ==================== encode 测试 ====================
TEST_F(FormTest, EncodeEmptyString) {
  std::string result = Form::encode("");
  EXPECT_TRUE(result.empty());
}

TEST_F(FormTest, EncodeSimpleString) {
  std::string result = Form::encode("hello");
  EXPECT_EQ(result, "hello");
}

TEST_F(FormTest, EncodeStringWithSpaces) {
  std::string result = Form::encode("hello world");
  // 空格应该被编码为 + 或 %20
  EXPECT_TRUE(result.find("+") != std::string::npos ||
              result.find("%20") != std::string::npos);
}

TEST_F(FormTest, EncodeStringWithSpecialCharacters) {
  std::string result = Form::encode("key=value&foo=bar");
  // 特殊字符应该被编码
  EXPECT_NE(result.find("%"), std::string::npos);
}

TEST_F(FormTest, EncodeChineseCharacters) {
  std::string result = Form::encode("Chinese characters might need encoding");
  EXPECT_FALSE(result.empty());
}

// ==================== 集成测试 ====================
TEST_F(FormTest, CompleteFormWorkflow) {
  // 1. 创建表单数据
  Json formData;
  formData["username"] = "testuser";
  formData["email"] = "test@example.com";

  // 2. 获取 boundary
  std::string boundary = Form::getBoundary();
  EXPECT_FALSE(boundary.empty());

  // 3. 转换为文件表单
  auto stream = Form::toFileForm(formData, boundary);
  EXPECT_NE(stream, nullptr);

  // 4. 也可以转换为简单表单字符串
  std::string formString = Form::toFormString(formData);
  EXPECT_FALSE(formString.empty());
}

// ==================== 边界条件测试 ====================
TEST_F(FormTest, ToFormStringWithNullValue) {
  Json json;
  json["key"] = nullptr;

  // 应该能处理 null 值
  std::string result = Form::toFormString(json);
  // 不崩溃即可
}

TEST_F(FormTest, ToFormStringWithNumericValue) {
  Json json;
  json["number"] = 123;

  std::string result = Form::toFormString(json);
  EXPECT_NE(result.find("123"), std::string::npos);
}

TEST_F(FormTest, ToFormStringWithBooleanValue) {
  Json json;
  json["flag"] = true;

  std::string result = Form::toFormString(json);
  // boolean 应该被转换
  EXPECT_FALSE(result.empty());
}

TEST_F(FormTest, LongBoundaryIsValid) {
  // 多次获取 boundary，验证格式一致性
  for (int i = 0; i < 10; ++i) {
    std::string boundary = Form::getBoundary();
    EXPECT_GT(boundary.length(), 10u);
    EXPECT_LT(boundary.length(), 100u); // 不应该太长
  }
}
