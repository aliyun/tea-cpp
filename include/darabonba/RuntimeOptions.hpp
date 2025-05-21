#ifndef DARABONBA_RUNTIME_OPTIONS_H_
#define DARABONBA_RUNTIME_OPTIONS_H_

#include <darabonba/Model.hpp>
#include <darabonba/Type.hpp>

namespace Darabonba {

class ExtendsParameters : public Model {
public:
    // 构造函数，支持默认参数初始化为nullptr
    ExtendsParameters(const std::map<std::string, std::string>& headers = {},
                      const std::map<std::string, std::string>& queries = {}) 
        : headers_(headers), queries_(queries) {}

    // 验证函数
    void validate() const {}

    // 将对象转换为map
    Darabonba::Json toMap() const override {
        auto result = Model::toMap();
        if (!headers_.empty()) {
            result["headers"] = mapToString(headers_);
        }
        if (!queries_.empty()) {
            result["queries"] = mapToString(queries_);
        }
        return result;
    }

    // 从map初始化对象
    void fromMap(const std::map<std::string, std::string>& m) {
        auto headersIt = m.find("headers");
        if (headersIt != m.end()) {
            headers_ = stringToMap(headersIt->second);
        }

        auto queriesIt = m.find("queries");
        if (queriesIt != m.end()) {
            queries_ = stringToMap(queriesIt->second);
        }
    }

private:
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> queries_;

    // 辅助函数：将map转换为字符串
    std::string mapToString(const std::map<std::string, std::string>& map) const {
        // std::ostringstream oss;
        // for (const auto& pair : map) {
        //     oss << pair.first << ":" << pair.second << ";";
        // }
        // return oss.str();
    }

    // 辅助函数：将字符串转换为map
    std::map<std::string, std::string> stringToMap(const std::string& str) const {
        // std::map<std::string, std::string> ret;
        // std::istringstream iss(str);
        // std::string item;
        // while (std::getline(iss, item, ';')) {
        //     auto pos = item.find(':');
        //     if (pos != std::string::npos) {
        //         ret[item.substr(0, pos)] = item.substr(pos + 1);
        //     }
        // }
        // return ret;
    }
};

class RuntimeOptions : public Model {
  friend void to_json(Json &j, const RuntimeOptions &obj) {
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
  }
  friend void from_json(const Json &j, RuntimeOptions &obj) {
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
  }

public:
  RuntimeOptions() = default;
  RuntimeOptions(const RuntimeOptions &) = default;
  RuntimeOptions(RuntimeOptions &&) = default;
  RuntimeOptions(const Darabonba::Json &obj) { from_json(obj, *this); }

  virtual ~RuntimeOptions() = default;

