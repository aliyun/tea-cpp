#ifndef DARABONBA_HTTP_CURL_H_
#define DARABONBA_HTTP_CURL_H_

#include <curl/curl.h>
#include <darabonba/Stream.hpp>
#include <darabonba/http/Header.hpp>
#include <memory>

namespace Darabonba {

namespace Http {

namespace Curl {

/**
 * @note The header callback will be called once for each header and only
 * complete header lines are passed on to the callback. Do not assume that the
 * header line is null-terminated!
 * @ref https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
 */
size_t writeHeader(char *buffer, size_t size, size_t nitems, void *userdata);

size_t readIStream(char *buffer, size_t size, size_t nitems, void *userdata);

size_t readFileFiled(char *buffer, size_t size, size_t nitems, void *userdata);

void closeIStream(void *userdata);

void setCurlRequestBody(CURL *curl, std::shared_ptr<Darabonba::IStream> body);

void setCurlProxy(CURL *curl, const std::string &proxy);

/**
 * @brief Restrict name resolution to IPv4 (CURLOPT_IPRESOLVE =
 * CURL_IPRESOLVE_V4) on this easy handle only. Nothing is set when ipv4Only is
 * false, so the default curl behavior is kept.
 * @return true if the option was set on the handle
 */
bool setCurlIpv4Only(CURL *curl, bool ipv4Only);

curl_slist *setCurlHeader(CURL *curl, const Darabonba::Http::Header &header);

int debugFunction(CURL *curl, curl_infotype type, char *data, size_t size,
                  void *userptr);

} // namespace Curl
} // namespace Http
} // namespace Darabonba

#endif