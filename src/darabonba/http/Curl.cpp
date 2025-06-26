#include <darabonba/Stream.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/Form.hpp>
#include <darabonba/http/URL.hpp>
#include <memory>

namespace Darabonba {
namespace Http {
namespace Curl {

std::string toLower(const std::string& str) {
  std::string lower;
  lower.resize(str.size());
  std::transform(str.begin(), str.end(), lower.begin(), ::tolower);
  return lower;
}

size_t writeHeader(char *buffer, size_t size, size_t nitems, void *userdata) {
  auto header = static_cast<Header *>(userdata);
  std::string s(buffer, size * nitems);
  auto pos = s.find(':');
  if (pos == std::string::npos) {
    // it maybe HTTP request line, HTTP response line or blank line('\r\n')
    return size * nitems;
  }
  header->emplace(s.substr(0, pos), s.substr(pos + 2));
  header->emplace(toLower(s.substr(0, pos)), s.substr(pos + 2));
  return size * nitems;
}

void setCurlRequestBody(CURL *easyHandle,
                        std::shared_ptr<Darabonba::IStream> body) {
  auto fileform = std::dynamic_pointer_cast<FileFormStream>(body);
  if (fileform) {
    curl_mimepart *part;
    fileform->setMine(curl_mime_init(easyHandle));
    auto mime = fileform->mime();
    Json form = fileform->form();
    std::shared_ptr<FileField> file;
    string fileKey = "";
    for (auto it = form.begin(); it != form.end(); ++it) {
      if(isFileFiled(it.value())) {
        file = std::make_shared<FileField>(it.value());
        fileKey = it.key();
        continue;
      }
      part = curl_mime_addpart(mime);
      auto key = it.key();
      auto value = it.value();
      std::string data = "";
      if(value.is_string()) {
        data = value;
      } else {
        data = value.dump();
      }
      curl_mime_name(part, key.c_str());
      curl_mime_data(part, data.c_str(), CURL_ZERO_TERMINATED);
    }

    if(file != nullptr && !file->empty()) {
      part = curl_mime_addpart(mime);
      curl_mime_name(part, fileKey.c_str());
      curl_mime_filename(part, file->filename().c_str());
      curl_mime_type(part, file->contentType().c_str());
      curl_mime_data_cb(part, CURL_ZERO_TERMINATED,
                        readIStream,
                        nullptr,
                        closeIStream,
                        file->content().get());
    }
    curl_easy_setopt(easyHandle, CURLOPT_MIMEPOST, mime);

    return;
  }
  auto is = std::dynamic_pointer_cast<IStream>(body);
  if (is) {
    curl_easy_setopt(easyHandle, CURLOPT_POST, 1L);
    curl_easy_setopt(easyHandle, CURLOPT_READDATA, body.get());
    curl_easy_setopt(easyHandle, CURLOPT_READFUNCTION, readIStream);
    return;
  }
}


void closeIStream(void* userdata) {
  Darabonba::IStream* stream = static_cast<Darabonba::IStream*>(userdata);
  if(stream) {
    delete stream;
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
    std::string firstLowerCase = p.first;
    std::transform(firstLowerCase.begin(), firstLowerCase.end(), firstLowerCase.begin(), ::tolower);

    std::string modifiedSecond = p.second;
    if (firstLowerCase == "content-type") {
      const std::string boundaryPrefix = "multipart/form-data; boundary=";
      if (p.second.compare(0, boundaryPrefix.size(), boundaryPrefix) == 0) {
        modifiedSecond = "multipart/form-data";
      }
    }
    list = curl_slist_append(list, (p.first + ": " + modifiedSecond).c_str());
  }
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  return list;
}

int debugFunction(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr)
{
  switch (type) {
  case CURLINFO_TEXT:
    std::cerr << "Request Info:" << data << std::endl;
    break;
  case CURLINFO_HEADER_OUT:
    std::cerr << "Request Header:" << std::endl;
    std::cerr << std::string(data, size) << std::endl;
    break;
  case CURLINFO_HEADER_IN:
    std::cerr << "Response Header:" << std::endl;
    std::cerr << std::string(data, size) << std::endl;
    break;
  case CURLINFO_DATA_OUT:
    std::cerr << "Request Body:" << std::endl;
    std::cerr << std::string(data, size) << std::endl;
    break;
  case CURLINFO_DATA_IN:
    std::cerr << "Response Body:" << std::endl;
    std::cerr << std::string(data, size) << std::endl;
    break;
  default: /* in case a new one is introduced to shock us */
    return 0;
  }

  return 0;
}
} // namespace Curl
} // namespace Http
} // namespace Darabonba