#include <darabonba/Map.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace Darabonba;

// ==================== size (unordered_map) ====================
TEST(Darabonba_Map, sizeUnorderedMap) {
  std::unordered_map<std::string, std::string> m = {
      {"a", "1"}, {"b", "2"}, {"c", "3"}};
  EXPECT_EQ(Map::size(m), 3u);
}

TEST(Darabonba_Map, sizeUnorderedMapEmpty) {
  std::unordered_map<std::string, int> m;
  EXPECT_EQ(Map::size(m), 0u);
}

// ==================== size (map) ====================
TEST(Darabonba_Map, sizeOrderedMap) {
  std::map<std::string, std::string> m = {{"x", "10"}, {"y", "20"}};
  EXPECT_EQ(Map::size(m), 2u);
}

TEST(Darabonba_Map, sizeOrderedMapEmpty) {
  std::map<std::string, int> m;
  EXPECT_EQ(Map::size(m), 0u);
}

// ==================== size (Json) ====================
TEST(Darabonba_Map, sizeJson) {
  Json j;
  j["key1"] = "val1";
  j["key2"] = "val2";
  EXPECT_EQ(Map::size(j), 2u);
}

TEST(Darabonba_Map, sizeJsonEmpty) {
  Json j = Json::object();
  EXPECT_EQ(Map::size(j), 0u);
}

// ==================== keySet (unordered_map) ====================
TEST(Darabonba_Map, keySetUnorderedMap) {
  std::unordered_map<std::string, int> m = {{"a", 1}, {"b", 2}, {"c", 3}};
  auto keys = Map::keySet(m);
  EXPECT_EQ(keys.size(), 3u);
  EXPECT_THAT(keys, testing::UnorderedElementsAre("a", "b", "c"));
}

TEST(Darabonba_Map, keySetUnorderedMapEmpty) {
  std::unordered_map<std::string, int> m;
  auto keys = Map::keySet(m);
  EXPECT_TRUE(keys.empty());
}

// ==================== keySet (map) ====================
TEST(Darabonba_Map, keySetOrderedMap) {
  std::map<std::string, int> m = {{"x", 10}, {"y", 20}, {"z", 30}};
  auto keys = Map::keySet(m);
  EXPECT_EQ(keys.size(), 3u);
  // std::map 有序
  EXPECT_THAT(keys, testing::ElementsAre("x", "y", "z"));
}

TEST(Darabonba_Map, keySetOrderedMapEmpty) {
  std::map<std::string, int> m;
  auto keys = Map::keySet(m);
  EXPECT_TRUE(keys.empty());
}

// ==================== keySet (Json) ====================
TEST(Darabonba_Map, keySetJson) {
  Json j;
  j["alpha"] = 1;
  j["beta"] = 2;
  auto keys = Map::keySet(j);
  EXPECT_EQ(keys.size(), 2u);
  EXPECT_THAT(keys, testing::UnorderedElementsAre("alpha", "beta"));
}

TEST(Darabonba_Map, keySetJsonEmpty) {
  Json j = Json::object();
  auto keys = Map::keySet(j);
  EXPECT_TRUE(keys.empty());
}

// ==================== size with different value types ====================
TEST(Darabonba_Map, sizeUnorderedMapIntValues) {
  std::unordered_map<std::string, int> m = {{"one", 1}, {"two", 2}};
  EXPECT_EQ(Map::size(m), 2u);
}

TEST(Darabonba_Map, sizeOrderedMapDoubleValues) {
  std::map<std::string, double> m = {{"pi", 3.14}, {"e", 2.718}};
  EXPECT_EQ(Map::size(m), 2u);
}
