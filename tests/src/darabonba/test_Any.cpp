#include <darabonba/Any.hpp>
#include <gtest/gtest.h>
#include <string>

// ==================== 默认构造 ====================
TEST(SimpleAny, DefaultConstructor) {
  SimpleAny a;
  EXPECT_EQ(a.type(), typeid(void));
}

// ==================== 值构造 ====================
TEST(SimpleAny, ConstructWithInt) {
  SimpleAny a(42);
  EXPECT_EQ(a.type(), typeid(int));
  EXPECT_EQ(a.cast<int>(), 42);
}

TEST(SimpleAny, ConstructWithDouble) {
  SimpleAny a(3.14);
  EXPECT_EQ(a.type(), typeid(double));
  EXPECT_DOUBLE_EQ(a.cast<double>(), 3.14);
}

TEST(SimpleAny, ConstructWithString) {
  SimpleAny a(std::string("hello"));
  EXPECT_EQ(a.type(), typeid(std::string));
  EXPECT_EQ(a.cast<std::string>(), "hello");
}

TEST(SimpleAny, ConstructWithBool) {
  SimpleAny a(true);
  EXPECT_EQ(a.type(), typeid(bool));
  EXPECT_TRUE(a.cast<bool>());
}

// ==================== 拷贝构造 ====================
TEST(SimpleAny, CopyConstructor) {
  SimpleAny a(42);
  SimpleAny b(a);
  EXPECT_EQ(b.type(), typeid(int));
  EXPECT_EQ(b.cast<int>(), 42);
}

TEST(SimpleAny, CopyConstructorFromDefault) {
  SimpleAny a;
  SimpleAny b(a);
  EXPECT_EQ(b.type(), typeid(void));
}

// ==================== 赋值操作 ====================
TEST(SimpleAny, AssignmentOperator) {
  SimpleAny a(42);
  SimpleAny b;
  b = a;
  EXPECT_EQ(b.type(), typeid(int));
  EXPECT_EQ(b.cast<int>(), 42);
}

TEST(SimpleAny, SelfAssignment) {
  SimpleAny a(42);
  SimpleAny &ref = a;
  a = ref; // 通过引用避免 -Wself-assign-overloaded
  EXPECT_EQ(a.type(), typeid(int));
  EXPECT_EQ(a.cast<int>(), 42);
}

TEST(SimpleAny, AssignDifferentTypes) {
  SimpleAny a(42);
  SimpleAny b(std::string("world"));
  a = b;
  EXPECT_EQ(a.type(), typeid(std::string));
  EXPECT_EQ(a.cast<std::string>(), "world");
}

// ==================== cast 类型不匹配 ====================
TEST(SimpleAny, CastWrongType) {
  SimpleAny a(42);
  EXPECT_THROW(a.cast<std::string>(), std::bad_cast);
}

TEST(SimpleAny, CastWrongTypeDouble) {
  SimpleAny a(3.14);
  EXPECT_THROW(a.cast<int>(), std::bad_cast);
}

// ==================== type ====================
TEST(SimpleAny, TypeInfo) {
  SimpleAny a(42);
  EXPECT_EQ(a.type(), typeid(int));

  SimpleAny b(std::string("test"));
  EXPECT_EQ(b.type(), typeid(std::string));

  SimpleAny c;
  EXPECT_EQ(c.type(), typeid(void));
}

// ==================== 更换存储类型 ====================
TEST(SimpleAny, ReassignDifferentType) {
  SimpleAny a(42);
  EXPECT_EQ(a.cast<int>(), 42);

  a = SimpleAny(std::string("changed"));
  EXPECT_EQ(a.cast<std::string>(), "changed");
}

// ==================== 复杂类型 ====================
TEST(SimpleAny, StoreVector) {
  std::vector<int> vec = {1, 2, 3};
  SimpleAny a(vec);
  EXPECT_EQ(a.type(), typeid(std::vector<int>));
  auto result = a.cast<std::vector<int>>();
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], 1);
}
