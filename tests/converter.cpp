#include "gtest/gtest.h"
#include <darabonba/core.hpp>
#include <map>

using namespace std;

TEST(tests_converter, merge_string_string) {
  map<string, string> m1 = {{"foo", string("bar")}};
  map<string, string> m2 = {{"a", string("b")}};
  map<string, string> m3 = {{"c", string("d")}};
  map<string, string> m4 = {{"e", string("f")}};
  map<string, string> res = Darabonba::Converter::merge(m1, m2, m3, m4);

  map<string, string> expect = {{"foo", string("bar")},
                                {"a", string("b")},
                                {"c", string("d")},
                                {"e", string("f")}};
  ASSERT_EQ(expect["foo"], res["foo"]);
  ASSERT_EQ(expect["a"], res["a"]);
  ASSERT_EQ(expect["c"], res["c"]);
  ASSERT_EQ(expect["e"], res["e"]);
}

TEST(tests_converter, merge_string_any) {
  map<string, boost::any> m1 = {{"foo", boost::any(string("bar"))}};
  map<string, boost::any> m2 = {{"a", boost::any(string("b"))}};
  map<string, boost::any> m3 = {{"c", boost::any(string("d"))}};
  map<string, boost::any> m4 = {{"e", boost::any(string("f"))}};
  map<string, boost::any> res = Darabonba::Converter::merge(m1, m2, m3, m4);

  map<string, boost::any> expect = {{"foo", boost::any(string("bar"))},
                                    {"a", boost::any(string("b"))},
                                    {"c", boost::any(string("d"))},
                                    {"e", boost::any(string("f"))}};
  ASSERT_EQ(boost::any_cast<string>(expect["foo"]),
            boost::any_cast<string>(res["foo"]));
  ASSERT_EQ(boost::any_cast<string>(expect["a"]),
            boost::any_cast<string>(res["a"]));
  ASSERT_EQ(boost::any_cast<string>(expect["c"]),
            boost::any_cast<string>(res["c"]));
  ASSERT_EQ(boost::any_cast<string>(expect["e"]),
            boost::any_cast<string>(res["e"]));
}