  virtual void validate() const override {}

  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  }

  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  }

  virtual bool empty() const override {
    return autoretry_ == nullptr && backoffPeriod_ == nullptr &&
           backoffPolicy_ == nullptr && ca_ == nullptr && cert_ == nullptr &&
           connectTimeout_ == nullptr && httpProxy_ == nullptr &&
           httpsProxy_ == nullptr && ignoreSSL_ == nullptr &&
           keepAlive_ == nullptr && key_ == nullptr && localAddr_ == nullptr &&
           maxAttempts_ == nullptr && maxIdleConns_ == nullptr &&
           noProxy_ == nullptr && readTimeout_ == nullptr &&
           socks5NetWork_ == nullptr && socks5Proxy_ == nullptr;
  }

  bool hasAutoretry() const { return this->autoretry_ != nullptr; }
  bool autoretry() const { DARABONBA_PTR_GET_DEFAULT(autoretry_, 0); }
  RuntimeOptions &setAutoretry(bool autoretry) {
    DARABONBA_PTR_SET_VALUE(autoretry_, autoretry);
  }

  bool hasBackoffPeriod() const { return this->backoffPeriod_ != nullptr; }
  int64_t backoffPeriod() const {
    DARABONBA_PTR_GET_DEFAULT(backoffPeriod_, 0);
  }
  RuntimeOptions &setBackoffPeriod(int64_t backoffPeriod) {
    DARABONBA_PTR_SET_VALUE(backoffPeriod_, backoffPeriod);
  }

  bool hasBackoffPolicy() const { return this->backoffPolicy_ != nullptr; }
  std::string backoffPolicy() const {
    DARABONBA_PTR_GET_DEFAULT(backoffPolicy_, "");
  }
  RuntimeOptions &setBackoffPolicy(const std::string &backoffPolicy) {
    DARABONBA_PTR_SET_VALUE(backoffPolicy_, backoffPolicy);
  }
  RuntimeOptions &setBackoffPolicy(std::string &&backoffPolicy) {
    DARABONBA_PTR_SET_RVALUE(backoffPolicy_, backoffPolicy);
  }

  bool hasCa() const { return this->ca_ != nullptr; }
  std::string ca() const { DARABONBA_PTR_GET_DEFAULT(ca_, ""); }
  RuntimeOptions &setCa(const std::string &ca) {
    DARABONBA_PTR_SET_VALUE(ca_, ca);
  }
  RuntimeOptions &setCa(std::string &&ca) { DARABONBA_PTR_SET_RVALUE(ca_, ca); }

  bool hasCert() const { return this->cert_ != nullptr; }
  std::string cert() const { DARABONBA_PTR_GET_DEFAULT(cert_, ""); }
  RuntimeOptions &setCert(const std::string &cert) {
    DARABONBA_PTR_SET_VALUE(cert_, cert);
  }
  RuntimeOptions &setCert(std::string &&cert) {
    DARABONBA_PTR_SET_RVALUE(cert_, cert);
  }

  bool hasConnectTimeout() const { return this->connectTimeout_ != nullptr; }
  int64_t connectTimeout() const {
    DARABONBA_PTR_GET_DEFAULT(connectTimeout_, 0);
  }
  RuntimeOptions &setConnectTimeout(int64_t connectTimeout) {
    DARABONBA_PTR_SET_VALUE(connectTimeout_, connectTimeout);
  }

  bool hasHttpProxy() const { return this->httpProxy_ != nullptr; }
  std::string httpProxy() const { DARABONBA_PTR_GET_DEFAULT(httpProxy_, ""); }
  RuntimeOptions &setHttpProxy(const std::string &httpProxy) {
    DARABONBA_PTR_SET_VALUE(httpProxy_, httpProxy);
  }
  RuntimeOptions &setHttpProxy(std::string &&httpProxy) {
    DARABONBA_PTR_SET_RVALUE(httpProxy_, httpProxy);
  }

  bool hasHttpsProxy() const { return this->httpsProxy_ != nullptr; }
  std::string httpsProxy() const { DARABONBA_PTR_GET_DEFAULT(httpsProxy_, ""); }
  RuntimeOptions &setHttpsProxy(const std::string &httpsProxy) {
    DARABONBA_PTR_SET_VALUE(httpsProxy_, httpsProxy);
  }
  RuntimeOptions &setHttpsProxy(std::string &&httpsProxy) {
    DARABONBA_PTR_SET_RVALUE(httpsProxy_, httpsProxy);
  }

  bool hasIgnoreSSL() const { return this->ignoreSSL_ != nullptr; }
  bool ignoreSSL() const { DARABONBA_PTR_GET_DEFAULT(ignoreSSL_, 0); }
  RuntimeOptions &setIgnoreSSL(bool ignoreSSL) {
    DARABONBA_PTR_SET_VALUE(ignoreSSL_, ignoreSSL);
  }

  bool hasKeepAlive() const { return this->keepAlive_ != nullptr; }
  bool keepAlive() const { DARABONBA_PTR_GET_DEFAULT(keepAlive_, 0); }
  RuntimeOptions &setKeepAlive(bool keepAlive) {
    DARABONBA_PTR_SET_VALUE(keepAlive_, keepAlive);
  }

  bool hasKey() const { return this->key_ != nullptr; }
  std::string key() const { DARABONBA_PTR_GET_DEFAULT(key_, ""); }
  RuntimeOptions &setKey(const std::string &key) {
    DARABONBA_PTR_SET_VALUE(key_, key);
  }
  RuntimeOptions &setKey(std::string &&key) {
    DARABONBA_PTR_SET_RVALUE(key_, key);
  }

  bool hasLocalAddr() const { return this->localAddr_ != nullptr; }
  std::string localAddr() const { DARABONBA_PTR_GET_DEFAULT(localAddr_, ""); }
  RuntimeOptions &setLocalAddr(const std::string &localAddr) {
    DARABONBA_PTR_SET_VALUE(localAddr_, localAddr);
  }
  RuntimeOptions &setLocalAddr(std::string &&localAddr) {
    DARABONBA_PTR_SET_RVALUE(localAddr_, localAddr);
  }

  bool hasMaxAttempts() const { return this->maxAttempts_ != nullptr; }
  int64_t maxAttempts() const { DARABONBA_PTR_GET_DEFAULT(maxAttempts_, 0); }
  RuntimeOptions &setMaxAttempts(int64_t maxAttempts) {
    DARABONBA_PTR_SET_VALUE(maxAttempts_, maxAttempts);
  }

  bool hasMaxIdleConns() const { return this->maxIdleConns_ != nullptr; }
  int64_t maxIdleConns() const { DARABONBA_PTR_GET_DEFAULT(maxIdleConns_, 0); }
  RuntimeOptions &setMaxIdleConns(int64_t maxIdleConns) {
    DARABONBA_PTR_SET_VALUE(maxIdleConns_, maxIdleConns);
  }

  bool hasNoProxy() const { return this->noProxy_ != nullptr; }
  std::string noProxy() const { DARABONBA_PTR_GET_DEFAULT(noProxy_, ""); }
  RuntimeOptions &setNoProxy(const std::string &noProxy) {
    DARABONBA_PTR_SET_VALUE(noProxy_, noProxy);
  }
  RuntimeOptions &setNoProxy(std::string &&noProxy) {
    DARABONBA_PTR_SET_RVALUE(noProxy_, noProxy);
  }

  bool hasReadTimeout() const { return this->readTimeout_ != nullptr; }
  int64_t readTimeout() const { DARABONBA_PTR_GET_DEFAULT(readTimeout_, 0); }
  RuntimeOptions &setReadTimeout(int64_t readTimeout) {
    DARABONBA_PTR_SET_VALUE(readTimeout_, readTimeout);
  }

  bool hasSocks5NetWork() const { return this->socks5NetWork_ != nullptr; }
  std::string socks5NetWork() const {
    DARABONBA_PTR_GET_DEFAULT(socks5NetWork_, "");
  }
  RuntimeOptions &setSocks5NetWork(const std::string &socks5NetWork) {
    DARABONBA_PTR_SET_VALUE(socks5NetWork_, socks5NetWork);
  }
  RuntimeOptions &setSocks5NetWork(std::string &&socks5NetWork) {
    DARABONBA_PTR_SET_RVALUE(socks5NetWork_, socks5NetWork);
  }

  bool hasSocks5Proxy() const { return this->socks5Proxy_ != nullptr; }
  std::string socks5Proxy() const {
    DARABONBA_PTR_GET_DEFAULT(socks5Proxy_, "");
  }
  RuntimeOptions &setSocks5Proxy(const std::string &socks5Proxy) {
    DARABONBA_PTR_SET_VALUE(socks5Proxy_, socks5Proxy);
  }
  RuntimeOptions &setSocks5Proxy(std::string &&socks5Proxy) {
    DARABONBA_PTR_SET_RVALUE(socks5Proxy_, socks5Proxy);
  }

