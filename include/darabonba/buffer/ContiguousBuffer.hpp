#ifndef DARBONBACORE_CONTIGUOUSBUFFER_H_
#define DARBONBACORE_CONTIGUOUSBUFFER_H_

#include <algorithm>
#include <cstring>
#include <darabonba/buffer/IOBuffer.hpp>

namespace Darabonba {
namespace Buffer {
class ContiguousBuffer : public IOBuffer {
public:
  ContiguousBuffer(size_t capacity) : cap_(capacity) {
    begin_ = readPos_ = writePos_ = new char[cap_];
  }
  ContiguousBuffer() : ContiguousBuffer(DEFAULT_CAPACITY) {}

  ContiguousBuffer(const ContiguousBuffer &) = delete;
  ContiguousBuffer(ContiguousBuffer &&obj)
      : cap_(obj.cap_), readPos_(obj.readPos_), writePos_(obj.writePos_) {
    delete[] begin_;
    begin_ = obj.begin_;
    obj.cap_ = 0;
    obj.begin_ = obj.readPos_ = obj.writePos_ = nullptr;
  }
  virtual ~ContiguousBuffer() { delete[] begin_; }

  ContiguousBuffer &operator=(const ContiguousBuffer &) = delete;
  ContiguousBuffer &operator=(ContiguousBuffer &&obj) {
    if (this == &obj)
      return *this;
    delete[] begin_;
    begin_ = obj.begin_;
    cap_ = obj.cap_;
    readPos_ = obj.readPos_;
    writePos_ = obj.writePos_;
    obj.begin_ = obj.readPos_ = obj.writePos_ = nullptr;
    obj.cap_ = 0;
    return *this;
  }

  virtual size_t readableSize() const override { return writePos_ - readPos_; }

  virtual size_t read(char *buffer, size_t expectSize) override {
    auto realSize = (std::min)(expectSize, readableSize());
    memcpy(buffer, readPos_, realSize);
    readPos_ += realSize;
    return realSize;
  }

  virtual size_t writableSize() const override {
    return begin_ + cap_ - writePos_;
  }

  virtual size_t write(char *buffer, size_t expectSize) override {
    ensureWriteable(expectSize);
    memcpy(writePos_, buffer, expectSize);
    writePos_ += expectSize;
    return expectSize;
  }

  void ensureWriteable(size_t size) {
    auto writable_size = writableSize();
    if (writable_size >= size)
      return;
    auto readable_size = readableSize();
    if ((readPos_ - begin_) + writable_size > size) {
      // Move the position of the data to be read.
      memmove(begin_, readPos_, readable_size);
      // reset the pointer
      readPos_ = begin_;
      writePos_ = readPos_ + readable_size;
    } else {
      // reallocate memory
      auto new_size = (readable_size + size) * 2;
      auto newBuf_ = new char[new_size];
      memcpy(newBuf_, readPos_, readable_size);
      // reset the pointer
      delete[] begin_;
      begin_ = readPos_ = newBuf_;
      writePos_ = readPos_ + readable_size;
      cap_ = new_size;
    }
  }

protected:
  enum { DEFAULT_CAPACITY = 4 * 1024 };
  char *begin_;
  size_t cap_;
  char *readPos_;
  char *writePos_;
};
} // namespace Buffer
} // namespace Darabonba

#endif