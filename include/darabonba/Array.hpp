#ifndef DARABONBA_ARRAY_H_
#define DARABONBA_ARRAY_H_

#include <algorithm>
#include <cmath>
#include <darabonba/Exception.hpp>
#include <darabonba/Type.hpp>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace Darabonba {
class Array {
public:
  static std::vector<std::string> split(const std::vector<std::string> &raw,
                                        int index, int limit) {
    index = (std::max)(index, 0);
    if (static_cast<size_t>(index) >= raw.size() || limit <= index)
      return {};
    limit = (std::min)((std::max)(limit, 0), static_cast<int>(raw.size()));
    return std::vector<std::string>(raw.begin() + index, raw.begin() + limit);
  }

  static bool contains(const std::vector<std::string> &raw,
                       const std::string &str) {
    return std::find(raw.begin(), raw.end(), str) != raw.end();
  }

  static int index(const std::vector<std::string> &raw,
                   const std::string &str) {
    auto pos = std::find(raw.begin(), raw.end(), str);
    return pos == raw.end() ? -1 : static_cast<int>(pos - raw.begin());
  }

  static int size(const std::vector<std::string> &raw) { return static_cast<int>(raw.size()); }

  static const std::string &get(const std::vector<std::string> &raw,
                                int index) {
    if (index < 0 || static_cast<size_t>(index) >= raw.size())
      throw Darabonba::DaraException("Invalid index.");
    return raw[index];
  }

  template <typename ForwardIter>
  static std::string join(ForwardIter first, ForwardIter last,
                          const std::string &delim) {
    std::string ret;
    if (first == last) {
      return ""; // Return empty string if the range is empty
    }
    while (first != last) {
      ret += (*first++);
      if (first != last) {
        ret += delim;
      }
    }

    return ret;
  }
  static std::string join(const std::vector<std::string> &array,
                          const std::string &delim) {
    return join(array.begin(), array.end(), delim);
  }

  static std::vector<std::string> concat(const std::vector<std::string> &raw,
                                         const std::vector<std::string> &sep) {
    std::vector<std::string> ret = raw;
    std::copy(sep.begin(), sep.end(), std::back_inserter(ret));
    return ret;
  }

  static std::vector<std::string> &ascSort(std::vector<std::string> &raw) {
    std::sort(raw.begin(), raw.end());
    return raw;
  }

  static std::vector<std::string> &descSort(std::vector<std::string> &raw) {
    std::sort(raw.begin(), raw.end(), std::greater<std::string>());
    return raw;
  }

  template <typename T>
  static void append(std::vector<T> &array, const T &item) {
    array.emplace_back(item);
  }

  // Shift: Removes the first element from an array and returns that removed
  // element.
  static std::string shift(std::vector<std::string> &arr) {
    if (arr.empty()) {
      return ""; // Or throw an exception, depending on desired behavior
    }
    std::string first = arr.front();
    arr.erase(arr.begin());
    return first;
  }

  // Pop: Removes the last element from an array and returns that element.
  static std::string pop(std::vector<std::string> &arr) {
    if (arr.empty()) {
      return ""; // Or throw an exception, depending on desired behavior
    }
    std::string last = arr.back();
    arr.pop_back();
    return last;
  }

  // Unshift: Adds one or more elements to the beginning of an array and returns
  // the new length of the array.
  static int32_t unshift(std::vector<std::string> &arr,
                         const std::string &element) {
    arr.insert(arr.begin(), element);
    return static_cast<int32_t>(arr.size());
  }

  // Push: Adds one or more elements to the end of an array and returns the new
  // length of the array.
  static int32_t push(std::vector<std::string> &arr,
                      const std::string &element) {
    arr.push_back(element);
    return static_cast<int32_t>(arr.size());
  }

  // Ascending Sort: Sorts the elements of an array in ascending order
  // (lexicographically).
  static std::vector<std::string> acsSort(std::vector<std::string> arr) {
    std::sort(arr.begin(), arr.end());
    return arr;
  }
};

} // namespace Darabonba

#endif
