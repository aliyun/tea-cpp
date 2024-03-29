#include <darabonba/core.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include <cpprest/http_client.h>
#include <fstream>
#include <utility>

#include <cpprest/interopstream.h> // Bridges for integrating Async streams with STL and WinRT streams
#include <cpprest/uri.h> // URI library

using namespace std;

void Darabonba::Core::sleep(const shared_ptr<int> &sleep_time) {
  int s = !sleep_time ? 0 : *sleep_time;
  boost::this_thread::sleep_for(boost::chrono::seconds(s));
}

int Darabonba::Core::getBackoffTime(const shared_ptr<boost::any> &backoff,
                                    const shared_ptr<int> &retry_times) {
  if (!backoff) {
    return 0;
  }
  int rt = !retry_times ? 0 : *retry_times;
  int back_off_time = 0;
  map<string, boost::any> backoff_val =
      boost::any_cast<map<string, boost::any>>(*backoff);
  string policy = "no";
  if (backoff_val.find("policy") != backoff_val.end()) {
    policy = boost::any_cast<string>(backoff_val["policy"]);
  }
  if (policy == "no") {
    return back_off_time;
  }

  if (backoff_val.find("period") != backoff_val.end()) {
    int period = boost::any_cast<int>(backoff_val.at("period"));
    back_off_time = period;
    if (back_off_time <= 0) {
      return rt;
    }
  }
  return back_off_time;
}

bool Darabonba::Core::isRetryable(const exception &ex) {
  auto *e = boost::current_exception_cast<Darabonba::Error>();
  return e == nullptr;
}

bool Darabonba::Core::isRetryable(const boost::exception &ex) {
  auto *e = boost::current_exception_cast<Darabonba::Error>();
  return e == nullptr;
}

bool Darabonba::Core::allowRetry(const shared_ptr<boost::any> &retry,
                                 const shared_ptr<int> &retry_times,
                                 const shared_ptr<int> &now) {
  if (!retry) {
    return false;
  }
  int r = !retry_times ? 0 : *retry_times;
  map<string, boost::any> retry_val =
      boost::any_cast<map<string, boost::any>>(*retry);
  if (retry_val.empty()) {
    return false;
  }
  if (retry_val.find("maxAttempts") != retry_val.end()) {
    boost::any max_attempts_any = retry_val.at("maxAttempts");
    int maxAttempts = 0;
    if (typeid(int) == max_attempts_any.type()) {
      maxAttempts = boost::any_cast<int>(retry_val.at("maxAttempts"));
    }
    return maxAttempts >= r;
  }
  return false;
}

Darabonba::Response Darabonba::Core::doAction(const boost::any &request) {
  map<string, boost::any> runtime;
  Darabonba::Request req;
  if (typeid(shared_ptr<Darabonba::Request>) == request.type()) {
    shared_ptr<Darabonba::Request> requestPtr =
        boost::any_cast<shared_ptr<Darabonba::Request>>(request);
    req = *requestPtr;
  } else if (typeid(Darabonba::Request) == request.type()) {
    req = boost::any_cast<Darabonba::Request>(request);
  }
  return Darabonba::Core::doAction(req, runtime);
}

string lowercase(string str) {
  transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return tolower(c); });
  return str;
}

string uppercase(string str) {
  transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return toupper(c); });
  return str;
}

string url_encode(const string &str) {
  ostringstream escaped;
  escaped.fill('0');
  escaped << hex;

  for (char c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }
    escaped << uppercase;
    escaped << '%' << setw(2) << int((unsigned char)c);
    escaped << nouppercase;
  }

  return escaped.str();
}

string http_query_str(map<string, string> query) {
  string str;
  if (!query.empty()) {
    vector<string> keys;
    keys.reserve(query.size());
    for (auto &it : query) {
      keys.push_back(it.first);
    }
    sort(keys.begin(), keys.end());
    vector<string> arr;
    for (const auto &k : keys) {
      if (query[k].empty()) {
        continue;
      }
      string tmp = k + "=" + url_encode(query[k]);
      arr.push_back(tmp);
    }
    str = boost::join(arr, "&");
  }
  return str;
}

string compose_url(const string &host, const map<string, string> &params,
                   const string &pathname, const string &schema,
                   const string &port) {
  string url = schema + "://" + host;
  url = lowercase(url);
  if (port != "80" && port != "443") {
    url = url + ":" + port;
  }
  url = url + pathname;
  if (!params.empty()) {
    string queryString = http_query_str(params);
    if (url.find('?') != string::npos) {
      if (url.back() != '&') {
        url = url + "&" + queryString;
      } else {
        url = url + queryString;
      }
    } else {
      url = url + "?" + queryString;
    }
  }
  return url;
}

