#include <darabonba/Util.hpp>
#include <stdexcept>

namespace Darabonba {

bool Util::assertAsBoolean(const Json &value) {
  if (!value.is_boolean()) {
    throw Exception("value is not a bool");
  }
  return value.get<bool>();
}

std::string Util::assertAsString(const Json &value) {
  if (!value.is_string()) {
    throw Exception("value is not a string");
  }
  return value.get<std::string>();
}

Bytes Util::assertAsBytes(const Json &value) {
  if (!value.is_array()) {
    throw Exception("value is not a bytes");
  }
  try {
    return value.get<Bytes>();
  } catch (std::exception &err) {
    throw Exception("value is not a bytes");
  }
}

int64_t Util::assertAsNumber(const Json &value) {
  if (!value.is_number()) {
    throw Exception("value is not a bytes");
  }
  return value.get<int64_t>();
}

const Json &Util::assertAsMap(const Json &value) {
  if (!value.is_object()) {
    throw Exception("value is not a object");
  }
  return value;
}

IStream *Util::assertAsReadable(Stream *value) {

  auto ret = dynamic_cast<ISStream *>(value);
  if (ret)
    return ret;
  else
    throw Exception("value is not a object");
}

} // namespace Darabonba
