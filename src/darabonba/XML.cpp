#include <darabonba/XML.hpp>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <tinyxml2/tinyxml2.h>

static std::string jsonToXmlHelper(const Darabonba::Json &obj,
                                   const std::string &parentKey) {
  static const std::string defaultArrayKey = "item";
  std::ostringstream oss;
  if (obj.is_object()) {
    for (const auto &item : obj.items()) {
      const auto &val = item.value();
      if (val.is_array()) {
        oss << jsonToXmlHelper(val, item.key());
      } else {
        oss << '<' << item.key() << '>'
            << jsonToXmlHelper(item.value(), item.key()) << "</" << item.key()
            << '>';
      }
    }
  } else if (obj.is_array()) {
    const auto &key = parentKey.empty() ? defaultArrayKey : parentKey;
    for (const auto &elem : obj) {
      oss << '<' << key << '>' << jsonToXmlHelper(elem, "") << "</" << key
          << '>';
    }
  } else {
    if (obj.is_string()) {
      // ignore the double quotes
      oss << obj.get<std::string>();
    } else {
      oss << obj;
    }
  }
  return oss.str();
}

static std::string jsonToXml(const Darabonba::Json &obj,
                             const std::string &parentKey) {
  if (parentKey.empty() || obj.is_array())
    return jsonToXmlHelper(obj, parentKey);
  return '<' + parentKey + '>' + jsonToXmlHelper(obj, parentKey) + "</" +
         parentKey + '>';
}

static Darabonba::Json xmlToJson(tinyxml2::XMLElement *elem) {
  Darabonba::Json result;
  // process the attribute
  for (auto attr = elem->FirstAttribute(); attr; attr = attr->Next()) {
    result["@attributes"][attr->Name()] = attr->Value();
  }
  // process the children node
  for (auto child = elem->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    // handle duplicate nodes
    auto it = result.find(child->Name());
    if (it != result.end()) {
      if (it->is_array()) {
        it->emplace_back(xmlToJson(child));
      } else {
        auto array = Darabonba::Json::array();
        array.emplace_back(std::move(it.value()));
        array.emplace_back(xmlToJson(child));
        it.value() = std::move(array);
      }
    } else {
      result[child->Name()] = xmlToJson(child);
    }
  }
  if (result.empty()) {
    try {
      result = Darabonba::Json::parse(elem->GetText());
    } catch (Darabonba::Json::parse_error e) {
      // parse to string
      result = Darabonba::Json::parse('"' + std::string(elem->GetText()) + '"');
    }
  }
  return result;
}

static Darabonba::Json xmlToJson(const void *data, size_t size) {
  tinyxml2::XMLDocument doc;
  Darabonba::Json result;
  doc.Parse(reinterpret_cast<const char *>(data), size);
  return xmlToJson(doc.RootElement());
}

namespace Darabonba {
Json XML::parseXml(const std::string &body, const void *resp) {
  return xmlToJson(reinterpret_cast<const void *>(body.c_str()), body.size());
}

std::string XML::toXML(const Json &body) { return jsonToXml(body, ""); }

} // namespace Darabonba