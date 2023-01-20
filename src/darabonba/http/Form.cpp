#include <darabonba/http/Form.hpp>
#include <darabonba/Core.hpp>

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
  string formstring = tmp.str();
  formstring.pop_back();
  return formstring;
}

} // namespace Http
} // namespace Darabonba
