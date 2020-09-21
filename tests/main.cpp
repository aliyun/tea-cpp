#include "gtest/gtest.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(tests, basic) {
  std::string *str = nullptr;
  printf("%d\n", str == nullptr);
  str = new std::string("test");
  int *i = nullptr;
  i = new int(100);
  printf("%s\n", str->c_str());
  printf("%d\n", *i);
}