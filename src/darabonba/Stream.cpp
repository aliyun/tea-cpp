#include <algorithm>
#include <darabonba/Stream.hpp>

namespace Darabonba {
std::shared_ptr<IStream> Stream::readFromFilePath(const std::string &path) {
  return std::shared_ptr<IStream>(
      new IFStream(std::ifstream(path, std::ios::binary)));
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
  if (std::ifstream::eof() || std::ifstream::bad())
    return 0;
  auto realSize = std::ifstream::readsome(buffer, expectSize);
  if (realSize < 0)
    return 0;
  else if (realSize == 0) {
    std::ifstream::read(buffer, std::max(expectSize / 10, size_t(1)));
    return std::ifstream::gcount();
  }
  return realSize;
}

size_t OSStream::write(char *buffer, size_t expectSize) {
  std::ostringstream::write(buffer, expectSize);
  return expectSize;
}

} // namespace Darabonba
