#include <darabonba/Exception.hpp>
#include <darabonba/Type.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

class ExceptionTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

// ==================== DaraException 基类测试 ====================
TEST_F(ExceptionTest, InitBaseDaraExceptionWithMessage) {
  DaraException ex("Test error message");

  EXPECT_EQ(ex.getMessage(), "Test error message");
  EXPECT_EQ(ex.getName(), "DaraException");
}

TEST_F(ExceptionTest, InitBaseDaraExceptionDefault) {
  DaraException ex;

  EXPECT_TRUE(ex.getMessage().empty());
  EXPECT_EQ(ex.getName(), "DaraException");
}

TEST_F(ExceptionTest, DaraExceptionFromJsonSerialization) {
  // 测试 from_json/to_json
  DaraException ex("Original message");
  Json j;
  to_json(j, ex);

  EXPECT_EQ(j["message"], "Original message");

  DaraException ex2;
  from_json(j, ex2);
  EXPECT_EQ(ex2.getMessage(), "Original message");
}

TEST_F(ExceptionTest, DaraExceptionWhat) {
  DaraException ex("Error message");

  std::string what = ex.what();
  EXPECT_FALSE(what.empty());
  EXPECT_NE(what.find("Error message"), std::string::npos);
}

TEST_F(ExceptionTest, DaraExceptionCopyConstructor) {
  DaraException ex1("Original error");
  DaraException ex2(ex1);

  EXPECT_EQ(ex2.getMessage(), ex1.getMessage());
  EXPECT_EQ(ex2.getName(), ex1.getName());
}

TEST_F(ExceptionTest, DaraExceptionMoveConstructor) {
  DaraException ex1("Move test");
  DaraException ex2(std::move(ex1));

  EXPECT_EQ(ex2.getMessage(), "Move test");
}

TEST_F(ExceptionTest, DaraExceptionAssignment) {
  DaraException ex1("First error");
  DaraException ex2("Second error");
  ex2 = ex1;

  EXPECT_EQ(ex2.getMessage(), "First error");
}

TEST_F(ExceptionTest, DaraExceptionMoveAssignment) {
  DaraException ex1("Move assign test");
  DaraException ex2("Original");
  ex2 = std::move(ex1);

  EXPECT_EQ(ex2.getMessage(), "Move assign test");
}

// ==================== ResponseException 测试 ====================
TEST_F(ExceptionTest, InitResponseExceptionDefault) {
  ResponseException ex;

  EXPECT_EQ(ex.getName(), "ResponseException");
}

TEST_F(ExceptionTest, InitResponseExceptionWithJSON) {
  Json json;
  json["message"] = "Response error";
  json["code"] = "RESP_ERROR";
  json["statusCode"] = 500;
  json["retryAfter"] = 30;

  ResponseException ex(json);

  EXPECT_EQ(ex.getMessage(), "Response error");
  EXPECT_EQ(ex.getCode(), "RESP_ERROR");
  EXPECT_EQ(ex.getStatusCode(), 500);
  EXPECT_EQ(ex.getRetryAfter(), 30);
}

TEST_F(ExceptionTest, ResponseExceptionGetters) {
  Json json;
  json["message"] = "Test";
  json["statusCode"] = 404;

  ResponseException ex(json);

  EXPECT_EQ(ex.getStatusCode(), 404);
}

TEST_F(ExceptionTest, ResponseExceptionDescription) {
  Json json;
  json["message"] = "Error msg";
  json["code"] = "ERR";
  json["statusCode"] = 503;
  json["description"] = "Service temporarily unavailable";

  ResponseException ex(json);

  EXPECT_EQ(ex.getDescription(), "Service temporarily unavailable");
}

TEST_F(ExceptionTest, ResponseExceptionAccessDeniedDetail) {
  Json json;
  json["message"] = "Access denied";
  json["code"] = "AccessDenied";
  json["statusCode"] = 403;
  json["accessDeniedDetail"] = "Insufficient permissions";

  ResponseException ex(json);

  EXPECT_EQ(ex.getAccessDeniedDetail(), "Insufficient permissions");
}

