#ifndef DARABONBA_ENCODE_SHA256_H_
#define DARABONBA_ENCODE_SHA256_H_

#include <darabonba/encode/Hash.hpp>

namespace Darabonba {
namespace Encode {
class SHA256 : public Hash {
public:
  SHA256() : Hash(EVP_sha256()) {}

  SHA256(const SHA256 &) = default;
  SHA256(SHA256 &&) = default;
  SHA256 &operator=(const SHA256 &) = default;
  SHA256 &operator=(SHA256 &&) = default;
  virtual ~SHA256() {}

  virtual Bytes final() override { return final(32); }

  virtual SHA256 *clone() override { return new SHA256(*this); }

  static Bytes hash(const Bytes &content) {
    SHA256 hash;
    hash.update(reinterpret_cast<const void *>(&content[0]), content.size());
    return hash.final();
  }
  static Bytes hash(const void *content, size_t contentSize) {
    SHA256 hash;
    hash.update(content, contentSize);
    return hash.final();
  }

protected:
  using Hash::final;
};
} // namespace Encode
} // namespace Darabonba

#endif