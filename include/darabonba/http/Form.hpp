#ifndef DARABONBA_HTTP_FORM_H_
#define DARABONBA_HTTP_FORM_H_

#include <darabonba/String.hpp>
#include <darabonba/http/Query.hpp>
#include <string>

namespace Darabonba {
namespace Http {

class Form {
public:
  /**
   * @brief Encode the string into application/x-www-form-urlencoded format
   * @ref https://en.wikipedia.org/wiki/URL_encoding
   * @ref https://datatracker.ietf.org/doc/html/rfc3986
   */
  static std::string encode(const std::string &content);

  static std::string getBoundary(); 

  static std::string toFormString(const Json &val);

};

} // namespace Http
} // namespace Darabonba

#endif
