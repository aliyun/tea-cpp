#ifndef BYTES_HPP
#define BYTES_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <darabonba/Type.hpp>

namespace Darabonba {

class BytesUtil {
public:
    // Constructor from std::string
    BytesUtil(const std::string &str)
        : data(str.begin(), str.end()) {}

    // Constructor from a vector of unsigned char
    BytesUtil(const std::vector<unsigned char> &vec)
        : data(vec) {}

    // 将字符串转换为字节数据
    static Darabonba::Bytes toBytes(const std::string &str) {
      Bytes binaryData;
      for (char c : str) {
        binaryData.push_back(static_cast<uint8_t>(c));
      }
      return binaryData;
    }

    // 将字节数据转换为十六进制字符串
    std::string toHex() const {
        std::ostringstream oss;
        for (unsigned char byte : data) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return oss.str();
    }

    // 将字节数据转换为Base64字符串
    std::string toBase64() const {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        BIO_write(bio, data.data(), data.size());
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        std::string result(bufferPtr->data, bufferPtr->length - 1);
        BIO_free_all(bio);
        return result;
    }

    // 返回字节数据的大小
    int32_t size() const {
        return static_cast<int32_t>(data.size());
    }

    // 将字节数据转换为JSON表示
    std::string toJSON() const {
        // 具体实现根据实际需求而定，简单版本可以返回hex编码的字符串
        return "\"" + toHex() + "\"";
    }

    // 从给定格式的数据生成Bytes
static Bytes from(const std::string &str, const std::string &format) {
  if (format == "base64") {
    BIO *bio, *b64;
    int strLen = str.length();
    std::vector<unsigned char> buffer((strLen * 3) / 4 + 1);
        
    bio = BIO_new_mem_buf(str.data(), strLen);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);  // Ignore newlines in Base64 encoded data

    int length = BIO_read(bio, buffer.data(), buffer.size());
    BIO_free_all(bio);

    if (length < 0) {
      throw std::runtime_error("Failed to decode base64");
    }

    buffer.resize(length);  // Adjust to actual size
    return Bytes(buffer);
  } else if (format == "utf8" || format == "utf-8") {
    // Convert UTF-8 string to byte vector
    std::vector<unsigned char> buffer(str.begin(), str.end());
    return Bytes(buffer);
  } else if (format == "hex") {
    if (str.length() % 2 != 0) {
      throw std::invalid_argument("Hex string must have an even length");
    }

    std::vector<unsigned char> buffer;
    buffer.reserve(str.length() / 2);

    for (size_t i = 0; i < str.length(); i += 2) {
      std::string byteString = str.substr(i, 2);
      unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
      buffer.push_back(byte);
    }

    return Bytes(buffer);
  } else {
    throw std::invalid_argument("Unsupported format");
  }
}

private:
    std::vector<unsigned char> data;
};

} // namespace Darabonba

#endif // BYTES_HPP
