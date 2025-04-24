#ifndef DARABONBA_XML_H_
#define DARABONBA_XML_H_

#include <darabonba/Type.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace Darabonba {
class XML {
public:
  static Json parseXml(const std::string &body, const void *resp = nullptr);

  static std::string toXML(const Json &body);

protected:
};
} // namespace Darabonba

#endif
