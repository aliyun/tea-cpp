#include "gtest/gtest.h"
#include <boost/exception/current_exception_cast.hpp>
#include <darabonba/core.hpp>
#include <memory>
#include <regex>

#include "mock/model.h"

using namespace Darabonba;
using namespace std;

TEST(tests_model, test_construct) {
  map<string, boost::any> config;
  config.insert(pair<string, boost::any>(string("stringfield"),
                                         boost::any(string("test"))));
  MockModel model(config);
  ASSERT_EQ(string("test"), *model.stringfield);
}

TEST(tests_model, test_validateRequired) {
  string field_name = "foo";
  boost::any field_value;
  try {
    string f;
    field_value = f;
    Model::validateRequired(field_name, &field_value, true);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo is required.", err);
  }

  try {
    field_value = string("");
    Model::validateRequired(field_name, &field_value, true);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo is required.", err);
  }
}

TEST(tests_model, test_validateMaxLength) {
  string field_name = "foo";
  auto *field_value = new string("test");
  try {
    Model::validateMaxLength(field_name, field_value, 3);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo is exceed max-length: 3", err);
  }
}

TEST(tests_model, test_validateMinLength) {
  string field_name = "foo";
  auto *field_value = new string("test");
  try {
    Model::validateMinLength(field_name, field_value, 5);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo is exceed min-length: 5", err);
  }
}

TEST(tests_model, test_validateMaximum) {
  string field_name = "foo";
  boost::any field_value;
  field_value = 101;
  try {
    Model::validateMaximum(field_name, &field_value, 100);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo cannot be greater than 100", err);
  }

  field_value = LONG_MAX;
  try {
    Model::validateMaximum(field_name, &field_value, LONG_MAX - 1);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo cannot be greater than " + to_string(LONG_MAX - 1), err);
  }
}

TEST(tests_model, test_validateMinimum) {
  string field_name = "foo";
  boost::any field_value;
  field_value = 99;
  try {
    Model::validateMinimum(field_name, &field_value, 100);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo cannot be less than 100", err);
  }

  field_value = LONG_MAX - 1;
  try {
    Model::validateMinimum(field_name, &field_value, LONG_MAX);
    ASSERT_TRUE(false);
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo cannot be less than " + to_string(LONG_MAX), err);
  }
}

TEST(tests_model, test_validatePattern) {
  string field_name = "foo";
  string field_value =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  Model::validatePattern(field_name, &field_value,
                         "[a-z0-9A-Z]+"); // No exception is OK
  try {
    field_value = "@test#";
    Model::validatePattern(field_name, &field_value, "[a-z0-9A-Z]+");
    ASSERT_TRUE(false); // should not be here
  } catch (boost::exception &e) {
    string err = boost::current_exception_cast<exception>()->what();
    ASSERT_EQ("foo is not match [a-z0-9A-Z]+", err);
  }
}