TEST_F(ExceptionTest, ResponseExceptionCopyConstructor) {
  Json json;
  json["message"] = "Copy test";
  json["statusCode"] = 500;

  ResponseException ex1(json);
  ResponseException ex2(ex1);

  EXPECT_EQ(ex2.getMessage(), "Copy test");
  EXPECT_EQ(ex2.getStatusCode(), 500);
}

TEST_F(ExceptionTest, ResponseExceptionMoveConstructor) {
  Json json;
  json["message"] = "Move test";
  json["statusCode"] = 404;

  ResponseException ex1(json);
  ResponseException ex2(std::move(ex1));

  EXPECT_EQ(ex2.getMessage(), "Move test");
  EXPECT_EQ(ex2.getStatusCode(), 404);
}

// ==================== ValidateException 测试 ====================
TEST_F(ExceptionTest, InitValidateExceptionWithCodeAndMessage) {
  // ValidateException 构造函数参数是 (code, message)，name_ 固定为
  // "ValidateException"
  ValidateException ex("FieldValidateError", "Field 'name' is required");

  EXPECT_EQ(ex.getName(), "ValidateException");
  // getMessage() 返回的是 code + ": " + msg
  EXPECT_NE(ex.getMessage().find("FieldValidateError"), std::string::npos);
  EXPECT_NE(ex.getMessage().find("Field 'name' is required"),
            std::string::npos);
}

TEST_F(ExceptionTest, ValidateExceptionInheritance) {
  ValidateException ex("RequiredFieldError", "Required field missing");

  // ValidateException 继承自 DaraException
  DaraException &baseRef = ex;
  EXPECT_NE(baseRef.getMessage().find("Required field missing"),
            std::string::npos);
}

TEST_F(ExceptionTest, ValidateExceptionDefaultConstructor) {
  ValidateException ex;

  EXPECT_EQ(ex.getName(), "ValidateException");
}

// ==================== RequiredArgumentException 测试 ====================
TEST_F(ExceptionTest, InitRequiredArgumentException) {
  RequiredArgumentException ex("username");

  std::string what = ex.what();
  EXPECT_FALSE(what.empty());
  // 验证包含参数名
  EXPECT_NE(std::string(what).find("username"), std::string::npos);
}

// ==================== RetryError 测试 ====================
TEST_F(ExceptionTest, InitRetryErrorWithMessage) {
  RetryError err("Retry limit exceeded");

  std::string what = err.what();
  EXPECT_NE(std::string(what).find("Retry limit exceeded"), std::string::npos);
}

TEST_F(ExceptionTest, RetryErrorInheritance) {
  RetryError err("Maximum retries reached");

  // RetryError 继承自 std::exception
  std::exception &baseRef = err;
  std::string what = baseRef.what();
  EXPECT_FALSE(what.empty());
}

// ==================== 异常抛出和捕获测试 ====================
TEST_F(ExceptionTest, ThrowAndCatchDaraException) {
  try {
    throw DaraException("Test throw");
    FAIL() << "Expected exception to be thrown";
  } catch (const DaraException &e) {
    EXPECT_EQ(e.getMessage(), "Test throw");
  }
}

TEST_F(ExceptionTest, ThrowAndCatchResponseException) {
  try {
    Json json;
    json["message"] = "HTTP Error";
    json["statusCode"] = 502;
    throw ResponseException(json);
    FAIL() << "Expected exception to be thrown";
  } catch (const ResponseException &e) {
    EXPECT_EQ(e.getStatusCode(), 502);
  }
}

TEST_F(ExceptionTest, CatchResponseExceptionAsBaseDaraException) {
  try {
    Json json;
    json["message"] = "Server Error";
    json["statusCode"] = 500;
    throw ResponseException(json);
  } catch (const DaraException &e) {
    // ResponseException 可以被捕获为 DaraException
    EXPECT_EQ(e.getName(), "ResponseException");
  }
}

