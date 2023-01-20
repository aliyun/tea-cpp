#include <darabonba/core.hpp>
#include <regex>

using namespace Darabonba;
using namespace std;

void Model::validateRequired(const string &fieldName,
                             const json &map) {
  if (map.find(fieldName) == map.end()) {
    throw ValidateError::New("RequiredValidateError", fieldName + " is required.");
  }
}

void Model::validateMaxLength(const string &fieldName,
                              const json &map, int val) {
  if (map.find(fieldName) == map.end()) {
    return;
  }
  int field_len = map[fieldName].get<string>().length();
  if (field_len > val) {
    throw ValidateError::New("MaxLengthValidateError",  fieldName + " is exceed max-length: " + to_string(val));
  }
}
void Model::validateMinLength(const string &fieldName,
                              const json &map, int val) {
  if (map.find(fieldName) == map.end()) {
    return;
  }
  int field_len = map[fieldName].get<string>().length();
  if (field_len < val) {
    throw ValidateError::New("MinLengthValidateError",  fieldName + " is exceed min-length: " + to_string(val));
  }
}

template <typename T> void Model::validateMaximum(const string &fieldName,
                            const json &map, T val) {
  if (map.find(fieldName) == map.end()) {
    return;
  }
  T field = map[fieldName].get<T>();
  if (field > val) {
    throw ValidateError::New("MaximumValidateError",
        fieldName + " cannot be greater than " + to_string(val));
  }
}

template <typename T> void Model::validateMinimum(const string &fieldName,
                            const json &map, T val) {
  if (map.find(fieldName) == map.end()) {
    return;
  }
  T field = map[fieldName].get<T>();
  if (field < val) {
    throw ValidateError::New("MinimumValidateError",
                             fieldName + " cannot be less than " + to_string(val));
  }
}

void Model::validatePattern(const string &fieldName,
                            const json &map,
                            const string &val) {
  if (map.find(fieldName) == map.end()) {
    return;
  }
  string field = map[fieldName].get<string>();
  regex pattern(val, regex::icase);
  if (!regex_search(field, pattern)) {
    throw ValidateError::New("PatternValidateError",  fieldName + " is not match " + val);
  }
}
