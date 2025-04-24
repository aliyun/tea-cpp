#ifndef DARABONBA_ENCODE_BASE64_H_
#define DARABONBA_ENCODE_BASE64_H_

#include <cstdint>
#include <darabonba/Type.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace Darabonba {
namespace Encode {
class Base64 {
public:
  template <typename ForwardIter>
  static std::string encode(ForwardIter first, ForwardIter last) {
    std::string ret;
    ret.reserve((std::distance(first, last) + 2) / 3 * 4);

    for (uint8_t c1, c2, c3; first != last;) {
      // get the 1st byte
      c1 = *first++;
      if (first == last) {
        // the higher 6-bits
        ret.push_back(base64Chars[c1 >> 2]);
        // the lower 2-bits
        ret.push_back(base64Chars[(c1 & 0x3) << 4]);
        // padding
        ret.append("==");
        break;
      }
      // get 2nd byte
      c2 = *first++;
      if (first == last) {
        // the higher 6-bits of the 1st byte
        ret.push_back(base64Chars[c1 >> 2]);
        // the lower 2-bits of the 1st byte, the higher 4-bits of the 2nd byte
        ret.push_back(base64Chars[((c1 & 0x3) << 4) | ((c2 & 0xf0) >> 4)]);
        // the lower 4-bits of the 2nd byte
        ret.push_back(base64Chars[(c2 & 0xf) << 2]);
        // padding
        ret.push_back('=');
        break;
      }
      // get the 3rd byte
      c3 = *first++;
      // the higher 6-bits of the 1st byte
      ret.push_back(base64Chars[c1 >> 2]);
      // the lower 2-bits of the 1st byte, the higher 4-bits of the 2nd byte
      ret.push_back(base64Chars[((c1 & 0x3) << 4) | ((c2 & 0xf0) >> 4)]);
      // the higher 4-bits of the 2nd byte, the lower 2-bits of the 3rd byte
      ret.push_back(base64Chars[((c2 & 0xf) << 2) | ((c3 & 0xc0) >> 6)]);
      // the lower 6-bits of the 3rd byte
      ret.push_back(base64Chars[c3 & 0x3f]);
    }
    return ret;
  }

  template <typename ForwardIter>
  static Bytes decode(ForwardIter first, ForwardIter last) {
    auto length = std::distance(first, last);
    if (length % 4 != 0) {
      throw Darabonba::Exception("Invalid base64 encoded data.");
    }
    Bytes ret;
    ret.reserve(length / 4 * 3);

    decltype(ret.size()) countOfPadding = 0;
    // i is the index of the encoded data, j is the index of the decoded data
    for (decltype(ret.size()) i = 0; first != last; ++i) {
      char c = static_cast<char>(*first++);
      int v = posOfBase64Char(c);
      if (v != -1) {
        switch (i % 4) {
        case 0:
          ret.push_back((v << 2) & 0xfc); // all 6-bits of the 1st char
          break;
        case 1:
          ret.back() |= (v >> 4) & 0x03;  // the higher 2-bits of the 2nd char
          ret.push_back((v << 4) & 0xf0); // the lower 4-bits of the 2nd char
          break;
        case 2:
          ret.back() |= (v >> 2) & 0x0f;  // the higher 4-bits of the 3rd char
          ret.push_back((v << 6) & 0xc0); // the lower 2-bits of the 3rd;
          break;
        case 3:
          ret.back() |= v & 0x3f; // all the 6-bits of the 4th char
          break;
        }
      } else if (c == '=') {
        countOfPadding = 1;
        ret.pop_back(); // remove the filling char
        break;
      } else {
        throw Darabonba::Exception("Invalid base64 encoded data.");
      }
    }
    for (; first != last;) {
      if (*first++ == '=')
        ++countOfPadding;
      else
        throw Darabonba::Exception("Invalid base64 encoded data.");
    }
    if (countOfPadding > 3) {
      throw Darabonba::Exception("Invalid base64 encoded data.");
    }
    return ret;
  }

protected:
  static const char base64Chars[64 + 1];

  static inline int posOfBase64Char(const unsigned char c) {
    if ('A' <= c && c <= 'Z')
      return c - 'A';
    else if ('a' <= c && c <= 'z')
      return ('Z' - 'A') + (c - 'a') + 1;
    else if ('0' <= c && c <= '9')
      return ('Z' - 'A') + ('z' - 'a') + c - '0' + 2;
    else if (c == '+')
      return 62;
    else if (c == '/')
      return 63;
    else
      return -1;
  }
};
} // namespace Encode

} // namespace Darabonba

#endif