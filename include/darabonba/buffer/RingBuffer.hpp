#ifndef DARBONBACORE_RINGBUFFER_H_
#define DARBONBACORE_RINGBUFFER_H_
#include <algorithm>
#include <cstring>
#include <darabonba/buffer/IOBuffer.hpp>
#include <list>

namespace Darabonba {
namespace Buffer {
class RingBuffer : public IOBuffer {

protected:
  enum { DEFAULT_SEGMENT_SIZE = 4 * 1024 };
  class Segment {
  public:
    Segment(size_t capacity) : cap(capacity) { data = new char[capacity]; }
    Segment(const Segment &) = delete;
    Segment(Segment &&obj) : cap(obj.cap) {
      delete[] data;
      data = obj.data;
      obj.data = nullptr;
      obj.cap = 0;
    }
    ~Segment() { delete[] data; }

    Segment &operator=(const Segment &) = delete;
    Segment &operator=(Segment &&obj) {
      if (&obj == this)
        return *this;
      delete[] data;
      data = obj.data;
      cap = obj.cap;
      obj.data = nullptr;
      obj.cap = 0;
      return *this;
    }

    size_t capacity() const { return cap; }

    size_t read(char *buffer, size_t expectSize, size_t offset = 0) {
      if (data == nullptr)
        return 0;
      auto cnt = (std::min)(expectSize, cap - offset);
      memcpy(buffer, data + offset, cnt);
      return cnt;
    }

    size_t write(char *buffer, size_t expectSize, size_t offset = 0) {
      if (data == nullptr)
        return 0;
      auto cnt = (std::min)(expectSize, cap - offset);
      memcpy(data + offset, buffer, cnt);
      return cnt;
    }

  protected:
    char *data = nullptr;
    size_t cap = 0;
  };

public:
  RingBuffer(size_t segmentSize = DEFAULT_SEGMENT_SIZE)
      : segmentSize_(segmentSize) {
    writeIter_ = data_.begin();
    addSegment(1);
  }
  RingBuffer(const RingBuffer &) = delete;
  RingBuffer(RingBuffer &&obj)
      : data_(std::move(obj.data_)), writeIter_(obj.writeIter_),
        nRead_(obj.nRead_), nWrite_(obj.nWrite_) {
    obj.nRead_ = obj.nWrite_ = 0;
    obj.writeIter_ = obj.data_.begin();
  }
  virtual ~RingBuffer() {}

  RingBuffer &operator=(const RingBuffer &) = delete;
  RingBuffer &operator=(RingBuffer &&obj) {
    data_ = std::move(obj.data_);
    writeIter_ = obj.writeIter_;
    nRead_ = obj.nRead_;
    nWrite_ = obj.nWrite_;
    obj.nRead_ = obj.nWrite_ = 0;
    obj.writeIter_ = obj.data_.begin();
    return *this;
  }

  virtual size_t readableSize() const override {
    if (data_.empty())
      return 0;
    return segmentSize_ * std::distance(data_.cbegin(),
                                        decltype(data_.cbegin())(writeIter_)) +
           nWrite_ - nRead_;
  }

  virtual size_t writableSize() const override {
    if (data_.empty())
      return 0;
    return std::distance(std::next(decltype(data_.begin())(writeIter_)),
                         data_.cend()) *
               segmentSize_ +
           (segmentSize_ - nWrite_);
  }

  virtual size_t read(char *buffer, size_t expectSize) override {
    auto realSize = (std::min)(expectSize, readableSize());
    for (auto remain = realSize; remain > 0;) {
      auto cnt = data_.front().read(buffer, remain, nRead_);
      buffer += cnt;
      remain -= cnt;
      nRead_ += cnt;
      if (nRead_ == data_.front().capacity())
        forwardReader();
    }
    return realSize;
  }

  virtual size_t write(char *buffer, size_t expectSize) override {
    ensureWritable(expectSize);
    for (auto remain = expectSize; remain > 0;) {
      auto cnt = writeIter_->write(buffer, remain, nWrite_);
      buffer += cnt;
      remain -= cnt;
      nWrite_ += cnt;
      if (nWrite_ == writeIter_->capacity())
        forwardWriter();
    }
    return expectSize;
  }

  void ensureWritable(size_t size) {
    auto remain = writableSize();
    // Make sure there is always space left
    if (remain > size)
      return;
    size_t cnt = (size - remain) / segmentSize_ + 1;
    addSegment(cnt);
  }

protected:
  void forwardReader() {
    auto seg = std::move(data_.front());
    data_.pop_front();
    data_.emplace_back(std::move(seg));
    nRead_ = 0;
  }

  void forwardWriter() {
    if (nWrite_ == 0)
      return;
    ++writeIter_;
    if (writeIter_ == data_.end())
      addSegment(1);
    nWrite_ = 0;
  }

  bool addSegment(size_t n) {
    bool isEnd = writeIter_ == data_.end();
    if (isEnd)
      writeIter_ = data_.emplace(writeIter_, Segment(segmentSize_));
    for (size_t i = isEnd ? 1 : 0; i < n; ++i)
      data_.emplace_back(Segment(segmentSize_));
    return true;
  }

  size_t segmentSize_ = DEFAULT_SEGMENT_SIZE;
  std::list<Segment> data_;

  decltype(data_)::iterator writeIter_;
  size_t nRead_ = 0;
  size_t nWrite_ = 0;
};

} // namespace Buffer
} // namespace Darabonba

#endif