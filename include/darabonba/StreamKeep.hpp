#ifndef DARABONBA_STREAM_KEEP_HPP_
#define DARABONBA_STREAM_KEEP_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Darabonba {
namespace StreamKeep {

// Reserved binary subtype. A JSON value with this subtype owns the stream it
// points at: copying the value keeps the stream alive, and destroying the last
// copy releases it even if nobody claims the stream.
constexpr std::uint64_t kSubtype = 0xDA7ABA11A0000001ULL;
constexpr std::uint64_t kMagic = 0xDABAB0DA57AEAuLL;

inline bool decode(const void *data, std::size_t size, void **keep) {
  if (data == nullptr || keep == nullptr ||
      size != sizeof(std::uint64_t) + sizeof(void *)) {
    return false;
  }
  std::uint64_t magic = 0;
  void *pointer = nullptr;
  const auto *bytes = static_cast<const unsigned char *>(data);
  std::memcpy(&magic, bytes, sizeof(magic));
  std::memcpy(&pointer, bytes + sizeof(magic), sizeof(pointer));
  if (magic != kMagic || pointer == nullptr) {
    return false;
  }
  *keep = pointer;
  return true;
}

} // namespace StreamKeep
} // namespace Darabonba

extern "C" {
void darabonba_stream_keep_addref(void *keep);
void darabonba_stream_keep_release(void *keep);
}

#endif
