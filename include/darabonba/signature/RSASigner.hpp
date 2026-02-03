#ifndef darabonba_signature_rsa_h_
#define darabonba_signature_rsa_h_

#include <cstdint>
#include <darabonba/Type.hpp>
#include <darabonba/encode/Hash.hpp>
#include <memory>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string>
#include <vector>

namespace Darabonba {
namespace Signature {
class RSASigner {
public:
  RSASigner(const void *privatePemKey, size_t keyLen,
            std::unique_ptr<Encode::Hash> hash)
      : hash_(std::move(hash)) {
    BIO *bioKey = BIO_new_mem_buf(privatePemKey, keyLen);
    if (!bioKey) {
      throw Darabonba::DaraException(
          "Can't create the memory buffer for private key.");
    }
    PEM_read_bio_PrivateKey(bioKey, &pkey_, nullptr, nullptr);
    BIO_free(bioKey); // 释放 BIO 资源
    bioKey = nullptr;

    if (!pkey_) {
      throw Darabonba::DaraException("Can't load the private key");
    }
    ctx_ = EVP_PKEY_CTX_new(pkey_, nullptr);
    if (!ctx_) {
      EVP_PKEY_free(pkey_);
      pkey_ = nullptr;
      throw Darabonba::DaraException("Can't create the context.");
    }
    if (EVP_PKEY_sign_init(ctx_) <= 0) {
      EVP_PKEY_CTX_free(ctx_);
      ctx_ = nullptr;
      EVP_PKEY_free(pkey_);
      pkey_ = nullptr;
      throw Darabonba::DaraException("Can't initialize the context.");
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx_, RSA_PKCS1_PADDING) <= 0) {
      EVP_PKEY_CTX_free(ctx_);
      ctx_ = nullptr;
      EVP_PKEY_free(pkey_);
      pkey_ = nullptr;
      throw Darabonba::DaraException("Can't set padding.");
    }
    auto md = EVP_MD_CTX_get0_md(hash_->ctx_);

    if (EVP_PKEY_CTX_set_signature_md(ctx_, md) <= 0) {
      EVP_PKEY_CTX_free(ctx_);
      ctx_ = nullptr;
      EVP_PKEY_free(pkey_);
      pkey_ = nullptr;
      throw Darabonba::DaraException("Can't set hash method.");
    }
  }

  RSASigner(const std::string &privatePemKey,
            std::unique_ptr<Encode::Hash> hash)
      : RSASigner(reinterpret_cast<const void *>(privatePemKey.c_str()),
                  privatePemKey.size(), std::move(hash)) {}

  RSASigner(const RSASigner &) = delete;
  RSASigner(RSASigner &&) = delete;
  RSASigner &operator=(const RSASigner &) = delete;
  RSASigner &operator=(RSASigner &&) = delete;

  //   RSASigner(const RSASigner &obj)
  //       : pkey_(EVP_PKEY_dup(obj.pkey_)), ctx_(EVP_PKEY_CTX_dup(obj.ctx_)),
  //         hash_(obj.hash_->clone()) {}

  //   RSASigner(RSASigner &&obj)
  //       : pkey_(obj.pkey_), ctx_(obj.ctx_), hash_(std::move(obj.hash_)) {
  //     obj.pkey_ = nullptr;
  //     obj.ctx_ = nullptr;
  //     obj.hash_ = nullptr;
  //   }

  //   RSASigner &operator=(const RSASigner &obj) {
  //     if (this == &obj)
  //       return *this;
  //     pkey_ = EVP_PKEY_dup(obj.pkey_);
  //     ctx_ = EVP_PKEY_CTX_dup(obj.ctx_);
  //     hash_ = std::unique_ptr<Encode::Hash>(obj.hash_->clone());
  //   }

  //   RSASigner &operator=(RSASigner &&obj) {
  //     if (this == &obj)
  //       return *this;
  //     pkey_ = obj.pkey_;
  //     ctx_ = obj.ctx_;
  //     hash_ = std::move(obj.hash_);
  //     obj.pkey_ = nullptr;
  //     obj.ctx_ = nullptr;
  //     obj.hash_ = nullptr;
  //   }

  virtual ~RSASigner() {
    EVP_PKEY_CTX_free(ctx_);
    EVP_PKEY_free(pkey_);
  }

  virtual Bytes final() {
    auto md = hash_->final();
    size_t resLen;
    if (EVP_PKEY_sign(ctx_, nullptr, &resLen,
                      reinterpret_cast<const unsigned char *>(&md[0]),
                      md.size()) <= 0) {
      return {};
    }
    Bytes ret;
    ret.resize(resLen);
    if (EVP_PKEY_sign(ctx_, reinterpret_cast<unsigned char *>(&ret[0]), &resLen,
                      reinterpret_cast<const unsigned char *>(&md[0]),
                      md.size()) <= 0) {
      return {};
    }
    return ret;
  }

  virtual void update(const void *data, size_t bytes) {
    hash_->update(data, bytes);
  }

  static Bytes sign(const void *content, size_t contentSize, const void *key,
                    size_t keyLen, std::unique_ptr<Encode::Hash> hash) {
    RSASigner signer(key, keyLen, std::move(hash));
    signer.update(content, contentSize);
    return signer.final();
  }

protected:
  EVP_PKEY *pkey_ = nullptr;
  EVP_PKEY_CTX *ctx_ = nullptr;
  std::unique_ptr<Encode::Hash> hash_ = nullptr;
};
} // namespace Signature
} // namespace Darabonba

#endif