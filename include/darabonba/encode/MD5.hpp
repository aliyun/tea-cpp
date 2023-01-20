#ifndef DARABONBA_ENCODE_MD5_H_
#define DARABONBA_ENCODE_MD5_H_

#include <darabonba/encode/Hash.hpp>

namespace Darabonba {
namespace Encode {
class MD5 : Hash {
public:
  MD5() : Hash(EVP_md5()) {}

  virtual ~MD5() {}

  virtual Bytes final() override { return final(16); }

  virtual MD5 *clone() override { return new MD5(*this); }

  static Bytes hash(const Bytes &content) {
    MD5 hash;
    hash.update(reinterpret_cast<const void *>(&content[0]), content.size());
    return hash.final();
  }

  static Bytes hash(const void *content, size_t contentSize) {
    MD5 hash;
    hash.update(content, contentSize);
    return hash.final();
  }

protected:
  using Hash::final;
};
} // namespace Encode
} // namespace Darabonba

#endif