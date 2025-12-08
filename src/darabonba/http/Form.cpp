#include <darabonba/String.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/http/Form.hpp>
#include <darabonba/http/FileField.hpp>

namespace Darabonba {
namespace Http {

std::string Form::encode(const std::string &content) {
  auto ret = Query::encode(content);
  return String::replace(ret, "%20", "+");
}

std::string Form::getBoundary() {
  return Core::uuid();
}

std::string Form::toFormString(const Json &val) {
  if (val.empty() || val.is_null()) {
    return "";
  }
  std::stringstream tmp;
  for (const auto &el : val.items()) {
    tmp << Http::Query::encode(el.key()) << " = "
        << Http::Query::encode(el.value().is_string()
                                   ? el.value().get<std::string>()
                                   : el.value().dump())
        << "&";
  }
  std::string formstring = tmp.str();
  formstring.pop_back();
  return formstring;
}


std::shared_ptr<IStream> Form::toFileForm(const Json &form,
                                              const std::string &boundary) {

  auto p = std::make_shared<FileFormStream>(form);
  p->setBoundary(boundary);
  return std::shared_ptr<IStream>(p);
}

} // namespace Http
} // namespace Darabonba
