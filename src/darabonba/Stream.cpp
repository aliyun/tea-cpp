#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <darabonba/Exception.hpp>
#include <darabonba/Stream.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace Darabonba {
namespace {

struct KeptStream {
  std::atomic<int> refs;
  std::mutex mutex;
  std::shared_ptr<IStream> stream;
};

} // namespace

extern "C" void darabonba_stream_keep_addref(void *keep) {
  if (keep != nullptr) {
    static_cast<KeptStream *>(keep)->refs.fetch_add(1, std::memory_order_relaxed);
  }
}

extern "C" void darabonba_stream_keep_release(void *keep) {
  if (keep == nullptr) {
    return;
  }
  auto *kept = static_cast<KeptStream *>(keep);
  if (kept->refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    delete kept;
  }
}

Json Stream::retain(std::shared_ptr<IStream> stream) {
  if (!stream) {
    return Json(nullptr);
  }
  std::unique_ptr<KeptStream> kept(new KeptStream);
  kept->refs.store(1, std::memory_order_relaxed);
  kept->stream = std::move(stream);

  unsigned char raw[sizeof(std::uint64_t) + sizeof(void *)];
  const std::uint64_t magic = StreamKeep::kMagic;
  void *pointer = kept.get();
  std::memcpy(raw, &magic, sizeof(magic));
  std::memcpy(raw + sizeof(magic), &pointer, sizeof(pointer));
  std::vector<std::uint8_t> bytes(raw, raw + sizeof(raw));
  Json value = Json::binary(std::move(bytes), StreamKeep::kSubtype);
  kept.release();
  return value;
}

std::shared_ptr<IStream> Stream::claimRetained(void *keep) {
  if (keep == nullptr) {
    return nullptr;
  }
  auto *kept = static_cast<KeptStream *>(keep);
  std::lock_guard<std::mutex> lock(kept->mutex);
  return std::move(kept->stream);
}

Bytes Stream::readAsBytes(std::shared_ptr<IStream> raw) {
  if (!raw)
    return Bytes();

  Bytes result;
  char buffer[1024];
  size_t bytesRead;
  while ((bytesRead = raw->read(buffer, sizeof(buffer))) > 0) {
    result.insert(result.end(), buffer, buffer + bytesRead);
  }
  return result;
}

std::string Stream::readAsString(std::shared_ptr<IStream> raw) {
  if (!raw)
    return "";

  std::ostringstream oss;
  char buffer[1024];
  size_t bytesRead;
  while ((bytesRead = raw->read(buffer, sizeof(buffer))) > 0) {
    oss.write(buffer, bytesRead);
  }
  return oss.str();
}

Json Stream::readAsJSON(std::shared_ptr<IStream> raw) {
  std::string str = readAsString(raw);
  if (str.empty()) {
    return Json();
  }
  try {
    return Json::parse(str);
  } catch (const nlohmann::json::parse_error &e) {
    throw Darabonba::DaraException(std::string("JSON parse error: ") + e.what());
  }
}

std::shared_ptr<IStream> Stream::toReadable(const std::string &raw) {
  return std::make_shared<ISStream>(raw);
}

std::shared_ptr<IStream> Stream::toReadable(const Bytes &raw) {
  return std::make_shared<ISStream>(raw);
}

std::shared_ptr<IStream> Stream::toReadable(const Json &raw) {                                                                                                            
  return std::make_shared<ISStream>(raw);                                                                                                                                 
}

std::shared_ptr<OStream> Stream::toWritable(const std::string &raw) {
  return std::make_shared<OSStream>(std::ostringstream(raw));
}
std::shared_ptr<OStream> Stream::toWritable(const Bytes &raw) {
  return std::make_shared<OSStream>(
      std::ostringstream(std::string(raw.begin(), raw.end())));
}

std::shared_ptr<IStream> Stream::readFromFilePath(const std::string &path) {
  auto* stream = new IFStream(path, std::ios::binary);
  if (!stream->isOpen()) {
    delete stream;
    throw Darabonba::DaraException("File not found or cannot be opened: " + path);
  }
  return std::shared_ptr<IStream>(stream);
}

std::shared_ptr<IStream> Stream::readFromBytes(Bytes &raw) {
  std::string s(raw.begin(), raw.end());
  auto p = new ISStream(std::istringstream(std::move(s)));
  return std::shared_ptr<IStream>(p);
}

std::shared_ptr<IStream> Stream::readFromString(const std::string &raw) {
  auto p = new ISStream(std::istringstream(raw));
  return std::shared_ptr<IStream>(p);
}

void Stream::reset(std::shared_ptr<Stream> raw) {
  if (raw == nullptr)
    return;
  auto fs = dynamic_cast<std::basic_istream<char> *>(raw.get());
  if (fs) {
    fs->seekg(0);
    return;
  }
}

size_t ISStream::read(char *buffer, size_t expectSize) {
  if (std::istringstream::eof() || std::istringstream::bad())
    return 0;
  return std::istringstream::readsome(buffer, expectSize);
}

size_t IFStream::read(char *buffer, size_t expectSize) {
  if (std::ifstream::eof() || std::ifstream::bad() || std::ifstream::fail())
    return 0;
  auto realSize = std::ifstream::readsome(buffer, expectSize);
  if (realSize < 0)
    return 0;
  else if (realSize == 0) {
    std::ifstream::read(buffer, (std::max)(expectSize / 10, size_t(1)));
    return std::ifstream::gcount();
  }
  return realSize;
}

size_t OSStream::write(char *buffer, size_t expectSize) {
  std::ostringstream::write(buffer, expectSize);
  return expectSize;
}

} // namespace Darabonba
