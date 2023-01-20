#ifndef DARABONBA_SIGNATURE_HMAC_H_
#define DARABONBA_SIGNATURE_HMAC_H_
#include <cstdint>
#include <openssl/evp.h>

#include <vector>
namespace Darabonba {
namespace Signature {
class Hmac {
public:
  Hmac(const EVP_MD *type, const Bytes &key)
      : Hmac(type, reinterpret_cast<const void *>(&key[0]), key.size()) {}
  Hmac(const EVP_MD *type, const void *key, size_t keyLen)
      : pkey_(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr,
                                   reinterpret_cast<const unsigned char *>(key),
                                   static_cast<unsigned int>(keyLen))),
        ctx_(EVP_MD_CTX_new()) {
    EVP_DigestSignInit(ctx_, nullptr, type, nullptr, pkey_);
  }

  // TODO: copy ctor and so on.
  Hmac(const Hmac &) = delete;
  Hmac(Hmac &&) = delete;
  Hmac &operator=(const Hmac &) = delete;
  Hmac &operator=(Hmac &&) = delete;

  virtual Bytes final() = 0;

  virtual void update(const void *data, size_t bytes) {
    EVP_DigestSignUpdate(ctx_, data, bytes);
  }

  virtual ~Hmac() {
    EVP_PKEY_free(pkey_);
    EVP_MD_CTX_free(ctx_);
  }

protected:
  Bytes final(size_t len) {
    Bytes hash;
    hash.resize(len);
    EVP_DigestSignFinal(ctx_, reinterpret_cast<unsigned char *>(&hash[0]),
                        &len);
    return hash;
  }

  EVP_PKEY *pkey_ = nullptr;
  EVP_MD_CTX *ctx_ = nullptr;
};
} // namespace Signature
} // namespace Darabonba

#endif