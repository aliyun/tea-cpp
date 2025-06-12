#include <darabonba/Convert.hpp>

namespace Darabonba {

// Helper function template for conversion
template <typename T>
T Convert::convert(const Json& value) {
    return value.get<T>();
}

// Integer conversion methods
int8_t Convert::int8Val(const Json& value) {
    return value.get<int8_t>();
}

int16_t Convert::int16Val(const Json& value) {
    return value.get<int16_t>();
}

int32_t Convert::int32Val(const Json& value) {
    return value.get<int32_t>();
}

int64_t Convert::int64Val(const Json& value) {
    return value.get<int64_t>();
}

// Unsigned integer conversion methods
uint8_t Convert::uint8Val(const Json& value) {
    return value.get<uint8_t>();
}

uint16_t Convert::uint16Val(const Json& value) {
    return value.get<uint16_t>();
}

uint32_t Convert::uint32Val(const Json& value) {
    return value.get<uint32_t>();
}

uint64_t Convert::uint64Val(const Json& value) {
    return value.get<uint64_t>();
}

// Floating point number conversion methods
float Convert::floatVal(const Json& value) {
    return value.get<float>();
}

double Convert::doubleVal(const Json& value) {
    return value.get<double>();
}

// String conversion method
std::string Convert::stringVal(const Json& value) {
    return value.get<std::string>();
}

// Boolean conversion method
bool Convert::boolVal(const Json& value) {
    return value.get<bool>();
}

// Conversion of any value to a byte array
std::vector<unsigned char> Convert::toBytes(const Json& value) {
    std::string str = value.get<std::string>();
    return std::vector<unsigned char>(str.begin(), str.end());
}

} // namespace Darabonba
