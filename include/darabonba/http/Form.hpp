#ifndef DARABONBA_HTTP_FORM_H_
#define DARABONBA_HTTP_FORM_H_

#include <darabonba/http/FileField.hpp>
#include <curl/curl.h>
#include <darabonba/Stream.hpp>
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

  static std::shared_ptr<IStream> toFileForm(const Json &form,
                                          const std::string &boundary);

};
} // namespace Http
} // namespace Darabonba

#endif
