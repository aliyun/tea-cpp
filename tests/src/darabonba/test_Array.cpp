#include <darabonba/Array.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace Darabonba;

TEST(Darabonba_Array, split) {
  std::vector<std::string> raw = {"1", "2", "3", "4", "5"};
  EXPECT_THAT(Array::split(raw, 0, 2), testing::ElementsAre("1", "2"));
  EXPECT_THAT(Array::split(raw, 1, 4), testing::ElementsAre("2", "3", "4"));
  EXPECT_THAT(Array::split(raw, 2, 5), testing::ElementsAre("3", "4", "5"));
  EXPECT_THAT(Array::split(raw, 0, 0), testing::ElementsAre()); // Empty range
  EXPECT_THAT(Array::split(raw, 5, 5), testing::ElementsAre()); // Empty range

  std::vector<std::string> empty_raw = {};
  EXPECT_THAT(Array::split(empty_raw, 0, 0), testing::ElementsAre());
}

TEST(Darabonba_Array, contains) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  EXPECT_EQ(Array::contains(raw, "apple"), true);
  EXPECT_EQ(Array::contains(raw, "grape"), false);
  EXPECT_EQ(Array::contains(raw, ""), false);

  std::vector<std::string> empty_raw = {};
  EXPECT_EQ(Array::contains(empty_raw, "apple"), false);
}

TEST(Darabonba_Array, index) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  EXPECT_EQ(Array::index(raw, "banana"), 1);
  EXPECT_EQ(Array::index(raw, "grape"), -1);
  EXPECT_EQ(Array::index(raw, ""), -1);

  std::vector<std::string> empty_raw = {};
  EXPECT_EQ(Array::index(empty_raw, "apple"), -1);
}

TEST(Darabonba_Array, size) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  EXPECT_EQ(Array::size(raw), 3);

  std::vector<std::string> empty_raw = {};
  EXPECT_EQ(Array::size(empty_raw), 0);
}

TEST(Darabonba_Array, get) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  EXPECT_EQ(Array::get(raw, 0), "apple");
  EXPECT_EQ(Array::get(raw, 1), "banana");
  EXPECT_EQ(Array::get(raw, 2), "cherry");

  // Test exception handling
  std::vector<std::string> empty_raw = {};
  ASSERT_THROW(Array::get(empty_raw, 0), Darabonba::Exception);

  ASSERT_THROW(Array::get(raw, -1), Darabonba::Exception);
  ASSERT_THROW(Array::get(raw, 3), Darabonba::Exception); // Out of bounds
}

TEST(Darabonba_Array, join) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  EXPECT_EQ(Array::join(raw, ","), "apple,banana,cherry");
  EXPECT_EQ(Array::join(raw, "-"), "apple-banana-cherry");
  EXPECT_EQ(Array::join(raw, ""), "applebananacherry");

  std::vector<std::string> empty_raw = {};
  EXPECT_EQ(Array::join(empty_raw, ","), "");

  std::vector<std::string> single_raw = {"apple"};
  EXPECT_EQ(Array::join(single_raw, ","), "apple");
}

TEST(Darabonba_Array, concat) {
  std::vector<std::string> raw1 = {"apple", "banana"};
  std::vector<std::string> raw2 = {"cherry", "date"};
  std::vector<std::string> expected = {"apple", "banana", "cherry", "date"};
  EXPECT_THAT(Array::concat(raw1, raw2),
              testing::ElementsAre("apple", "banana", "cherry", "date"));

  std::vector<std::string> empty_raw = {};
  EXPECT_THAT(Array::concat(raw1, empty_raw),
              testing::ElementsAre("apple", "banana"));
  EXPECT_THAT(Array::concat(empty_raw, raw2),
              testing::ElementsAre("cherry", "date"));
  EXPECT_THAT(Array::concat(empty_raw, empty_raw), testing::ElementsAre());
}

