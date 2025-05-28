#ifndef DARABONBA_TYPE_H_
#define DARABONBA_TYPE_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace Darabonba {

using Json = nlohmann::json;
using Bytes = nlohmann::json::binary_t;
// using Bytes = std::vector<uint8_t>;

} // namespace Darabonba

#endif