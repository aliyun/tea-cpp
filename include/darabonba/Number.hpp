#ifndef DARABONBA_NUMBER_H_
#define DARABONBA_NUMBER_H_
namespace Darabonba {

#include <string>

class Number {
public:
  static int parseInt(const std::string &raw) { return std::stoi(raw); }

  static long parseLong(const std::string &raw) { return std::stol(raw); }

  static float parseFloat(const std::string &raw) { return std::stof(raw); }

  static double parseDouble(const std::string &raw) { return std::stod(raw); }

  static long itol(int val) { return val; }

  static int ltoi(long val) { return val; }

  static long add(long val1, long val2) { return val1 + val2; }

  static long sub(long val1, long val2) { return val1 - val2; }

  static long mul(long val1, long val2) { return val1 * val2; }

  static double div(long val1, long val2) {
    return static_cast<double>(val1) / val2;
  }

  static bool gt(long val1, long val2) { return val1 > val2; }

  static bool gte(long val1, long val2) { return val1 >= val2; }

  static bool lt(long val1, long val2) { return val1 < val2; }

  static bool lte(long val1, long val2) { return val1 <= val2; }
};
} // namespace Darabonba

#endif
