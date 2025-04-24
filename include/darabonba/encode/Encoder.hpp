#ifndef DARABONBA_ENCODE_ENCODER_H_
#define DARABONBA_ENCODE_ENCODER_H_

#include <algorithm>
#include <cstdint>
#include <darabonba/String.hpp>
#include <darabonba/encode/Base64.hpp>
#include <darabonba/encode/HexEncoder.hpp>
#include <darabonba/encode/SHA256.hpp>
#include <darabonba/encode/SM3.hpp>
#include <darabonba/http/Query.hpp>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Darabonba {
namespace Encode {
class Encoder {
public:
  static std::string urlEncode(const std::string &raw) {
    return Http::Query::encode(raw);
  }

  static std::string percentEncode(const std::string &raw) {
    return Http::Query::encode(raw);
  }

  static std::string pathEncode(const std::string &path) {
    if (path.empty() || path == "/")
      return path;
    std::string ret;
    for (const auto &val : String::split(path, "/")) {
      ret += percentEncode(val) + '/';
    }
    ret.pop_back();
    return ret;
  }

  static std::string hexEncode(const Bytes &bytes) {
    return HexEncoder::encode(bytes.begin(), bytes.end());
  }

  static Bytes hash(const Bytes &bytes, const std::string &signatureAlgorithm) {

    if (signatureAlgorithm.find("HMAC-SHA256") != std::string::npos ||
        signatureAlgorithm.find("RSA-SHA256") != std::string::npos) {
      return SHA256::hash(bytes);
    } else if (signatureAlgorithm.find("HMAC-SM3") != std::string::npos) {
      return SM3::hash(bytes);
    }
    return {};
  }

  static std::string base64EncodeToString(const Bytes &bytes) {
    return Base64::encode(bytes.begin(), bytes.end());
  }

  static Bytes base64Decode(const std::string &src) {
    return Base64::decode(src.begin(), src.end());
  }
};

} // namespace Encode

} // namespace Darabonba

#endif