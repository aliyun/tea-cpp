#ifndef DARABONBA_CONVERT_HPP
#define DARABONBA_CONVERT_HPP

#include <cstdint>
#include <darabonba/Type.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace Darabonba {

class Convert {
public:
  // Integer conversion
  static int8_t int8Val(const Json &value);
  static int16_t int16Val(const Json &value);
  static int32_t int32Val(const Json &value);
  static int64_t int64Val(const Json &value);

  // Unsigned integer conversion
  static uint8_t uint8Val(const Json &value);
  static uint16_t uint16Val(const Json &value);
  static uint32_t uint32Val(const Json &value);
  static uint64_t uint64Val(const Json &value);

  // Number conversion
  static float floatVal(const Json &value);
  static double doubleVal(const Json &value);

  // String conversion
  static std::string stringVal(const Json &value);

  // Boolean conversion
  static bool boolVal(const Json &value);

  // Bytes conversion
  static std::vector<unsigned char> toBytes(const Json &value);

private:
  template <typename T> static T convert(const Json &value);
};

} // namespace Darabonba

#endif // DARABONBA_CONVERT_HPP