protected:
  // whether to try again
  std::shared_ptr<bool> autoretry_ = nullptr;

  // ignore SSL validation
  std::shared_ptr<bool> ignoreSSL_ = nullptr;

  // privite key for client certificate
  std::shared_ptr<std::string> key_ = nullptr;

  // client certificate
  std::shared_ptr<std::string> cert_ = nullptr;

  // server certificate
  std::shared_ptr<std::string> ca_ = nullptr;

  // maximum number of retries
  std::shared_ptr<int64_t> maxAttempts_ = nullptr;

  // backoff policy
  std::shared_ptr<std::string> backoffPolicy_ = nullptr;

  // backoff period
  std::shared_ptr<int64_t> backoffPeriod_ = nullptr;

  // read timeout
  std::shared_ptr<int64_t> readTimeout_ = nullptr;

  // connect timeout
  std::shared_ptr<int64_t> connectTimeout_ = nullptr;

  // http proxy url
  std::shared_ptr<std::string> httpProxy_ = nullptr;

  // https Proxy url
  std::shared_ptr<std::string> httpsProxy_ = nullptr;

  // agent blacklist
  std::shared_ptr<std::string> noProxy_ = nullptr;

  // maximum number of connections
  std::shared_ptr<int64_t> maxIdleConns_ = nullptr;

  // local addr
  std::shared_ptr<std::string> localAddr_ = nullptr;

  // SOCKS5 proxy
  std::shared_ptr<std::string> socks5Proxy_ = nullptr;

  // SOCKS5 netWork
  std::shared_ptr<std::string> socks5NetWork_ = nullptr;

  // whether to enable keep-alive
  std::shared_ptr<bool> keepAlive_ = nullptr;
};

} // namespace Darabonba

#endif