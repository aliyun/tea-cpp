#ifndef DARABONBA_ENCODE_HEXENCODER_H_
#define DARABONBA_ENCODE_HEXENCODER_H_

#include <cstdint>
#include <string>
#include <vector>

namespace Darabonba {
namespace Encode {
class HexEncoder {
public:
  template <typename ForwardIter>
  static std::string encode(ForwardIter first, ForwardIter last) {
    std::string ret(std::distance(first, last) * 2, '\0');
    char *p = const_cast<char *>(ret.c_str());
    while (first != last) {
      auto v = *first++;
      *p++ = hexDigits[(v >> 4) & 0xF];
      *p++ = hexDigits[v & 0xF];
    }
    return ret;
  }

protected:
  // Define as header-only constexpr to avoid DLL export/linkage issues on MSVC
  static constexpr const char* hexDigits = "0123456789abcdef";
};

} // namespace Encode

} // namespace Darabonba

#endif