TEST_F(ExceptionTest, CatchValidateExceptionAsBaseDaraException) {
  try {
    throw ValidateException("ValidationError", "Invalid input");
  } catch (const DaraException &e) {
    // ValidateException 可以被捕获为 DaraException
    EXPECT_NE(e.getMessage().find("Invalid input"), std::string::npos);
  }
}

TEST_F(ExceptionTest, CatchAsStdException) {
  try {
    throw DaraException("Standard exception test");
  } catch (const std::exception &e) {
    // DaraException 继承自 std::exception
    std::string what = e.what();
    EXPECT_FALSE(what.empty());
  }
}

TEST_F(ExceptionTest, ThrowAndCatchRequiredArgumentException) {
  try {
    throw RequiredArgumentException("apiKey");
    FAIL() << "Expected exception to be thrown";
  } catch (const DaraException &e) {
    EXPECT_NE(std::string(e.what()).find("apiKey"), std::string::npos);
  }
}

TEST_F(ExceptionTest, ThrowAndCatchRetryError) {
  try {
    throw RetryError("Max retries exceeded");
    FAIL() << "Expected exception to be thrown";
  } catch (const std::exception &e) {
    EXPECT_NE(std::string(e.what()).find("Max retries exceeded"),
              std::string::npos);
  }
}

// ==================== 边界条件测试 ====================
TEST_F(ExceptionTest, DaraExceptionWithEmptyMessage) {
  DaraException ex("");
  EXPECT_TRUE(ex.getMessage().empty());
}

TEST_F(ExceptionTest, DaraExceptionWithLongMessage) {
  std::string longMessage(1000, 'A');
  DaraException ex(longMessage);

  EXPECT_EQ(ex.getMessage().length(), 1000u);
}

TEST_F(ExceptionTest, ResponseExceptionWithEmptyJSON) {
  Json emptyJson;
  ResponseException ex(emptyJson);

  // 应该有默认值
  EXPECT_EQ(ex.getName(), "ResponseException");
  EXPECT_EQ(ex.getStatusCode(), 0);
  EXPECT_EQ(ex.getRetryAfter(), 0);
  EXPECT_TRUE(ex.getMessage().empty());
}

TEST_F(ExceptionTest, ResponseExceptionWithPartialJSON) {
  Json json;
  json["statusCode"] = 500;
  // 没有 message、code 等字段

  ResponseException ex(json);

  EXPECT_EQ(ex.getStatusCode(), 500);
  EXPECT_TRUE(ex.getMessage().empty());
  EXPECT_TRUE(ex.getCode().empty());
}

// ==================== retryAfter 测试 ====================
TEST_F(ExceptionTest, RetryAfterInitializedToZero) {
  DaraException ex("test");
  EXPECT_EQ(ex.getRetryAfter(), 0);
}

TEST_F(ExceptionTest, ResponseExceptionRetryAfterFromJSON) {
  Json json;
  json["retryAfter"] = 60;

  ResponseException ex(json);
  EXPECT_EQ(ex.getRetryAfter(), 60);
}

TEST_F(ExceptionTest, ResponseExceptionRetryAfterZeroWhenNotSet) {
  Json json;
  json["message"] = "No retry after set";

  ResponseException ex(json);
  EXPECT_EQ(ex.getRetryAfter(), 0);
}

// ==================== Getters 测试 ====================
TEST_F(ExceptionTest, DaraExceptionGetName) {
  DaraException ex("test");
  EXPECT_EQ(ex.getName(), "DaraException");
}

TEST_F(ExceptionTest, DaraExceptionGetCode) {
  DaraException ex("test");
  // code_ 默认为空
  EXPECT_TRUE(ex.getCode().empty());
}

TEST_F(ExceptionTest, ResponseExceptionWhatsContainsSDKError) {
  Json json;
  json["message"] = "Test message";
  json["code"] = "TestCode";
  json["statusCode"] = 400;

  ResponseException ex(json);
  std::string what = ex.what();

  // msg_ 应该包含 SDKError 格式
  EXPECT_NE(what.find("SDKError"), std::string::npos);
  EXPECT_NE(what.find("400"), std::string::npos);
}
