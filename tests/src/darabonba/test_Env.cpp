#include <darabonba/Env.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

class EnvTest : public ::testing::Test {
protected:
  void TearDown() override {
    // 清理测试设置的环境变量
    Env::unsetEnv("DARABONBA_TEST_VAR");
    Env::unsetEnv("DARABONBA_TEST_EMPTY");
  }
};

// ==================== setEnv / getEnv ====================
TEST_F(EnvTest, setAndGetEnv) {
  Env::setEnv("DARABONBA_TEST_VAR", "hello_world");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "hello_world");
}

TEST_F(EnvTest, setEnvOverwrite) {
  Env::setEnv("DARABONBA_TEST_VAR", "first");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "first");

  Env::setEnv("DARABONBA_TEST_VAR", "second");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "second");
}

TEST_F(EnvTest, setEnvEmptyValue) {
  Env::setEnv("DARABONBA_TEST_EMPTY", "");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_EMPTY"), "");
}

// ==================== getEnv 默认值 ====================
TEST_F(EnvTest, getEnvNonExistent) {
  std::string result = Env::getEnv("DARABONBA_NON_EXISTENT_KEY_12345");
  EXPECT_EQ(result, "");
}

TEST_F(EnvTest, getEnvWithDefault) {
  std::string result =
      Env::getEnv("DARABONBA_NON_EXISTENT_KEY_12345", "default_value");
  EXPECT_EQ(result, "default_value");
}

TEST_F(EnvTest, getEnvExistingIgnoresDefault) {
  Env::setEnv("DARABONBA_TEST_VAR", "actual");
  std::string result = Env::getEnv("DARABONBA_TEST_VAR", "default_value");
  EXPECT_EQ(result, "actual");
}

// ==================== unsetEnv ====================
TEST_F(EnvTest, unsetEnv) {
  Env::setEnv("DARABONBA_TEST_VAR", "to_be_removed");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "to_be_removed");

  Env::unsetEnv("DARABONBA_TEST_VAR");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "");
}

TEST_F(EnvTest, unsetEnvNonExistent) {
  // 删除不存在的变量不应崩溃
  Env::unsetEnv("DARABONBA_NEVER_SET_KEY_99999");
}

// ==================== 已有环境变量 ====================
TEST_F(EnvTest, getSystemEnv) {
  // PATH 在所有系统中都存在
  std::string path = Env::getEnv("PATH");
  EXPECT_FALSE(path.empty());
}

TEST_F(EnvTest, getHomeEnv) {
#ifdef _WIN32
  std::string home = Env::getEnv("USERPROFILE");
#else
  std::string home = Env::getEnv("HOME");
#endif
  EXPECT_FALSE(home.empty());
}

// ==================== 特殊字符 ====================
TEST_F(EnvTest, setEnvWithSpecialChars) {
  Env::setEnv("DARABONBA_TEST_VAR", "value=with&special#chars");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "value=with&special#chars");
}

TEST_F(EnvTest, setEnvWithUnicode) {
  Env::setEnv("DARABONBA_TEST_VAR", "中文值");
  EXPECT_EQ(Env::getEnv("DARABONBA_TEST_VAR"), "中文值");
}
