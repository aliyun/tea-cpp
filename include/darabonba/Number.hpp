#ifndef DARABONBA_NUMBER_H_
#define DARABONBA_NUMBER_H_

#include <string>

namespace Darabonba {

class Number {
public:
  static int parseInt(const std::string &raw) { return std::stoi(raw); }

  static long parseLong(const std::string &raw) { return std::stol(raw); }

  static float parseFloat(const std::string &raw) { return std::stof(raw); }

  static double parseDouble(const std::string &raw) { return std::stod(raw); }

  static long itol(int val) { return val; }

  static int ltoi(long val) { return val; }
};
} // namespace Darabonba

#endif
