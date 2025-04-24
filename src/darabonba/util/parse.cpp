#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/Util.hpp>
#include <darabonba/http/FileField.hpp>
#include <darabonba/http/MCurlResponse.hpp>
#include <darabonba/http/Query.hpp>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

using std::map;
using std::string;

namespace Darabonba {

Bytes Util::readAsBytes(std::shared_ptr<IStream> raw) {
  if (raw == nullptr)
    return {};
  // Darabonba::IFStream, Darabonba::ISStream
  auto basicIStream = std::dynamic_pointer_cast<std::basic_istream<char>>(raw);
  if (basicIStream) {
    auto pos = basicIStream->tellg();
    basicIStream->seekg(0, std::ios::end);
    auto size = basicIStream->tellg() - pos;
    basicIStream->seekg(pos, std::ios::beg);
    Bytes ret;
    ret.resize(size);
    basicIStream->read(reinterpret_cast<char *>(&ret[0]), size);
    return ret;
  }
  // Http::MCurlResponseBody
  auto respBody = std::dynamic_pointer_cast<Http::MCurlResponseBody>(raw);
  if (respBody) {
    respBody->waitForDone();
    auto size = respBody->readableSize();
    Bytes ret;
    ret.resize(size);
    respBody->read(reinterpret_cast<char *>(&ret[0]), size);
    return ret;
  }
  auto fileFormStream = std::dynamic_pointer_cast<Http::FileFormStream>(raw);
  if (fileFormStream) {
    Bytes ret;
    for (const auto &fileForm : *fileFormStream) {
      std::ifstream ifs(fileForm.content(), std::ios::binary);
      if (ifs.is_open()) {
        ret.insert(ret.end(), std::istreambuf_iterator<char>(ifs),
                   std::istreambuf_iterator<char>());
      } else {
        throw Exception("Failed to open file: " + fileForm.content() + "\n");
      }
    }
    fileFormStream->clear();
    return ret;
  }
  // TODO: custom IStream

  return {};
}

string Util::readAsString(std::shared_ptr<IStream> raw) {
  if (raw == nullptr)
    return "";
  // Darabonba::IFStream, Darabonba::ISStream
  auto basicIStream = std::dynamic_pointer_cast<std::basic_istream<char>>(raw);
  if (basicIStream) {
    auto pos = basicIStream->tellg();
    basicIStream->seekg(0, std::ios::end);
    auto size = basicIStream->tellg() - pos;
    basicIStream->seekg(pos, std::ios::beg);
    string ret;
    ret.resize(size);
    basicIStream->read(reinterpret_cast<char *>(&ret[0]), size);
    return ret;
  }
  // Http::MCurlResponseBody
  auto respBody = std::dynamic_pointer_cast<Http::MCurlResponseBody>(raw);
  if (respBody) {
    respBody->waitForDone();
    auto size = respBody->readableSize();
    string ret;
    ret.resize(respBody->readableSize());
    respBody->read(reinterpret_cast<char *>(&ret[0]), size);
    return ret;
  }
  auto fileFormStream = std::dynamic_pointer_cast<Http::FileFormStream>(raw);
  if (fileFormStream) {
    string ret;
    for (const auto &fileForm : *fileFormStream) {
      std::ifstream ifs(fileForm.content(), std::ios::binary);
      if (ifs.is_open()) {
        ret.insert(ret.end(), std::istreambuf_iterator<char>(ifs),
                   std::istreambuf_iterator<char>());
      } else {
        throw Exception("Failed to open file: " + fileForm.content() + "\n");
      }
    }
    fileFormStream->clear();
    return ret;
  }
  // TODO: custom IStream
  return "";
}

Json Util::readAsJSON(std::shared_ptr<IStream> raw) {
  if (raw == nullptr)
    return {};
  // Darabonba::IFStream, Darabonba::ISStream
  auto basicIStream = std::dynamic_pointer_cast<std::basic_istream<char>>(raw);
  if (basicIStream) {
    return Json::parse(*basicIStream);
  }
  auto str = readAsString(raw);
  return Json::parse(str);
}

string Util::toFormString(const Json &val) {
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

} // namespace Darabonba