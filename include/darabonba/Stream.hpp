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

private:
  static std::string cleanString(const std::string& str);
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
  virtual bool isFinished() const = 0;
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

  ISStream(const ISStream& other) {
    std::ostringstream oss;
    oss << other.rdbuf();
    std::istringstream tempStream(oss.str());
    std::istringstream::swap(tempStream);
  }

  virtual size_t read(char *buffer, size_t expectSize) override;

  virtual bool isFinished() const override {
    return eof();
  }
};

class IFStream : public IStream, protected std::ifstream {
public:
  IFStream() : m_openmode(std::ios_base::in) {}

  IFStream(const std::string& filepath, std::ios_base::openmode mode = std::ios_base::in)
      : std::ifstream(filepath, mode), m_filepath(filepath), m_openmode(mode) {
  }

  IFStream(const char* filepath, std::ios_base::openmode mode = std::ios_base::in)
      : std::ifstream(filepath, mode), m_filepath(filepath), m_openmode(mode) {
  }

  // 深拷贝构造函数
  IFStream(const IFStream& other)
      : std::ifstream(), m_filepath(other.m_filepath), m_openmode(other.m_openmode) {

    if (!m_filepath.empty()) {
      this->open(m_filepath, m_openmode);

      if (this->is_open() && other.is_open()) {
        // 使用 const_cast 来调用 tellg()
        auto pos = const_cast<IFStream&>(other).tellg();
        if (pos != std::streampos(-1)) {
          this->seekg(pos);
        }
        // 获取状态也需要 const_cast
        this->clear(const_cast<IFStream&>(other).rdstate());
      }
    }
  }

  // 深拷贝赋值操作符
  IFStream& operator=(const IFStream& other) {
    if (this == &other) {
      return *this;
    }

    if (this->is_open()) {
      this->close();
    }

    m_filepath = other.m_filepath;
    m_openmode = other.m_openmode;

    if (!m_filepath.empty()) {
      this->open(m_filepath, m_openmode);

      if (this->is_open() && other.is_open()) {
        auto pos = const_cast<IFStream&>(other).tellg();
        if (pos != std::streampos(-1)) {
          this->seekg(pos);
        }
        this->clear(const_cast<IFStream&>(other).rdstate());
      }
    }

    return *this;
  }

  virtual ~IFStream() {}

  virtual size_t read(char *buffer, size_t expectSize) override;

  virtual bool isFinished() const override {
    return eof();
  }

private:
  std::string m_filepath;
  std::ios_base::openmode m_openmode;
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
        if (auto* isStreamPtr = dynamic_cast<Darabonba::ISStream*>(ptr)) {
          return std::make_shared<Darabonba::ISStream>(*isStreamPtr);
        } else if (auto* ifStreamPtr = dynamic_cast<Darabonba::IFStream*>(ptr)) {
          return std::make_shared<Darabonba::IFStream>(*ifStreamPtr);
        }
        return std::shared_ptr<Darabonba::IStream>(ptr, [](Darabonba::IStream* ptr) {
          if(ptr && ptr->isFinished()) {
            delete ptr;
          }
        });
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
