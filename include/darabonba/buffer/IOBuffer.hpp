#ifndef DARBONBACORE_IOBUFFER_H_
#define DARBONBACORE_IOBUFFER_H_
#include <cstddef>

namespace Darabonba {
namespace Buffer {

class IOBuffer {
public:
  virtual size_t read(char *buffer, size_t expectSize) = 0;
  virtual size_t write(char *buffer, size_t expectSize) = 0;

  virtual size_t readableSize() const = 0;
  virtual size_t writableSize() const = 0;
  virtual ~IOBuffer() {}
};

} // namespace Buffer
} // namespace Darabonba

#endif
