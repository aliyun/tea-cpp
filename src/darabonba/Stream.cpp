#include <algorithm>
#include <cstdint>
#include <darabonba/Exception.hpp>
#include <darabonba/Stream.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Darabonba {
namespace {

struct KeptStream {
  int refs = 1;
  std::shared_ptr<IStream> stream;
};

std::mutex &retainedMutex() {
  static std::mutex mutex;
  return mutex;
}

std::unordered_map<std::uint64_t, std::unique_ptr<KeptStream>> &retained() {
  static std::unordered_map<std::uint64_t, std::unique_ptr<KeptStream>> table;
  return table;
}

std::uint64_t &nextRetainedId() {
  static std::uint64_t next = 0;
  return next;
}

} // namespace

extern "C" void darabonba_stream_keep_addref(std::uint64_t id) {
  std::lock_guard<std::mutex> lock(retainedMutex());
  auto found = retained().find(id);
  if (found == retained().end()) {
    return;
  }
  ++found->second->refs;
}

extern "C" void darabonba_stream_keep_release(std::uint64_t id) {
  std::unique_ptr<KeptStream> released;
  {
    std::lock_guard<std::mutex> lock(retainedMutex());
    auto found = retained().find(id);
    if (found != retained().end() && --found->second->refs == 0) {
      released = std::move(found->second);
      retained().erase(found);
    }
  }
}

Json Stream::retain(std::shared_ptr<IStream> stream) {
  if (!stream) {
    return Json(nullptr);
  }
  std::unique_ptr<KeptStream> kept(new KeptStream);
  kept->stream = std::move(stream);
  std::uint64_t id = 0;
  {
    std::lock_guard<std::mutex> lock(retainedMutex());
    id = ++nextRetainedId();
    retained()[id] = std::move(kept);
  }

  try {
    Json value = Json::binary(std::vector<std::uint8_t>(), StreamKeep::kSubtype);
    value.get_binary().set_keep_id(id);
    return value;
  } catch (...) {
    darabonba_stream_keep_release(id);
    throw;
  }
}

bool Stream::claimRetained(std::uint64_t id, std::shared_ptr<IStream> *stream) {
  if (stream == nullptr) {
    return false;
  }
  std::shared_ptr<IStream> claimed;
  {
    std::lock_guard<std::mutex> lock(retainedMutex());
    auto found = retained().find(id);
    if (found == retained().end()) {
      return false;
    }
    claimed = std::move(found->second->stream);
  }
  *stream = std::move(claimed);
  return true;
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
