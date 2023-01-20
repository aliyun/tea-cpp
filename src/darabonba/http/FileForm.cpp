#include <darabonba/http/FileField.hpp>

namespace Darabonba {
namespace Http {
std::shared_ptr<Stream> FileField::toFileForm(const Json &form,
                                              const std::string &boundary) {

  auto p = new FileFormStream;
  p->emplace_back(FileField(form));
  return std::shared_ptr<Stream>(p);
}
} // namespace Http
} // namespace Darabonba