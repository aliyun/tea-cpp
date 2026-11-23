#ifndef DARABONBA_STREAM_KEEP_HPP_
#define DARABONBA_STREAM_KEEP_HPP_

#include <cstdint>

namespace Darabonba {
namespace StreamKeep {

// Historical token shape. Ownership is a field on the JSON object retain()
// returned, never a value decoded from these bytes.
constexpr std::uint64_t kSubtype = 0xDA7ABA11A0000001ULL;
constexpr std::uint64_t kMagic = 0xDABAB0DA57AEAuLL;

} // namespace StreamKeep
} // namespace Darabonba

extern "C" {
void darabonba_stream_keep_addref(std::uint64_t id);
void darabonba_stream_keep_release(std::uint64_t id);
}

#endif
