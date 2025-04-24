#ifndef DARABONBA_SIGNATURE_HMACMD5_H_
#define DARABONBA_SIGNATURE_HMACMD5_H_

#include <darabonba/signature/Hmac.hpp>

namespace Darabonba {
namespace Signature {
class HmacMD5 : public Hmac {
public:
  HmacMD5(const Bytes &key) : Hmac(EVP_md5(), key) {}
  HmacMD5(const void *key, size_t keyLen) : Hmac(EVP_md5(), key, keyLen) {}

  virtual Bytes final() override { return final(16); }

  static Bytes sign(const Bytes &content, const Bytes &key) {
    HmacMD5 hmac(key);
    hmac.update(reinterpret_cast<const void *>(&content[0]), content.size());
    return hmac.final();
  }

  static Bytes sign(const void *content, size_t contentSize, const void *key,
                    size_t keyLen) {
    HmacMD5 hmac(key, keyLen);
    hmac.update(content, contentSize);
    return hmac.final();
  }

protected:
  using Hmac::final;
};

} // namespace Signature
} // namespace Darabonba

#endif