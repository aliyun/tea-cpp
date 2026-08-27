#include <darabonba/Stream.hpp>
#include <darabonba/Exception.hpp>
#include <darabonba/http/Curl.hpp>
#include <darabonba/http/Form.hpp>
#include <darabonba/http/URL.hpp>
#include <memory>
#include <algorithm>
#include <cctype>

namespace Darabonba {
namespace Http {
namespace Curl {

std::string toLower(const std::string &str) {
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
    auto mime = fileform->getMime();
    Json form = fileform->getForm();
    std::shared_ptr<FileField> file;
    std::string fileKey;
    for (auto it = form.begin(); it != form.end(); ++it) {
      if (isFileFiled(it.value())) {
        file = std::make_shared<FileField>(it.value());
        fileKey = it.key();
        continue;
      }
      part = curl_mime_addpart(mime);
      auto key = it.key();
      auto value = it.value();
      std::string data = "";
      if (value.is_string()) {
        data = value;
      } else {
        data = value.dump();
      }
      curl_mime_name(part, key.c_str());
      curl_mime_data(part, data.c_str(), CURL_ZERO_TERMINATED);
    }

    if (file != nullptr && !file->empty()) {
      fileform->setFileField(file);
      part = curl_mime_addpart(mime);
      curl_mime_name(part, fileKey.c_str());
      curl_mime_filename(part, file->getFilename().c_str());
      curl_mime_type(part, file->getContentType().c_str());
      curl_mime_data_cb(part, CURL_ZERO_TERMINATED, readFileFiled, nullptr,
                        nullptr, file.get());
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

size_t readFileFiled(char *buffer, size_t size, size_t nitems, void *userdata) {
  auto ff = static_cast<FileField *>(userdata);
  if (ff == nullptr)
    return 0;
  auto f = ff->getContent();
  if (f == nullptr)
    return 0;
  return f->read(buffer, size * nitems);
}

size_t readIStream(char *buffer, size_t size, size_t nitems, void *userdata) {
  auto f = static_cast<IStream *>(userdata);
  if (f == nullptr)
    return 0;
  return f->read(buffer, size * nitems);
}

namespace {

std::string normalizeNetworkToken(const std::string &network) {
  std::string value = network;
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

std::string stripSocksScheme(std::string proxy) {
  static const char *prefixes[] = {"socks5h://", "socks5://", "socks4a://",
                                   "socks4://"};
  for (const auto *prefix : prefixes) {
    const std::string scheme(prefix);
    if (proxy.size() >= scheme.size() &&
        std::equal(scheme.begin(), scheme.end(), proxy.begin(),
                   [](char a, char b) {
                     return std::tolower(static_cast<unsigned char>(a)) ==
                            std::tolower(static_cast<unsigned char>(b));
                   })) {
      return proxy.substr(scheme.size());
    }
  }
  return proxy;
}

URL parseProxyEndpoint(const std::string &proxy) {
  std::string endpoint = stripSocksScheme(proxy);
  if (endpoint.find("://") == std::string::npos) {
    endpoint = "http://" + endpoint;
  }
  return URL(endpoint);
}

bool usesRemoteDnsResolution(const std::string &proxy) {
  std::string lower = proxy;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lower.rfind("socks5h://", 0) == 0;
}

} // namespace

void setCurlProxy(CURL *curl, const std::string &proxy) {
  URL url(parseProxyEndpoint(proxy));
  if (url.getHost().empty()) {
    throw ValidateException("InvalidConfiguration",
                            "Invalid proxy URL: " + proxy);
  }
  uint16_t port = url.getPort();
  if (port == 0) {
    port = 1080;
  }
  std::string out = url.getHost() + ":" + std::to_string(port);
  curl_easy_setopt(curl, CURLOPT_PROXY, out.c_str());
  if (!url.getUserInfo().empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, url.getUserInfo().c_str());
  }
}

void validateProxyOptions(const std::string &httpProxy,
                          const std::string &httpsProxy,
                          const std::string &socks5Proxy,
                          const std::string &socks5Network) {
  if (!socks5Network.empty() && socks5Proxy.empty()) {
    throw ValidateException("InvalidConfiguration",
                          "socks5NetWork requires socks5Proxy to be configured");
  }

  if (!socks5Network.empty()) {
    const std::string network = normalizeNetworkToken(socks5Network);
    if (network != "tcp" && network != "udp") {
      throw ValidateException("InvalidConfiguration",
                              "Invalid socks5NetWork: " + socks5Network +
                                  ", expected tcp or udp");
    }
    if (network == "udp") {
      throw ValidateException(
          "InvalidConfiguration",
          "SOCKS5 UDP network is not supported for HTTP requests");
    }
  }

  if (!socks5Proxy.empty()) {
    URL url(parseProxyEndpoint(socks5Proxy));
    if (url.getHost().empty()) {
      throw ValidateException("InvalidConfiguration",
                              "Invalid socks5Proxy: " + socks5Proxy);
    }
  }

  if (!httpProxy.empty()) {
    URL url(parseProxyEndpoint(httpProxy));
    if (url.getHost().empty()) {
      throw ValidateException("InvalidConfiguration",
                              "Invalid httpProxy: " + httpProxy);
    }
  }

  if (!httpsProxy.empty()) {
    URL url(parseProxyEndpoint(httpsProxy));
    if (url.getHost().empty()) {
      throw ValidateException("InvalidConfiguration",
                              "Invalid httpsProxy: " + httpsProxy);
    }
  }
}

void setCurlSocks5Proxy(CURL *curl, const std::string &proxy,
                        const std::string &network) {
  (void)network;
  const bool remoteDns = usesRemoteDnsResolution(proxy);
  curl_easy_setopt(curl, CURLOPT_PROXYTYPE,
                   remoteDns ? CURLPROXY_SOCKS5_HOSTNAME : CURLPROXY_SOCKS5);
  setCurlProxy(curl, proxy);
}

void applyCurlProxyOptions(CURL *curl, const std::string &httpProxy,
                           const std::string &httpsProxy,
                           const std::string &socks5Proxy,
                           const std::string &socks5Network,
                           const std::string &noProxy) {
  validateProxyOptions(httpProxy, httpsProxy, socks5Proxy, socks5Network);

  if (!noProxy.empty()) {
    curl_easy_setopt(curl, CURLOPT_NOPROXY, noProxy.c_str());
  }

  if (!httpProxy.empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    setCurlProxy(curl, httpProxy);
  } else if (!httpsProxy.empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
    setCurlProxy(curl, httpsProxy);
  } else if (!socks5Proxy.empty()) {
    setCurlSocks5Proxy(curl, socks5Proxy, socks5Network);
  }
}

curl_slist *setCurlHeader(CURL *curl, const Darabonba::Http::Header &header) {
  curl_slist *list = nullptr;
  for (const auto &p : header) {
    std::string firstLowerCase = p.first;
    std::transform(firstLowerCase.begin(), firstLowerCase.end(),
                   firstLowerCase.begin(), ::tolower);

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

int debugFunction(CURL *handle, curl_infotype type, char *data, size_t size,
                  void *userptr) {
  (void)handle;  // unused parameter
  (void)userptr; // unused parameter
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