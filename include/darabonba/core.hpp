#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <iostream>
#include <utility>
#include <sstream>
#include <json.hpp>
#include <curl/curl.h>

#if defined(_WIN32)
#include <objbase.h>
#include <Windows.h>
#else
#include <uuid/uuid.h>
#endif

#define GUID_LEN 64

#define DARABONBA_COMMA ,

#define DARABONBA_JSON_TO(name, key) \
  if(p._##key) { \
    j[#name] = *(p._##key); \
  }

#define DARABONBA_JSON_FROM(name, key, type) \
  if(j.find(#name) != j.end()) {             \
    if(j[#name].is_null()) {                   \
      p._##key = nullptr;                   \
    } else{                                    \
      p._##key = make_shared<type>(j[#name]);  \
    }                                           \
  }

using namespace std;
using json = nlohmann::json;

namespace Darabonba {

// default MCUrl for all request
static CURLM *defaultMCUrl = nullptr;

class Model {
public:
  Model() = default;
  ~Model() = default;
  virtual void validate(const json &map) = 0;
  virtual json toMap() = 0;
  static void validateRequired(const string &field_name,
                                      const json &map);
  static void validateMaxLength(const string &field_name,
                                const json &map, int val);
  static void validateMinLength(const string &field_name,
                                const json &map, int val);
  template <typename T>static void validateMaximum(const string &field_name,
                              const json &map, T val);
  template <typename T>static void validateMinimum(const string &field_name,
                              const json &map, T val);
  static void validatePattern(const string &field_name,
                              const json &map,
                              const string &val);
};

class RequestBody {
public:
  RequestBody(string body): _body(body) { };
  RequestBody() { };
  virtual void read(CURL* curl) {
    if(_body.empty()){
      return;
    }
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)_body.size());
  }

  virtual struct curl_slist* headerChecker(struct curl_slist* list) {
    return list;
  }

  void setBody(string body) {
    _body = body;
  }

private:
  string _body;
};

class Request {
public:
  Request();
  ~Request();

  const string url() const;

  string protocol() const {
      return _protocol;
  };

  void setProtocol(string protocol) {
      _protocol = protocol;
  };

  string host() const {
      return _host;
  };

  void setHost(string host) {
      _host = host;
  };

  int32_t port() const {
      return _port;
  };

  void setPort(int32_t port) {
    _port = port;
  };

  string method() const {
      return _method;
  };

  void setMethod(string method) {
    _method = method;
  };

  string pathname() const {
      return _pathname;
  };

  void setPathname(string pathname) {
    _pathname = pathname;
  };

  const map<string, string>& query() const {
      return _query;
  };

  void setQuery(const map<string, string>& query) {
    _query = query;
  };

  shared_ptr<RequestBody> body() const {
      return _body;
  };

  void setBody(shared_ptr<RequestBody> body) {
    _body = body;
  };

  const map<string, string>& headers() const {
      return _headers;
  };

  void setHeaders(const map<string, string>& headers) {
    _headers = headers;
  };

  void addQuery(string key, string value) {
    _query.emplace(key, value);
  }

  void addHeader(string key, string value) {
    _headers.emplace(key, value);
  }

private:
  string _protocol = "http";
  string _host;
  int32_t _port = 80;
  string _method = "GET";
  string _pathname;
  map<string, string> _query;
  shared_ptr<RequestBody> _body;
  map<string, string> _headers;
};

class ResponseBody {
public:
  ResponseBody(CURLM *multi_handle, CURL* curl)
      : _mcurl(multi_handle), _curl(curl), _os(new stringstream("")) {};
  ~ResponseBody() {
    delete _os;
    curl_easy_cleanup(_curl);

  };
  size_t read();
  void write(char *ptr, size_t size);

  bool getReady(){
    return _ready;
  }

  bool setReady(bool ready){
    return _ready = ready;
  }

  bool getFlow(){
    return _flow;
  }

  string str(){
    return _os->str();
  }

private:
  CURLM* _mcurl;
  CURL* _curl;
  stringstream* _os;
  size_t _size = 0;
  bool _flow = false;
  bool _ready = false;
};

class Response {
public:
  Response(CURLM *multi_handle, CURL* curl):
     _curl(curl), respBody(multi_handle, curl) {};
  ~Response() { };

  void setHeader(string key, string value) {
    _headers.emplace(key, value);
  };

  string header(string key) {
      return _headers[key];
  };

  long statusCode() {
    if(_statusCode == 0) {
      curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &_statusCode);
    }
    return _statusCode;
  };

  string statusMessage() {
    return _statusMessage;
  };

  void setStatusMessage(string statusMessage) {
    _statusMessage = statusMessage;
  };

  ResponseBody* body() {
    return &respBody;
  };

private:
  CURL* _curl;
  ResponseBody respBody;
  long _statusCode = 0;
  string _statusMessage;
  map<string, string> _headers;
};



class RuntimeOptions : public Model {
public:
  RuntimeOptions() {}
  RuntimeOptions(const json &map);
  friend void to_json(json& j, const RuntimeOptions& p){
    DARABONBA_JSON_TO(autoretry, autoretry)
    DARABONBA_JSON_TO(ignoreSSL, ignoreSSL)
    DARABONBA_JSON_TO(key, key)
    DARABONBA_JSON_TO(cert, cert)
    DARABONBA_JSON_TO(ca, ca)
    DARABONBA_JSON_TO(max_attempts, maxAttempts)
    DARABONBA_JSON_TO(backoff_policy, backoffPolicy)
    DARABONBA_JSON_TO(backoff_period, backoffPeriod)
    DARABONBA_JSON_TO(readTimeout, readTimeout)
    DARABONBA_JSON_TO(connectTimeout, connectTimeout)
    DARABONBA_JSON_TO(httpProxy, httpProxy)
    DARABONBA_JSON_TO(httpsProxy, httpsProxy)
    DARABONBA_JSON_TO(noProxy, noProxy)
    DARABONBA_JSON_TO(maxIdleConns, maxIdleConns)
    DARABONBA_JSON_TO(localAddr, localAddr)
    DARABONBA_JSON_TO(socks5Proxy, socks5Proxy)
    DARABONBA_JSON_TO(socks5NetWork, socks5NetWork)
    DARABONBA_JSON_TO(keepAlive, keepAlive)
  }
  friend void from_json(const json& j, RuntimeOptions& p){
    DARABONBA_JSON_FROM(autoretry, autoretry, bool)
    DARABONBA_JSON_FROM(ignoreSSL, ignoreSSL, bool)
    DARABONBA_JSON_FROM(key, key, string)
    DARABONBA_JSON_FROM(cert, cert, string)
    DARABONBA_JSON_FROM(ca, ca, string)
    DARABONBA_JSON_FROM(max_attempts, maxAttempts, int64_t)
    DARABONBA_JSON_FROM(backoff_policy, backoffPolicy, string)
    DARABONBA_JSON_FROM(backoff_period, backoffPeriod, int64_t)
    DARABONBA_JSON_FROM(readTimeout, readTimeout, int64_t)
    DARABONBA_JSON_FROM(connectTimeout, connectTimeout, int64_t)
    DARABONBA_JSON_FROM(httpProxy, httpProxy, string)
    DARABONBA_JSON_FROM(httpsProxy, httpsProxy, string)
    DARABONBA_JSON_FROM(noProxy, noProxy, string)
    DARABONBA_JSON_FROM(maxIdleConns, maxIdleConns, int64_t)
    DARABONBA_JSON_FROM(localAddr, localAddr, string)
    DARABONBA_JSON_FROM(socks5Proxy, socks5Proxy, string)
    DARABONBA_JSON_FROM(socks5NetWork, socks5NetWork, string)
    DARABONBA_JSON_FROM(keepAlive, keepAlive, bool)
  }
  json toMap();
  void validate(const json &map);
  bool autoretry() const;
  void setAutoretry(bool autoretry);
  bool ignoreSSL() const;
  void setIgnoreSSL(bool ignoreSSL);
  string key() const;
  void setKey(string key);
  string cert() const;
  void setCert(string cert);
  string ca() const;
  void setCa(string ca);
  int64_t maxAttempts() const;
  void setMaxAttempts(int64_t maxAttempts);
  string backoffPolicy() const;
  void setBackoffPolicy(string backoffPolicy);
  int64_t backoffPeriod() const;
  void setBackoffPeriod(int64_t backoffPeriod);
  int64_t readTimeout() const;
  void setReadTimeout(int64_t readTimeout);
  int64_t connectTimeout() const;
  void setConnectTimeout(int64_t connectTimeout);
  string httpProxy() const;
  void setHttpProxy(string httpProxy);
  string httpsProxy() const;
  void setHttpsProxy(string httpsProxy);
  string noProxy() const;
  void setNoProxy(string noProxy);
  int64_t maxIdleConns() const;
  void setMaxIdleConns(int64_t maxIdleConns);
  string localAddr() const;
  void setLocalAddr(string localAddr);
  string socks5Proxy() const;
  void setSocks5Proxy(string socks5Proxy);
  string socks5NetWork() const;
  void setSocks5NetWork(string socks5NetWork);
  bool keepAlive() const;
  void setKeepAlive(bool keepAlive);
  ~RuntimeOptions() = default;
private:
  // whether to try again
  shared_ptr<bool> _autoretry;

  // ignore SSL validation
  shared_ptr<bool> _ignoreSSL;

  // privite key for client certificate
  shared_ptr<string> _key;

  // client certificate
  shared_ptr<string> _cert;

  // server certificate
  shared_ptr<string> _ca;

  // maximum number of retries
  shared_ptr<int64_t> _maxAttempts;

  // backoff policy
  shared_ptr<string> _backoffPolicy;

  // backoff period
  shared_ptr<int64_t> _backoffPeriod;

  // read timeout
  shared_ptr<int64_t> _readTimeout;

  // connect timeout
  shared_ptr<int64_t> _connectTimeout;

  // http proxy url
  shared_ptr<string> _httpProxy;

  // https Proxy url
  shared_ptr<string> _httpsProxy;

  // agent blacklist
  shared_ptr<string> _noProxy;

  // maximum number of connections
  shared_ptr<int64_t> _maxIdleConns;

  // local addr
  shared_ptr<string> _localAddr;

  // SOCKS5 proxy
  shared_ptr<string> _socks5Proxy;

  // SOCKS5 netWork
  shared_ptr<string> _socks5NetWork;

  // whether to enable keep-alive
  shared_ptr<bool> _keepAlive;

};



class Core {
public:
  static shared_ptr<Response> doAction(const Darabonba::Request &request,
                           const Darabonba::RuntimeOptions &runtime);
  static shared_ptr<Response> doAction(const Darabonba::Request &request);
  static bool allowRetry(const json &retry, int retry_times);
  static int getBackoffTime(const json &backoff, int retry_times);
  static void sleep(int sleep_time);
  static string uuid();
  static string getEnv(const string env);
  static json merge(const json& jf, const json& jb);
//  static web::http::client::http_client_config
//  httpConfig(const Darabonba::Request &req, map<string, boost::any> runtime);
};

class Url {
public:
  explicit Url(const std::string &url = "");
  Url(const Url &other) = default;
  Url(Url &&other) = default;
  ~Url();
  Url &operator=(const Url &url) = default;
  Url &operator=(Url &&other) = default;
  bool operator==(const Url &url) const;
  bool operator!=(const Url &url) const;

  std::string authority() const;
  void clear();
  std::string fragment() const;
  void fromString(const std::string &url);
  bool hasFragment() const;
  bool hasQuery() const;
  std::string host() const;
  bool isEmpty() const;
  bool isValid() const;
  int port() const;
  std::string password() const;
  std::string path() const;
  std::string query() const;
  std::string scheme() const;
  void setAuthority(const std::string &authority);
  void setFragment(const std::string &fragment);
  void setHost(const std::string &host);
  void setPassword(const std::string &password);
  void setPath(const std::string &path);
  void setPort(int port);
  void setQuery(const std::string &query);
  void setScheme(const std::string &scheme);
  void setUserInfo(const std::string &userInfo);
  void setUserName(const std::string &userName);
  std::string toString() const;
  std::string userInfo() const;
  std::string userName() const;

private:
  std::string scheme_;
  std::string userName_;
  std::string password_;
  std::string host_;
  std::string path_;
  int port_;
  std::string query_;
  std::string fragment_;
};

class HttpClient {
public:
  HttpClient(CURLM *mcurl);
  ~HttpClient();

  shared_ptr<Response> doAction(const Request &request, const RuntimeOptions &runtimeOptions);
  static size_t recvBody(char *ptr, size_t size, size_t nmemb, void *userdata);
  static size_t readCallback(char *dest, size_t size, size_t nmemb, void *userp);

private:
  CURLM *mCurlHandle_;
};

struct Error : virtual exception {
public:
  Error();
  Error(const string &msg);
  Error(const char* msg);
  Error(const json &errorInfo);

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Error, message, code, statusCode, data, name, description);
  const char *what() const noexcept override { return message.c_str(); }

private:
  string message;
  string code;
  long statusCode;
  json data;
  string name;
  string description;
};

struct UnretryableError : public Error {
public:
  UnretryableError(const Request &last_request,
                   const json &last_exception) {
    _last_request = last_request;
    _last_exception = last_exception.get<Error>();
  }
  Darabonba::Request getLastRequest();
  Error getLastException();

private:
  Request _last_request;
  Error _last_exception;
};

struct RetryableError : public Error {
public:
  RetryableError(const json &errorInfo): Error(errorInfo) {};
  static RetryableError New(string code, string message) {
    json errorInfo;
    errorInfo["code"] = code;
    errorInfo["message"] = message;
    return RetryableError(errorInfo);
  }
};

struct ValidateError : public Error {
public:
  ValidateError(const json &errorInfo): Error(errorInfo) {};
  static ValidateError New(string code, string message) {
    json errorInfo;
    errorInfo["code"] = code;
    errorInfo["message"] = message;
    return ValidateError(errorInfo);
  }
};
} // namespace Darabonba

#endif