TEST(Darabonba_Array, ascSort) {
  std::vector<std::string> raw = {"banana", "apple", "cherry"};
  std::vector<std::string> sorted = Array::ascSort(raw);
  EXPECT_THAT(sorted, testing::ElementsAre("apple", "banana", "cherry"));
  // Ensure it sorts in place, checking the original raw is modified
  EXPECT_THAT(raw, testing::ElementsAre("apple", "banana", "cherry"));

  std::vector<std::string> empty_raw = {};
  std::vector<std::string> sorted_empty = Array::ascSort(empty_raw);
  EXPECT_THAT(sorted_empty, testing::ElementsAre());
}

TEST(Darabonba_Array, descSort) {
  std::vector<std::string> raw = {"banana", "apple", "cherry"};
  std::vector<std::string> sorted = Array::descSort(raw);
  EXPECT_THAT(sorted, testing::ElementsAre("cherry", "banana", "apple"));

  // Ensure it sorts in place, checking the original raw is modified
  EXPECT_THAT(raw, testing::ElementsAre("cherry", "banana", "apple"));

  std::vector<std::string> empty_raw = {};
  std::vector<std::string> sorted_empty = Array::descSort(empty_raw);
  EXPECT_THAT(sorted_empty, testing::ElementsAre());
}

TEST(Darabonba_Array, append) {
  std::vector<std::string> raw = {"apple", "banana"};
  Array::append(raw, std::string("cherry")); // 显式转换
  EXPECT_THAT(raw, testing::ElementsAre("apple", "banana", "cherry"));

  std::vector<int> int_raw = {1, 2};
  Array::append(int_raw, 3);
  EXPECT_THAT(int_raw, testing::ElementsAre(1, 2, 3));
}

TEST(Darabonba_Array, shift) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  std::string first = Array::shift(raw);
  EXPECT_EQ(first, "apple");
  EXPECT_THAT(raw, testing::ElementsAre("banana", "cherry"));

  std::vector<std::string> empty_raw = {};
  std::string first_empty = Array::shift(empty_raw);
  EXPECT_EQ(first_empty, "");
  EXPECT_THAT(empty_raw, testing::ElementsAre());
}

TEST(Darabonba_Array, pop) {
  std::vector<std::string> raw = {"apple", "banana", "cherry"};
  std::string last = Array::pop(raw);
  EXPECT_EQ(last, "cherry");
  EXPECT_THAT(raw, testing::ElementsAre("apple", "banana"));

  std::vector<std::string> empty_raw = {};
  std::string last_empty = Array::pop(empty_raw);
  EXPECT_EQ(last_empty, "");
  EXPECT_THAT(empty_raw, testing::ElementsAre());
}

TEST(Darabonba_Array, unshift) {
  std::vector<std::string> raw = {"banana", "cherry"};
  int32_t new_size = Array::unshift(raw, "apple");
  EXPECT_EQ(new_size, 3);
  EXPECT_THAT(raw, testing::ElementsAre("apple", "banana", "cherry"));

  std::vector<std::string> empty_raw = {};
  new_size = Array::unshift(empty_raw, "apple");
  EXPECT_EQ(new_size, 1);
  EXPECT_THAT(empty_raw, testing::ElementsAre("apple"));
}

TEST(Darabonba_Array, push) {
  std::vector<std::string> raw = {"apple", "banana"};
  int32_t new_size = Array::push(raw, "cherry");
  EXPECT_EQ(new_size, 3);
  EXPECT_THAT(raw, testing::ElementsAre("apple", "banana", "cherry"));

  std::vector<std::string> empty_raw = {};
  new_size = Array::push(empty_raw, "apple");
  EXPECT_EQ(new_size, 1);
  EXPECT_THAT(empty_raw, testing::ElementsAre("apple"));
}

TEST(Darabonba_Array, acsSortCopy) {
  std::vector<std::string> raw = {"banana", "apple", "cherry"};
  std::vector<std::string> sorted = Array::acsSort(raw);
  EXPECT_THAT(sorted, testing::ElementsAre("apple", "banana", "cherry"));
  EXPECT_THAT(raw, testing::ElementsAre("banana", "apple", "cherry"));

  std::vector<std::string> empty_raw = {};
  std::vector<std::string> sorted_empty = Array::acsSort(empty_raw);
  EXPECT_THAT(sorted_empty, testing::ElementsAre());
}
