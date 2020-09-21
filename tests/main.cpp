#include "gtest/gtest.h"
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <iostream>

using namespace std;

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

class MergeTest {
public:
  template <typename... Params>
  static std::map<std::string, boost::any>
  merge(std::map<std::string, boost::any> *m, Params... parameters) {
    if (m == nullptr) {
      m = new std::map<std::string, boost::any>();
    }
    mergeMap(m, parameters...);
    return *m;
  }

private:
  template <typename... Params>
  static void mergeMap(std::map<std::string, boost::any> *m,
                       std::map<std::string, boost::any> value,
                       Params... parameters) {
    assignMap(m, value);
    mergeMap(m, parameters...);
  }
  static void mergeMap(std::map<std::string, boost::any> *m,
                       std::map<std::string, boost::any> value) {
    assert(m != nullptr);
    assignMap(m, value);
  }
  static void assignMap(std::map<std::string, boost::any> *m,
                        std::map<std::string, boost::any> value) {
    for (auto it : value) {
      m->insert(pair<string, boost::any>(it.first, it.second));
    }
  }
};

TEST(tests, params) {
  map<string, boost::any> m1 = {{"foo", boost::any(string("bar"))}};
  map<string, boost::any> m2 = {{"a", boost::any(string("b"))}};
  map<string, boost::any> m3 = {{"c", boost::any(string("d"))}};
  map<string, boost::any> m4 = {{"e", boost::any(string("f"))}};
  map<string, boost::any> m = MergeTest::merge(&m1, m2, m3, m4);
  for (auto it : m) {
    printf("%s : %s\n", it.first.c_str(),
           boost::any_cast<string>(it.second).c_str());
  }
}