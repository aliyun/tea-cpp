/*
* Copyright 1999-2019 Alibaba Cloud All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <darabonba/core.hpp>
#include <sstream>
#include <memory>
#include <stdlib.h>
#include <string>

using namespace Darabonba;
using namespace std;
using json = nlohmann::json;

struct ctx{
  std::istream *is;
  int total;
};

size_t HttpClient::readCallback(char *dest, size_t size, size_t nmemb, void *userp) {
  ctx *w = (ctx *)userp;
  std::istream *is = w->is;
  size_t buffer_size = size * nmemb;
  int pos = is->tellg();
  int left = w->total - pos;
  if(left > 0){
    if(buffer_size > left) {
      buffer_size = left;
    }
    is->read(dest, buffer_size);
    return buffer_size; /* we copied this many bytes */
  }
  return 0;
}

size_t HttpClient::recvBody(char *ptr, size_t size, size_t nmemb, void *userdata) {
  ResponseBody* out = static_cast<ResponseBody *>(userdata);
  if(!out->getReady()) {
    out->setReady(true);
  }
  bool flow = out->getFlow();
  if(!flow) {
    // 还没开始
    return CURL_WRITEFUNC_PAUSE;
  }
  size_t buffer_size = size * nmemb;
  out->write(ptr, buffer_size);
  return buffer_size;
}


size_t recvHeaders(char *buffer, size_t size, size_t nitems, void *userdata) {
 Response *response = static_cast<Response *>(userdata);
 std::string line(buffer);
 auto pos = line.find(':');
 if (pos == line.npos) {
  response->setStatusMessage(line);
  return nitems * size;
 }
 std::string name = line.substr(0, pos);
 std::string value = line.substr(pos + 2);
 size_t p = 0;
 if ((p = value.rfind('\r')) != value.npos)
   value[p] = '\0';
 response->setHeader(name, value);
 return nitems * size;
}

void setCUrlProxy(CURL *curl, const string &proxy) {
  Url url = Url(proxy);
  std::ostringstream out;
  out << url.host() << ":" << url.port();
  curl_easy_setopt(curl, CURLOPT_PROXY, out.str().c_str());
  if(!url.userInfo().empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, url.userInfo().c_str());
  }
}


HttpClient::HttpClient(CURLM *mcurl)
   : mCurlHandle_(mcurl) {}

HttpClient::~HttpClient() { }

shared_ptr<Response>
HttpClient::doAction(const Request &request, const RuntimeOptions &runtimeOptions) {
   CURL *curl = curl_easy_init();
   shared_ptr<Response> resp = make_shared<Response>(mCurlHandle_, curl);

   long connect_timeout = runtimeOptions.connectTimeout();
   long read_timeout = runtimeOptions.readTimeout();
   bool ignore_SSL = runtimeOptions.ignoreSSL();
   string httpProxy = runtimeOptions.httpProxy();
   string httpsProxy = runtimeOptions.httpsProxy();
   string noProxy = runtimeOptions.noProxy();

   const string url = request.url();
   string method = request.method();
   curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

  curl_slist *list = nullptr;
  auto headers = request.headers();
  for (const auto &p : headers) {
    std::string str = p.first;
    str.append(": ").append(p.second);
    list = curl_slist_append(list, str.c_str());
  }

   shared_ptr<RequestBody> body = request.body();
   if(body) {
     body->read(curl);
   } else {
     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
   }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

   curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
   if (ignore_SSL) {
     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
   } else {
     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
   }
   curl_easy_setopt(curl, CURLOPT_HEADERDATA, resp.get());
   curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeaders);

   if (connect_timeout > 0) {
     curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connect_timeout);
   }

   if (read_timeout > 0) {
     curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, read_timeout);
   }

   if(!httpProxy.empty()) {
     curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
     setCUrlProxy(curl, httpProxy);
   }

   if(!httpsProxy.empty()) {
     curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
     setCUrlProxy(curl, httpsProxy);
   }

   if(!noProxy.empty()) {
     curl_easy_setopt(curl, CURLOPT_NOPROXY, noProxy.c_str());
   }

   curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp->body());
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpClient::recvBody);
//   setCUrlProxy(curl, proxy());
   string debug = Core::getEnv("DEBUG");
   if (debug == "sdk") {
     curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
   }
   CURLMcode res;
   curl_multi_add_handle(mCurlHandle_, curl);
   int still_running;
   do {
     if(resp->body()->getReady()) {
       break;
     }
     Core::sleep(1);
     res = curl_multi_perform(mCurlHandle_, &still_running);
     if(res) {
       string message = string("curl failed, code") + to_string((int)res);
       throw RetryableError::New("DaraRequestError", message);
     }
   } while(still_running);

   curl_slist_free_all(list);
   return resp;
}