#ifndef DARABONBA_ENCODE_SHA1_H_
#define DARABONBA_ENCODE_SHA1_H_
#include <darabonba/encode/Hash.hpp>

namespace Darabonba {
namespace Encode {
class SHA1 : public Hash {
public:
  SHA1() : Hash(EVP_sha1()) {}

  SHA1(const SHA1 &) = default;
  SHA1(SHA1 &&) = default;
  SHA1 &operator=(const SHA1 &) = default;
  SHA1 &operator=(SHA1 &&) = default;
  virtual ~SHA1() {}

  virtual Bytes final() override { return final(20); }

  virtual SHA1 *clone() override { return new SHA1(*this); }

  static Bytes hash(const Bytes &content) {
    SHA1 hash;
    hash.update(reinterpret_cast<const void *>(&content[0]), content.size());
    return hash.final();
  }
  static Bytes hash(const void *content, size_t contentSize) {
    SHA1 hash;
    hash.update(content, contentSize);
    return hash.final();
  }

protected:
  using Hash::final;
};
} // namespace Encode
} // namespace Darabonba

#endif