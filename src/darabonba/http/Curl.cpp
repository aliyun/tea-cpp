#include <darabonba/Stream.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/FileField.hpp>
#include <darabonba/http/URL.hpp>
#include <memory>

namespace Darabonba {
namespace Http {
namespace Curl {

size_t writeHeader(char *buffer, size_t size, size_t nitems, void *userdata) {
  auto header = static_cast<Header *>(userdata);
  std::string s(buffer, size * nitems);
  auto pos = s.find(':');
  if (pos == std::string::npos) {
    // it maybe HTTP request line, HTTP response line or blank line('\r\n')
    return size * nitems;
  }
  header->emplace(s.substr(0, pos), s.substr(pos + 2));
  return size * nitems;
}

void setCurlRequestBody(CURL *easyHandle,
                        std::shared_ptr<Darabonba::IStream> body) {
  auto ffs = std::dynamic_pointer_cast<FileFormStream>(body);
  if (ffs) {
    ffs->setMine(curl_mime_init(easyHandle));
    auto mime = ffs->mime();
    for (const auto &fileform : *ffs) {
      auto part = curl_mime_addpart(mime);
      curl_mime_name(part, fileform.filename().c_str());
      curl_mime_filedata(part, fileform.content().c_str());
      curl_mime_type(part, fileform.contentType().c_str());
    }
    curl_easy_setopt(easyHandle, CURLOPT_MIMEPOST, mime);
    return;
  }
  auto is = std::dynamic_pointer_cast<IStream>(body);
  if (is) {
    curl_easy_setopt(easyHandle, CURLOPT_READDATA, body.get());
    curl_easy_setopt(easyHandle, CURLOPT_READFUNCTION, readIStream);
    return;
  }
}

size_t readIStream(char *buffer, size_t size, size_t nitems, void *userdata) {
  auto f = static_cast<IStream *>(userdata);
  if (f == nullptr)
    return 0;
  return f->read(buffer, size * nitems);
}

void setCurlProxy(CURL *curl, const std::string &proxy) {
  URL url(proxy);
  std::string out = url.host() + ":" + std::to_string(url.port());
  curl_easy_setopt(curl, CURLOPT_PROXY, out.c_str());
  if (!url.userInfo().empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, url.userInfo().c_str());
  }
}

curl_slist *setCurlHeader(CURL *curl, const Darabonba::Http::Header &header) {
  curl_slist *list = nullptr;
  for (const auto &p : header) {
    list = curl_slist_append(list, (p.first + ": " + p.second).c_str());
  }
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  return list;
}
} // namespace Curl
} // namespace Http
} // namespace Darabonba