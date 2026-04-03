#include <darabonba/Type.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace Darabonba;

// 测试 DARA_STRING_TEMPLATE 对普通字符串类型的兼容性
TEST(DaraStringTemplate, BasicStringConcatenation) {
  std::string prefix = "pre";
  std::string suffix = "suf";
  
  EXPECT_EQ(DARA_STRING_TEMPLATE(prefix, suffix), "presuf");
  EXPECT_EQ(DARA_STRING_TEMPLATE("hello", " ", "world"), "hello world");
  EXPECT_EQ(DARA_STRING_TEMPLATE("", "middle", ""), "middle");
}

// 测试 DARA_STRING_TEMPLATE 对 int 类型的支持
TEST(DaraStringTemplate, IntConcatenation) {
  int num = 123;
  EXPECT_EQ(DARA_STRING_TEMPLATE("value:", num), "value:123");
  EXPECT_EQ(DARA_STRING_TEMPLATE(num, " items"), "123 items");
}

// 测试 DARA_STRING_TEMPLATE 对 json 字符串类型的处理（核心修复点）
TEST(DaraStringTemplate, JsonStringValueWithoutQuotes) {
  Json strJson = "oss-cn-shanghai.aliyuncs.com";
  
  // 关键测试：json 字符串类型应该输出值本身，而不是带引号的 JSON 格式
  EXPECT_EQ(DARA_STRING_TEMPLATE("bucket.", strJson), "bucket.oss-cn-shanghai.aliyuncs.com");
  EXPECT_EQ(DARA_STRING_TEMPLATE(strJson), "oss-cn-shanghai.aliyuncs.com");
  
  // 模拟真实 SDK 使用场景
  std::string bucketName = "sh-imagesearch-transfer-station";
  Json hostJson = "oss-cn-shanghai.aliyuncs.com";
  EXPECT_EQ(DARA_STRING_TEMPLATE("", bucketName, ".", hostJson), 
            "sh-imagesearch-transfer-station.oss-cn-shanghai.aliyuncs.com");
}

// 测试 DARA_STRING_TEMPLATE 对 json 数字类型的处理
TEST(DaraStringTemplate, JsonNumberValue) {
  Json numJson = 42;
  
  // 数字类型应该正常输出数值
  EXPECT_EQ(DARA_STRING_TEMPLATE("count:", numJson), "count:42");
}

// 测试 DARA_STRING_TEMPLATE 对 json 对象类型的处理
TEST(DaraStringTemplate, JsonObjectValue) {
  Json objJson = Json::object({{"key", "value"}});
  
  // 对象类型应该输出 JSON 格式（因为不是字符串类型）
  EXPECT_EQ(DARA_STRING_TEMPLATE("data:", objJson), "data:{\"key\":\"value\"}");
}

// 测试 DARA_STRING_TEMPLATE 对 json 数组类型的处理
TEST(DaraStringTemplate, JsonArrayValue) {
  Json arrJson = Json::array({"a", "b", "c"});
  
  // 数组类型应该输出 JSON 格式
  EXPECT_EQ(DARA_STRING_TEMPLATE("items:", arrJson), "items:[\"a\",\"b\",\"c\"]");
}

// 测试混合类型拼接
TEST(DaraStringTemplate, MixedTypes) {
  std::string prefix = "result=";
  Json strJson = "success";
  int code = 200;
  
  EXPECT_EQ(DARA_STRING_TEMPLATE(prefix, strJson, ", code=", code), 
            "result=success, code=200");
}

// 测试空 json 字符串
TEST(DaraStringTemplate, EmptyJsonString) {
  Json emptyStr = "";
  
  EXPECT_EQ(DARA_STRING_TEMPLATE("prefix", emptyStr, "suffix"), "prefixsuffix");
}

// 测试 json null 类型
TEST(DaraStringTemplate, JsonNullValue) {
  Json nullJson = nullptr;
  
  EXPECT_EQ(DARA_STRING_TEMPLATE("value:", nullJson), "value:null");
}

// 测试 json boolean 类型
TEST(DaraStringTemplate, JsonBoolValue) {
  Json trueJson = true;
  Json falseJson = false;
  
  EXPECT_EQ(DARA_STRING_TEMPLATE("flag=", trueJson), "flag=true");
  EXPECT_EQ(DARA_STRING_TEMPLATE("flag=", falseJson), "flag=false");
}