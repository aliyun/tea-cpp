#ifndef DARABONBA_NUMBER_H_
#define DARABONBA_NUMBER_H_

#include <cstdint>
#include <string>

namespace Darabonba {

class Number {
public:
  static int parseInt(const std::string &raw) { return std::stoi(raw); }

  static int64_t parseLong(const std::string &raw) { return std::stoll(raw); }

  static float parseFloat(const std::string &raw) { return std::stof(raw); }

  static double parseDouble(const std::string &raw) { return std::stod(raw); }

  static int64_t itol(int val) { return val; }

  static int ltoi(int64_t val) { return val; }
};
} // namespace Darabonba

#endif
