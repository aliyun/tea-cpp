#include <darabonba/Convert.hpp>

namespace Darabonba {

// Helper function template for conversion
template <typename T>
T Convert::convert(const SimpleAny& value) {
    return value.cast<T>();
}

// Integer conversion methods
int8_t Convert::int8Val(const SimpleAny& value) {
    return convert<int8_t>(value);
}

int16_t Convert::int16Val(const SimpleAny& value) {
    return convert<int16_t>(value);
}

int32_t Convert::int32Val(const SimpleAny& value) {
    return convert<int32_t>(value);
}

int64_t Convert::int64Val(const SimpleAny& value) {
    return convert<int64_t>(value);
}

// Unsigned integer conversion methods
uint8_t Convert::uint8Val(const SimpleAny& value) {
    return convert<uint8_t>(value);
}

uint16_t Convert::uint16Val(const SimpleAny& value) {
    return convert<uint16_t>(value);
}

uint32_t Convert::uint32Val(const SimpleAny& value) {
    return convert<uint32_t>(value);
}

uint64_t Convert::uint64Val(const SimpleAny& value) {
    return convert<uint64_t>(value);
}

// Floating point number conversion methods
float Convert::floatVal(const SimpleAny& value) {
    return convert<float>(value);
}

double Convert::doubleVal(const SimpleAny& value) {
    return convert<double>(value);
}

// String conversion method
std::string Convert::stringVal(const SimpleAny& value) {
    return convert<std::string>(value);
}

// Boolean conversion method
bool Convert::boolVal(const SimpleAny& value) {
    return convert<bool>(value);
}

// Conversion of any value to a byte array
std::vector<unsigned char> Convert::toBytes(const SimpleAny& value) {
    std::string str = convert<std::string>(value);
    return std::vector<unsigned char>(str.begin(), str.end());
}

} // namespace Darabonba
