#ifndef DARABONBA_SIGNATURE_SIGNER_H_
#define DARABONBA_SIGNATURE_SIGNER_H_

#include <darabonba/encode/MD5.hpp>
#include <darabonba/signature/HmacMD5.hpp>
#include <darabonba/signature/HmacSHA1.hpp>
#include <darabonba/signature/HmacSHA256.hpp>
#include <darabonba/signature/HmacSM3.hpp>
#include <darabonba/signature/RSASigner.hpp>
#include <darabonba/encode/SHA256.hpp>
#include <darabonba/String.hpp>

namespace Darabonba {

namespace Signature {
class Signer {
public:
  static Bytes HmacSHA1Sign(const std::string &stringToSign,
                            const std::string &secret) {
    return HmacSHA1::sign(reinterpret_cast<const void *>(stringToSign.c_str()),
                          stringToSign.size(),
                          reinterpret_cast<const void *>(secret.c_str()),
                          secret.size());
  }

  static Bytes HmacSHA1SignByBytes(const std::string &stringToSign,
                                   const Bytes &secret) {
    return HmacSHA1::sign(reinterpret_cast<const void *>(stringToSign.c_str()),
                          stringToSign.size(),
                          reinterpret_cast<const void *>(&secret[0]),
                          secret.size());
  }

  static Bytes HmacSHA256Sign(const std::string &stringToSign,
                              const std::string &secret) {
    return HmacSHA256::sign(
        reinterpret_cast<const void *>(stringToSign.c_str()),
        stringToSign.size(), reinterpret_cast<const void *>(secret.c_str()),
        secret.size());
  }

  static Bytes HmacSHA256SignByBytes(const std::string &stringToSign,
                                     const Bytes &secret) {
    return HmacSHA256::sign(
        reinterpret_cast<const void *>(stringToSign.c_str()),
        stringToSign.size(), reinterpret_cast<const void *>(&secret[0]),
        secret.size());
  }

  static Bytes HmacSM3Sign(const std::string &stringToSign,
                           const std::string &secret) {
    return HmacSM3::sign(reinterpret_cast<const void *>(stringToSign.c_str()),
                         stringToSign.size(),
                         reinterpret_cast<const void *>(secret.c_str()),
                         secret.size());
  }

  static Bytes HmacSM3SignByBytes(const std::string &stringToSign,
                                  const Bytes &secret) {
    return HmacSM3::sign(reinterpret_cast<const void *>(stringToSign.c_str()),
                         stringToSign.size(),
                         reinterpret_cast<const void *>(&secret[0]),
                         secret.size());
  }

  static Bytes SHA256withRSASign(const std::string &stringToSign,
                                 const std::string &secret) {
    static const std::string PEM_BEGIN = "-----BEGIN RSA PRIVATE KEY-----\n";
    static const std::string PEM_END = "\n-----END RSA PRIVATE KEY-----";

    auto pemSecret = secret;
    if (!String::hasPrefix(pemSecret, PEM_BEGIN)) {
      pemSecret = PEM_BEGIN + pemSecret;
    }
    if (!String::hasSuffix(secret, PEM_END)) {
      pemSecret += PEM_END; // sk.substr(sk.size() - PEM_END.size());
    }
    return RSASigner::sign(
        reinterpret_cast<const void *>(stringToSign.c_str()),
        stringToSign.size(), reinterpret_cast<const void *>(&pemSecret[0]),
        pemSecret.size(), std::unique_ptr<Encode::Hash>(new Encode::SHA256()));
  }

  static Bytes MD5Sign(const std::string &stringToSign) {
    return Encode::MD5::hash(
        reinterpret_cast<const void *>(stringToSign.c_str()),
        stringToSign.size());
  }

  static Bytes MD5SignForBytes(const Bytes &bytesToSign) {
    return Encode::MD5::hash(reinterpret_cast<const void *>(&bytesToSign[0]),
                             bytesToSign.size());
  }
};
} // namespace Signature
} // namespace Darabonba
#endif