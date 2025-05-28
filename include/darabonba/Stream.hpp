#ifndef DARABONBA_STREAM_H_
#define DARABONBA_STREAM_H_

#include <cstdint>
#include <darabonba/Type.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace Darabonba {

class IStream;
class OStream;

class Stream {
public:
  virtual ~Stream() {}

  static Bytes readAsBytes(std::shared_ptr<IStream> raw);

  static std::string readAsString(std::shared_ptr<IStream> raw);

  static Json readAsJSON(std::shared_ptr<IStream> raw);

  static std::shared_ptr<IStream> readFromFilePath(const std::string &path);

  static std::shared_ptr<IStream> readFromBytes(Bytes &raw);

  static std::shared_ptr<IStream> readFromString(const std::string &raw);

  static std::shared_ptr<IStream> toReadable(const std::string &raw);

  static std::shared_ptr<IStream> toReadable(const Bytes &raw);

  static std::shared_ptr<OStream> toWritable(const std::string &raw);

  static std::shared_ptr<OStream> toWritable(const Bytes &raw);

  static void reset(std::shared_ptr<Stream> raw);

};

class IStream : public Stream {
public:
  /**
   * @brief Read the data from stream
   * @param buffer The buffer storing data to be read.
   * @param expectSizeThe size of the data expected to be read.
   * @return The actual size of data read. If return 0, it indicates that the
   * stream has been fully read.
   */
  virtual size_t read(char *buffer, size_t expectSize) = 0;
  virtual ~IStream() {}
};

class ISStream : public IStream, protected std::istringstream {
public:
  ISStream() = default;
  // ISStream(ISStream &&) = default;
  ISStream(std::istringstream &&obj) : std::istringstream(std::move(obj)) {}
  ISStream(const std::string &obj) : std::istringstream(obj) {}
  ISStream(std::string &&obj) : std::istringstream(std::move(obj)) {}
  ISStream(const Bytes &obj)
      : std::istringstream(std::string(obj.begin(), obj.end())) {}
  ISStream(const Darabonba::Json &obj) {
    if (obj.is_string()) {
      std::istringstream::operator=(std::istringstream(obj.get<std::string>()));
    } else if (obj.is_binary()) {
      const auto &bytes = obj.get<Darabonba::Bytes>();
      std::istringstream::operator=(
          std::istringstream(std::string(bytes.begin(), bytes.end())));
    } else {
      std::istringstream::operator=(std::istringstream(obj.dump()));
    }
  }

  virtual ~ISStream() {}

  ISStream &operator=(std::istringstream &&obj) {
    if (this == &obj)
      return *this;
    std::istringstream::operator=(std::move(obj));
    return *this;
  }

  virtual size_t read(char *buffer, size_t expectSize) override;
};

class IFStream : public IStream, protected std::ifstream {
public:
  IFStream() = default;
  IFStream(std::ifstream &&obj) : std::ifstream(std::move(obj)) {}

  virtual ~IFStream() {}

  IFStream &operator=(std::ifstream &&obj) {
    if (this == &obj)
      return *this;
    std::ifstream::operator=(std::move(obj));
    return *this;
  }

  virtual size_t read(char *buffer, size_t expectSize) override;
};

class OStream : public Stream {
public:
  virtual ~OStream(){};
  /**
   * @brief Write the data to stream.
   * @param buffer The buffer storing the data to be written.
   * @param expectSize The expect size of data to be written.
   * @return The actual size of data written. If return 0, it indicates that the
   * stream has been fully written.
   */
  virtual size_t write(char *buffer, size_t expectSize) = 0;
};

class OSStream : public OStream, protected std::ostringstream {
public:
  OSStream() = default;
  OSStream(std::ostringstream &&obj) : std::ostringstream(std::move(obj)) {}
  virtual ~OSStream() {}

  OSStream &operator=(std::ostringstream &&obj) {
    if (this == &obj)
      return *this;
    std::ostringstream::operator=(std::move(obj));
    return *this;
  }

  virtual size_t write(char *buffer, size_t expectSize) override;
};

class OFStream : public OStream, public std::ofstream {
public:
  OFStream() = default;
  OFStream(std::ofstream &&obj) : std::ofstream(std::move(obj)) {}
  virtual ~OFStream() {}

  OFStream &operator=(std::ofstream &&obj) {
    if (this == &obj)
      return *this;
    std::ofstream::operator=(std::move(obj));
    return *this;
  }

  virtual size_t write(char *buffer, size_t expectSize) = 0;
};

class IOStream : public IStream, public OStream {
public:
  virtual ~IOStream() {}
};

} // namespace Darabonba

namespace nlohmann {
  template <>
  struct adl_serializer<std::shared_ptr<Darabonba::IStream>> {
    static void to_json(json &j, const std::shared_ptr<Darabonba::IStream> &body) {
      j = reinterpret_cast<uintptr_t>(body.get());
    }

    static std::shared_ptr<Darabonba::IStream> from_json(const json &j) {
      if (j.is_number_unsigned()) {
        Darabonba::IStream *ptr = reinterpret_cast<Darabonba::IStream *>(j.get<uintptr_t>());
        return std::shared_ptr<Darabonba::IStream>(ptr);
      }
      return nullptr;
    }
  };

  template <>
  struct adl_serializer<std::shared_ptr<Darabonba::OStream>> {
    static void to_json(json &j, const std::shared_ptr<Darabonba::OStream> &body) {
      j = reinterpret_cast<uintptr_t>(body.get());
    }

    static std::shared_ptr<Darabonba::OStream> from_json(const json &j) {
      if (j.is_number_unsigned()) {
        Darabonba::OStream *ptr = reinterpret_cast<Darabonba::OStream *>(j.get<uintptr_t>());
        return std::shared_ptr<Darabonba::OStream>(ptr);
      }
      return nullptr;
    }
  };
}
#endif
