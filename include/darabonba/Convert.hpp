#ifndef DARABONBA_CONVERT_HPP
#define DARABONBA_CONVERT_HPP

#include <string>
#include <sstream>
#include <vector>
#include <type_traits>
#include <cstdint>
#include "Any.hpp"

namespace Darabonba {

class Convert {
public:
    // Integer conversion
    static int8_t int8Val(const SimpleAny& value);
    static int16_t int16Val(const SimpleAny& value);
    static int32_t int32Val(const SimpleAny& value);
    static int64_t int64Val(const SimpleAny& value);

    // Unsigned integer conversion
    static uint8_t uint8Val(const SimpleAny& value);
    static uint16_t uint16Val(const SimpleAny& value);
    static uint32_t uint32Val(const SimpleAny& value);
    static uint64_t uint64Val(const SimpleAny& value);

    // Number conversion
    static float floatVal(const SimpleAny& value);
    static double doubleVal(const SimpleAny& value);

    // String conversion
    static std::string stringVal(const SimpleAny& value);

    // Boolean conversion
    static bool boolVal(const SimpleAny& value);

    // Bytes conversion
    static std::vector<unsigned char> toBytes(const SimpleAny& value);

private:
    template <typename T>
    static T convert(const SimpleAny& value);
};

} // namespace Darabonba

#endif // DARABONBA_CONVERT_HPP
