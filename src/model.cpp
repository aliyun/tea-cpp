#include <boost/throw_exception.hpp>
#include <darabonba/core.hpp>
#include <regex>

using namespace Darabonba;
using namespace std;
using namespace boost;

template <typename T> bool can_cast(any *a) { return typeid(T) == a->type(); }

void Model::validateRequired(const string &field_name, any *field, bool val) {
  if (val) {
    if (can_cast<int>(field)) {
      return;
    } else if (can_cast<string>(field)) {
      auto *f = any_cast<string>(field);
      if (f->empty()) {
        BOOST_THROW_EXCEPTION(enable_error_info(
            std::runtime_error(field_name + " is required.")));
      }
    }
  }
}

void Model::validateMaxLength(const string &field_name, string *field,
                              int val) {
  if (field == nullptr) {
    return;
  }
  int field_len = field->length();
  if (field_len > val) {
    BOOST_THROW_EXCEPTION(enable_error_info(std::runtime_error(
        field_name + " is exceed max-length: " + to_string(val))));
  }
}
void Model::validateMinLength(const string &field_name, string *field,
                              int val) {
  if (field == nullptr) {
    return;
  }
  int field_len = field->length();
  if (field_len < val) {
    BOOST_THROW_EXCEPTION(enable_error_info(std::runtime_error(
        field_name + " is exceed min-length: " + to_string(val))));
  }
}

void Model::validateMaximum(const string &field_name, any *field, any val) {
  if (can_cast<int>(field)) {
    int *src = any_cast<int>(field);
    int max = any_cast<int>(val);
    if (*src > max) {
      BOOST_THROW_EXCEPTION(enable_error_info(std::runtime_error(
          field_name + " cannot be greater than " + to_string(max))));
    }
  } else if (can_cast<long>(field)) {
    long *src = any_cast<long>(field);
    long max = any_cast<long>(val);
    if (*src > max) {
      BOOST_THROW_EXCEPTION(enable_error_info(std::runtime_error(
          field_name + " cannot be greater than " + to_string(max))));
    }
  }
}
void Model::validateMinimum(const string &field_name, any *field, any val) {
  if (can_cast<int>(field)) {
    int *src = any_cast<int>(field);
    int max = any_cast<int>(val);
    if (*src < max) {
      BOOST_THROW_EXCEPTION(enable_error_info(std::runtime_error(
          field_name + " cannot be less than " + to_string(max))));
    }
  } else if (can_cast<long>(field)) {
    long *src = any_cast<long>(field);
    long max = any_cast<long>(val);
    if (*src < max) {
      BOOST_THROW_EXCEPTION(enable_error_info(std::runtime_error(
          field_name + " cannot be less than " + to_string(max))));
    }
  }
}

void Model::validatePattern(const string &field_name, string *field,
                            const string &val) {
  if (nullptr == field) {
    return;
  }
  if (field->empty()) {
    return;
  }
  regex pattern(val, regex::icase);
  smatch result;
  if (!regex_match(*field, result, pattern)) {
    BOOST_THROW_EXCEPTION(enable_error_info(
        std::runtime_error(field_name + " is not match " + val)));
  }
}

Model::Model(const std::map<string, boost::any> &config) { _config = config; }

Model::~Model() = default;
Model::Model() = default;
