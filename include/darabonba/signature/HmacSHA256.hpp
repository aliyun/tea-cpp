#ifndef DARABONBA_SIGNATURE_HMACSHA256_H_
#define DARABONBA_SIGNATURE_HMACSHA256_H_

#include <darabonba/signature/Hmac.hpp>

namespace Darabonba {
namespace Signature {
class HmacSHA256 : public Hmac {
public:
  HmacSHA256(const Bytes &key) : Hmac(EVP_sha256(), key) {}
  HmacSHA256(const void *key, size_t keyLen)
      : Hmac(EVP_sha256(), key, keyLen) {}

  virtual Bytes final() override { return final(32); }

  static Bytes sign(const Bytes &content, const Bytes &key) {
    HmacSHA256 hmac(key);
    hmac.update(reinterpret_cast<const void *>(&content[0]), content.size());
    return hmac.final();
  }

  static Bytes sign(const void *content, size_t contentSize, const void *key,
                    size_t keyLen) {
    HmacSHA256 hmac(key, keyLen);
    hmac.update(content, contentSize);
    return hmac.final();
  }

protected:
  using Hmac::final;
};

} // namespace Signature
} // namespace Darabonba

#endif