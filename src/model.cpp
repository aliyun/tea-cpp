#include <boost/throw_exception.hpp>
#include <darabonba/core.hpp>
#include <regex>

using namespace Darabonba;
using namespace std;


template <typename T> bool can_cast(shared_ptr<boost::any>& a) { return typeid(T) == a->type(); }

void Model::validateRequired(const string &field_name,
                             const shared_ptr<boost::any>& field) {
  if (!field) {
    BOOST_THROW_EXCEPTION(boost::enable_error_info(
        std::runtime_error(field_name + " is required.")));
  }
}

void Model::validateMaxLength(const string &field_name,
                              const shared_ptr<string>& field,
                              int val) {
  if (!field) {
    return;
  }
  int field_len = field->length();
  if (field_len > val) {
    BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
        field_name + " is exceed max-length: " + to_string(val))));
  }
}
void Model::validateMinLength(const string &field_name,
                              const shared_ptr<string>& field,
                              int val) {
  if (!field) {
    return;
  }
  int field_len = field->length();
  if (field_len < val) {
    BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
        field_name + " is exceed min-length: " + to_string(val))));
  }
}

void Model::validateMaximum(const string &field_name,
                            shared_ptr<boost::any> field,
                            boost::any val) {
  if (!field) {
    return;
  }
  if (can_cast<int>(field)) {
    int src = boost::any_cast<int>(*field);
    int max = boost::any_cast<int>(val);
    if (src > max) {
      BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
          field_name + " cannot be greater than " + to_string(max))));
    }
  } else if (can_cast<long>(field)) {
    long src = boost::any_cast<long>(*field);
    long max = boost::any_cast<long>(val);
    if (src > max) {
      BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
          field_name + " cannot be greater than " + to_string(max))));
    }
  }
}
void Model::validateMinimum(const string &field_name,
                            shared_ptr<boost::any> field,
                            boost::any val) {
  if (!field) {
    return;
  }
  if (can_cast<int>(field)) {
    int src = boost::any_cast<int>(*field);
    int max = boost::any_cast<int>(val);
    if (src < max) {
      BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
          field_name + " cannot be less than " + to_string(max))));
    }
  } else if (can_cast<long>(field)) {
    long src = boost::any_cast<long>(*field);
    long max = boost::any_cast<long>(val);
    if (src < max) {
      BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
          field_name + " cannot be less than " + to_string(max))));
    }
  }
}

void Model::validatePattern(const string &field_name,
                            const shared_ptr<string>& field,
                            const string &val) {
  if (!field) {
    return;
  }
  if (field->empty()) {
    return;
  }
  regex pattern(val, regex::icase);
  smatch result;
  if (!regex_match(*field, result, pattern)) {
    BOOST_THROW_EXCEPTION(boost::enable_error_info(
        std::runtime_error(field_name + " is not match " + val)));
  }
}