std::vector<std::string> explode(const std::string &str,
                                 const std::string &delimiter) {
  int pos = str.find(delimiter, 0);
  int pos_start = 0;
  int split_n = pos;
  string line_text(delimiter);

  std::vector<std::string> dest;

  while (pos > -1) {
    line_text = str.substr(pos_start, split_n);
    pos_start = pos + 1;
    pos = str.find(delimiter, pos + 1);
    split_n = pos - pos_start;
    dest.push_back(line_text);
  }
  line_text = str.substr(pos_start, str.length() - pos_start);
  dest.push_back(line_text);
  return dest;
}

template <typename T> T get(map<string, boost::any> m, string k, T _default) {
  if (m.find(k) != m.end()) {
    if (typeid(T) == m[k].type()) {
      return boost::any_cast<T>(m.at(k));
    }
  }
  return _default;
}

web::http::client::http_client_config
Darabonba::Core::httpConfig(const Darabonba::Request &req,
                            map<string, boost::any> runtime) {
  web::http::client::http_client_config cfg;
  int timeout = get<int>(runtime, "readTimeout", 0);
  timeout += get<int>(runtime, "connectTimeout", 0);
  if (timeout == 0) {
    timeout = 30;
  }
  std::chrono::microseconds t(timeout);
  cfg.set_timeout(utility::seconds(timeout));
  string noProxy = get<string>(runtime, "noProxy", "");
  bool use_proxy = true;
  if (!noProxy.empty()) {
    vector<string> no_proxy_list = explode(noProxy, ",");
    for (const auto &no_proxy_item : no_proxy_list) {
      if (req.host == no_proxy_item) {
        use_proxy = false;
      }
    }
  }
  if (use_proxy) {
    if (req.protocol == "http") {
      string httpProxy = get<string>(runtime, "httpProxy", "");
      if (!httpProxy.empty()) {
        web::web_proxy http_proxy(httpProxy);
        cfg.set_proxy(http_proxy);
      }
    } else if (req.protocol == "https") {
      string httpsProxy = get<string>(runtime, "httpsProxy", "");
      if (!httpsProxy.empty()) {
        web::web_proxy https_proxy(httpsProxy);
        cfg.set_proxy(https_proxy);
      }
    }
  }
  return cfg;
}

Darabonba::Response Darabonba::Core::doAction(const boost::any &request,
                                              const boost::any &runtime) {
  Darabonba::Request req;
  if (typeid(shared_ptr<Darabonba::Request>) == request.type()) {
    shared_ptr<Darabonba::Request> requestPtr =
        boost::any_cast<shared_ptr<Darabonba::Request>>(request);
    req = *requestPtr;
  } else if (typeid(Darabonba::Request) == request.type()) {
    req = boost::any_cast<Darabonba::Request>(request);
  }
  string protocol = req.protocol;
  string port = to_string(req.port);
  string method = uppercase(req.method);
  string pathname = req.pathname;
  map<string, string> query;
  for (const auto &param : req.query) {
    string key = param.first;
    string val = param.second;
    query.insert(pair<string, string>(key, val));
  }

  map<string, string> headers = req.headers;
  string host = req.host;
  if (headers.find("host") != headers.end()) {
    host = headers["host"];
    req.host = host;
  }
  string url = compose_url(host, query, pathname, protocol, port);
  map<string, boost::any> options;
  if (typeid(map<string, boost::any>) == runtime.type()) {
    options = boost::any_cast<map<string, boost::any>>(runtime);
  } else if (typeid(shared_ptr<map<string, boost::any>>) == runtime.type()) {
    shared_ptr<map<string, boost::any>> r =
        boost::any_cast<shared_ptr<map<string, boost::any>>>(runtime);
    options = *r;
  }
  web::http::client::http_client_config cfg = httpConfig(req, options);
  web::http::client::http_client client(
      web::uri(utility::conversions::to_string_t(url)), cfg);
  web::http::http_response response;

  // set headers
  web::http::http_request http_request;
  for (const auto &i : headers) {
    http_request.headers().add(i.first, i.second);
  }
  // set body
  if (req.body && !req.body->empty()) {
    string body = req.body->read();
    http_request.set_body(body);
  }

  http_request.set_method(method);

  response = client.request(http_request).get();
  if (nullptr != getenv("DEBUG")) {
    printf("status_code : %hu\n", response.status_code());
    printf("response : %s\n", response.to_string().c_str());
  }
  Darabonba::Response dara_response;
  dara_response.body = make_shared<Darabonba::Stream>(response.body());
  dara_response.statusCode = response.status_code();
  for (const auto &it : response.headers()) {
    string key = boost::to_lower_copy(it.first);
    dara_response.headers[key] = it.second;
  }
  dara_response.statusMessage = response.reason_phrase();
  return dara_response;
}
