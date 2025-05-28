#ifndef DARABONBA_RUNTIME_OPTIONS_H_
#define DARABONBA_RUNTIME_OPTIONS_H_

#include <darabonba/Model.hpp>
#include <darabonba/Type.hpp>
#include <darabonba/policy/Retry.hpp>

using namespace std;
using namespace Darabonba::Policy;
namespace Darabonba {

class ExtendsParameters : public Darabonba::Model {
public:
  friend void to_json(Darabonba::Json& j, const ExtendsParameters& obj) {
    DARABONBA_PTR_TO_JSON(headers, headers_);
    DARABONBA_PTR_TO_JSON(queries, queries_);
  };
  friend void from_json(const Darabonba::Json& j, ExtendsParameters& obj) {
    DARABONBA_PTR_FROM_JSON(headers, headers_);
    DARABONBA_PTR_FROM_JSON(queries, queries_);
  };
  ExtendsParameters() = default ;
  ExtendsParameters(const ExtendsParameters &) = default ;
  ExtendsParameters(ExtendsParameters &&) = default ;
  ExtendsParameters(const Darabonba::Json & obj) { from_json(obj, *this); };
  virtual ~ExtendsParameters() = default ;
  ExtendsParameters& operator=(const ExtendsParameters &) = default ;
  ExtendsParameters& operator=(ExtendsParameters &&) = default ;
  virtual void validate() const override {
  };
  virtual void fromMap(const Darabonba::Json &obj) override { from_json(obj, *this); validate(); };
  virtual Darabonba::Json toMap() const override { Darabonba::Json obj; to_json(obj, *this); return obj; };
  virtual bool empty() const override { this->headers_ != nullptr
                                        && this->queries_ != nullptr; };
  bool hasHeaders() { return this->headers_ != nullptr;};
  const map<string, string> & headers() const { DARABONBA_PTR_GET(headers_) };
  map<string, string> & headers() { DARABONBA_PTR_GET(headers_) };
  ExtendsParameters& setHeaders(const map<string, string> & headers) { DARABONBA_PTR_SET_VALUE(headers_, headers) };
  ExtendsParameters& setHeaders(map<string, string> && headers) { DARABONBA_PTR_SET_RVALUE(headers_, headers) };
  bool hasQueries() { return this->queries_ != nullptr;};
  const map<string, string> & queries() const { DARABONBA_PTR_GET(queries_) };
  map<string, string> & queries() { DARABONBA_PTR_GET(queries_) };
  ExtendsParameters& setQueries(const map<string, string> & queries) { DARABONBA_PTR_SET_VALUE(queries_, queries) };
  ExtendsParameters& setQueries(map<string, string> && queries) { DARABONBA_PTR_SET_RVALUE(queries_, queries) };
protected:
  std::shared_ptr<map<string, string>> headers_ = nullptr;
  std::shared_ptr<map<string, string>> queries_ = nullptr;
};

class RuntimeOptions : public Darabonba::Model {
public:
  friend void to_json(Darabonba::Json& j, const RuntimeOptions& obj) {
    DARABONBA_PTR_TO_JSON(autoretry, autoretry_);
    DARABONBA_PTR_TO_JSON(ignoreSSL, ignoreSSL_);
    DARABONBA_PTR_TO_JSON(key, key_);
    DARABONBA_PTR_TO_JSON(cert, cert_);
    DARABONBA_PTR_TO_JSON(ca, ca_);
    DARABONBA_PTR_TO_JSON(max_attempts, maxAttempts_);
    DARABONBA_PTR_TO_JSON(backoff_policy, backoffPolicy_);
    DARABONBA_PTR_TO_JSON(backoff_period, backoffPeriod_);
    DARABONBA_PTR_TO_JSON(readTimeout, readTimeout_);
    DARABONBA_PTR_TO_JSON(connectTimeout, connectTimeout_);
    DARABONBA_PTR_TO_JSON(httpProxy, httpProxy_);
    DARABONBA_PTR_TO_JSON(httpsProxy, httpsProxy_);
    DARABONBA_PTR_TO_JSON(noProxy, noProxy_);
    DARABONBA_PTR_TO_JSON(maxIdleConns, maxIdleConns_);
    DARABONBA_PTR_TO_JSON(localAddr, localAddr_);
    DARABONBA_PTR_TO_JSON(socks5Proxy, socks5Proxy_);
    DARABONBA_PTR_TO_JSON(socks5NetWork, socks5NetWork_);
    DARABONBA_PTR_TO_JSON(keepAlive, keepAlive_);
    DARABONBA_PTR_TO_JSON(extendsParameters, extendsParameters_);
    DARABONBA_PTR_TO_JSON(retryOptions, retryOptions_);
  };
  friend void from_json(const Darabonba::Json& j, RuntimeOptions& obj) {
    DARABONBA_PTR_FROM_JSON(autoretry, autoretry_);
    DARABONBA_PTR_FROM_JSON(ignoreSSL, ignoreSSL_);
    DARABONBA_PTR_FROM_JSON(key, key_);
    DARABONBA_PTR_FROM_JSON(cert, cert_);
    DARABONBA_PTR_FROM_JSON(ca, ca_);
    DARABONBA_PTR_FROM_JSON(max_attempts, maxAttempts_);
    DARABONBA_PTR_FROM_JSON(backoff_policy, backoffPolicy_);
    DARABONBA_PTR_FROM_JSON(backoff_period, backoffPeriod_);
    DARABONBA_PTR_FROM_JSON(readTimeout, readTimeout_);
    DARABONBA_PTR_FROM_JSON(connectTimeout, connectTimeout_);
    DARABONBA_PTR_FROM_JSON(httpProxy, httpProxy_);
    DARABONBA_PTR_FROM_JSON(httpsProxy, httpsProxy_);
    DARABONBA_PTR_FROM_JSON(noProxy, noProxy_);
    DARABONBA_PTR_FROM_JSON(maxIdleConns, maxIdleConns_);
    DARABONBA_PTR_FROM_JSON(localAddr, localAddr_);
    DARABONBA_PTR_FROM_JSON(socks5Proxy, socks5Proxy_);
    DARABONBA_PTR_FROM_JSON(socks5NetWork, socks5NetWork_);
    DARABONBA_PTR_FROM_JSON(keepAlive, keepAlive_);
    DARABONBA_PTR_FROM_JSON(extendsParameters, extendsParameters_);
    DARABONBA_PTR_FROM_JSON(retryOptions, retryOptions_);
  };
  RuntimeOptions() = default ;
  RuntimeOptions(const RuntimeOptions &) = default ;
  RuntimeOptions(RuntimeOptions &&) = default ;
  RuntimeOptions(const Darabonba::Json & obj) { from_json(obj, *this); };
  virtual ~RuntimeOptions() = default ;
  RuntimeOptions& operator=(const RuntimeOptions &) = default ;
  RuntimeOptions& operator=(RuntimeOptions &&) = default ;
  virtual void validate() const override {
  };
  virtual void fromMap(const Darabonba::Json &obj) override { from_json(obj, *this); validate(); };
  virtual Darabonba::Json toMap() const override { Darabonba::Json obj; to_json(obj, *this); return obj; };
  virtual bool empty() const override { this->autoretry_ != nullptr
                                        && this->ignoreSSL_ != nullptr && this->key_ != nullptr && this->cert_ != nullptr && this->ca_ != nullptr && this->maxAttempts_ != nullptr
                                        && this->backoffPolicy_ != nullptr && this->backoffPeriod_ != nullptr && this->readTimeout_ != nullptr && this->connectTimeout_ != nullptr && this->httpProxy_ != nullptr
                                        && this->httpsProxy_ != nullptr && this->noProxy_ != nullptr && this->maxIdleConns_ != nullptr && this->localAddr_ != nullptr && this->socks5Proxy_ != nullptr
                                        && this->socks5NetWork_ != nullptr && this->keepAlive_ != nullptr && this->extendsParameters_ != nullptr && this->retryOptions_ != nullptr; };
  bool hasAutoretry() { return this->autoretry_ != nullptr;};
  bool autoretry() const { DARABONBA_PTR_GET_DEFAULT(autoretry_, false) };
  RuntimeOptions& setAutoretry(bool autoretry) { DARABONBA_PTR_SET_VALUE(autoretry_, autoretry) };
  bool hasIgnoreSSL() { return this->ignoreSSL_ != nullptr;};
  bool ignoreSSL() const { DARABONBA_PTR_GET_DEFAULT(ignoreSSL_, false) };
  RuntimeOptions& setIgnoreSSL(bool ignoreSSL) { DARABONBA_PTR_SET_VALUE(ignoreSSL_, ignoreSSL) };
  bool hasKey() { return this->key_ != nullptr;};
  string key() const { DARABONBA_PTR_GET_DEFAULT(key_, "") };
  RuntimeOptions& setKey(string key) { DARABONBA_PTR_SET_VALUE(key_, key) };
  bool hasCert() { return this->cert_ != nullptr;};
  string cert() const { DARABONBA_PTR_GET_DEFAULT(cert_, "") };
  RuntimeOptions& setCert(string cert) { DARABONBA_PTR_SET_VALUE(cert_, cert) };
  bool hasCa() { return this->ca_ != nullptr;};
  string ca() const { DARABONBA_PTR_GET_DEFAULT(ca_, "") };
  RuntimeOptions& setCa(string ca) { DARABONBA_PTR_SET_VALUE(ca_, ca) };
  bool hasMaxAttempts() { return this->maxAttempts_ != nullptr;};
  int64_t maxAttempts() const { DARABONBA_PTR_GET_DEFAULT(maxAttempts_, 0) };
  RuntimeOptions& setMaxAttempts(int64_t maxAttempts) { DARABONBA_PTR_SET_VALUE(maxAttempts_, maxAttempts) };
  bool hasBackoffPolicy() { return this->backoffPolicy_ != nullptr;};
  string backoffPolicy() const { DARABONBA_PTR_GET_DEFAULT(backoffPolicy_, "") };
  RuntimeOptions& setBackoffPolicy(string backoffPolicy) { DARABONBA_PTR_SET_VALUE(backoffPolicy_, backoffPolicy) };
  bool hasBackoffPeriod() { return this->backoffPeriod_ != nullptr;};
  int64_t backoffPeriod() const { DARABONBA_PTR_GET_DEFAULT(backoffPeriod_, 0) };
  RuntimeOptions& setBackoffPeriod(int64_t backoffPeriod) { DARABONBA_PTR_SET_VALUE(backoffPeriod_, backoffPeriod) };
  bool hasReadTimeout() { return this->readTimeout_ != nullptr;};
  int64_t readTimeout() const { DARABONBA_PTR_GET_DEFAULT(readTimeout_, 0) };
  RuntimeOptions& setReadTimeout(int64_t readTimeout) { DARABONBA_PTR_SET_VALUE(readTimeout_, readTimeout) };
  bool hasConnectTimeout() { return this->connectTimeout_ != nullptr;};
  int64_t connectTimeout() const { DARABONBA_PTR_GET_DEFAULT(connectTimeout_, 0) };
  RuntimeOptions& setConnectTimeout(int64_t connectTimeout) { DARABONBA_PTR_SET_VALUE(connectTimeout_, connectTimeout) };
  bool hasHttpProxy() { return this->httpProxy_ != nullptr;};
  string httpProxy() const { DARABONBA_PTR_GET_DEFAULT(httpProxy_, "") };
  RuntimeOptions& setHttpProxy(string httpProxy) { DARABONBA_PTR_SET_VALUE(httpProxy_, httpProxy) };
  bool hasHttpsProxy() { return this->httpsProxy_ != nullptr;};
  string httpsProxy() const { DARABONBA_PTR_GET_DEFAULT(httpsProxy_, "") };
  RuntimeOptions& setHttpsProxy(string httpsProxy) { DARABONBA_PTR_SET_VALUE(httpsProxy_, httpsProxy) };
  bool hasNoProxy() { return this->noProxy_ != nullptr;};
  string noProxy() const { DARABONBA_PTR_GET_DEFAULT(noProxy_, "") };
  RuntimeOptions& setNoProxy(string noProxy) { DARABONBA_PTR_SET_VALUE(noProxy_, noProxy) };
  bool hasMaxIdleConns() { return this->maxIdleConns_ != nullptr;};
  int64_t maxIdleConns() const { DARABONBA_PTR_GET_DEFAULT(maxIdleConns_, 0) };
  RuntimeOptions& setMaxIdleConns(int64_t maxIdleConns) { DARABONBA_PTR_SET_VALUE(maxIdleConns_, maxIdleConns) };
  bool hasLocalAddr() { return this->localAddr_ != nullptr;};
  string localAddr() const { DARABONBA_PTR_GET_DEFAULT(localAddr_, "") };
  RuntimeOptions& setLocalAddr(string localAddr) { DARABONBA_PTR_SET_VALUE(localAddr_, localAddr) };
  bool hasSocks5Proxy() { return this->socks5Proxy_ != nullptr;};
  string socks5Proxy() const { DARABONBA_PTR_GET_DEFAULT(socks5Proxy_, "") };
  RuntimeOptions& setSocks5Proxy(string socks5Proxy) { DARABONBA_PTR_SET_VALUE(socks5Proxy_, socks5Proxy) };
  bool hasSocks5NetWork() { return this->socks5NetWork_ != nullptr;};
  string socks5NetWork() const { DARABONBA_PTR_GET_DEFAULT(socks5NetWork_, "") };
  RuntimeOptions& setSocks5NetWork(string socks5NetWork) { DARABONBA_PTR_SET_VALUE(socks5NetWork_, socks5NetWork) };
  bool hasKeepAlive() { return this->keepAlive_ != nullptr;};
  bool keepAlive() const { DARABONBA_PTR_GET_DEFAULT(keepAlive_, false) };
  RuntimeOptions& setKeepAlive(bool keepAlive) { DARABONBA_PTR_SET_VALUE(keepAlive_, keepAlive) };
  bool hasExtendsParameters() { return this->extendsParameters_ != nullptr;};
  const ExtendsParameters & extendsParameters() const { DARABONBA_PTR_GET(extendsParameters_) };
  ExtendsParameters & extendsParameters() { DARABONBA_PTR_GET(extendsParameters_) };
  RuntimeOptions& setExtendsParameters(const ExtendsParameters & extendsParameters) { DARABONBA_PTR_SET_VALUE(extendsParameters_, extendsParameters) };
  RuntimeOptions& setExtendsParameters(ExtendsParameters && extendsParameters) { DARABONBA_PTR_SET_RVALUE(extendsParameters_, extendsParameters) };
  bool hasRetryOptions() { return this->retryOptions_ != nullptr;};
  const RetryOptions & retryOptions() const { DARABONBA_PTR_GET(retryOptions_) };
  RetryOptions & retryOptions() { DARABONBA_PTR_GET(retryOptions_) };
  RuntimeOptions& setRetryOptions(const RetryOptions & retryOptions) { DARABONBA_PTR_SET_VALUE(retryOptions_, retryOptions) };
  RuntimeOptions& setRetryOptions(RetryOptions && retryOptions) { DARABONBA_PTR_SET_RVALUE(retryOptions_, retryOptions) };
protected:
  // whether to try again
  std::shared_ptr<bool> autoretry_ = nullptr;
  // ignore SSL validation
  std::shared_ptr<bool> ignoreSSL_ = nullptr;
  // privite key for client certificate
  std::shared_ptr<string> key_ = nullptr;
  // client certificate
  std::shared_ptr<string> cert_ = nullptr;
  // server certificate
  std::shared_ptr<string> ca_ = nullptr;
  // maximum number of retries
  std::shared_ptr<int64_t> maxAttempts_ = nullptr;
  // backoff policy
  std::shared_ptr<string> backoffPolicy_ = nullptr;
  // backoff period
  std::shared_ptr<int64_t> backoffPeriod_ = nullptr;
  // read timeout
  std::shared_ptr<int64_t> readTimeout_ = nullptr;
  // connect timeout
  std::shared_ptr<int64_t> connectTimeout_ = nullptr;
  // http proxy url
  std::shared_ptr<string> httpProxy_ = nullptr;
  // https Proxy url
  std::shared_ptr<string> httpsProxy_ = nullptr;
  // agent blacklist
  std::shared_ptr<string> noProxy_ = nullptr;
  // maximum number of connections
  std::shared_ptr<int64_t> maxIdleConns_ = nullptr;
  // local addr
  std::shared_ptr<string> localAddr_ = nullptr;
  // SOCKS5 proxy
  std::shared_ptr<string> socks5Proxy_ = nullptr;
  // SOCKS5 netWork
  std::shared_ptr<string> socks5NetWork_ = nullptr;
  // whether to enable keep-alive
  std::shared_ptr<bool> keepAlive_ = nullptr;
  // retry options
  std::shared_ptr<RetryOptions> retryOptions_ = nullptr;
  // Extends Parameters
  std::shared_ptr<ExtendsParameters> extendsParameters_ = nullptr;
};

} // namespace Darabonba

#endif