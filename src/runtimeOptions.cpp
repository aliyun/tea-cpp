//
// Created by page on 2023/1/26.
//
#include <darabonba/core.hpp>

using namespace std;
using json = nlohmann::json;
using namespace Darabonba;

RuntimeOptions::RuntimeOptions(const json &map) : Darabonba::Model() {
  validate(map);
  from_json(map, *this);
}

void RuntimeOptions::validate(const json &map) {
}

json RuntimeOptions::toMap() {
  json map;
  to_json(map, *this);
  return map;
}

bool RuntimeOptions::autoretry() const {
  if(_autoretry) {
    return *_autoretry;
  }
  return false;
}

void RuntimeOptions::setAutoretry(bool autoretry){
  _autoretry = make_shared<bool> (autoretry);
}

bool RuntimeOptions::ignoreSSL() const {
  if(_ignoreSSL) {
    return *_ignoreSSL;
  }
  return false;
}

void RuntimeOptions::setIgnoreSSL(bool ignoreSSL){
  _ignoreSSL = make_shared<bool> (ignoreSSL);
}

string RuntimeOptions::key() const {
  if(_key) {
    return *_key;
  }
  return "";
}

void RuntimeOptions::setKey(string key){
  _key = make_shared<string> (key);
}

string RuntimeOptions::cert() const {
  if(_cert) {
    return *_cert;
  }
  return "";
}

void RuntimeOptions::setCert(string cert){
  _cert = make_shared<string> (cert);
}

string RuntimeOptions::ca() const {
  if(_ca) {
    return *_ca;
  }
  return "";
}

void RuntimeOptions::setCa(string ca){
  _ca = make_shared<string> (ca);
}

int64_t RuntimeOptions::maxAttempts() const {
  if(_maxAttempts) {
    return *_maxAttempts;
  }
  return 0;
}

void RuntimeOptions::setMaxAttempts(int64_t maxAttempts){
  _maxAttempts = make_shared<int64_t> (maxAttempts);
}

string RuntimeOptions::backoffPolicy() const {
  if(_backoffPolicy) {
    return *_backoffPolicy;
  }
  return "";
}

void RuntimeOptions::setBackoffPolicy(string backoffPolicy){
  _backoffPolicy = make_shared<string> (backoffPolicy);
}

int64_t RuntimeOptions::backoffPeriod() const {
  if(_backoffPeriod) {
    return *_backoffPeriod;
  }
  return 0;
}

void RuntimeOptions::setBackoffPeriod(int64_t backoffPeriod){
  _backoffPeriod = make_shared<int64_t> (backoffPeriod);
}

int64_t RuntimeOptions::readTimeout() const {
  if(_readTimeout) {
    return *_readTimeout;
  }
  return 0;
}

void RuntimeOptions::setReadTimeout(int64_t readTimeout){
  _readTimeout = make_shared<int64_t> (readTimeout);
}

int64_t RuntimeOptions::connectTimeout() const {
  if(_connectTimeout) {
    return *_connectTimeout;
  }
  return 0;
}

void RuntimeOptions::setConnectTimeout(int64_t connectTimeout){
  _connectTimeout = make_shared<int64_t> (connectTimeout);
}

string RuntimeOptions::httpProxy() const {
  if(_httpProxy) {
    return *_httpProxy;
  }
  return "";
}

void RuntimeOptions::setHttpProxy(string httpProxy){
  _httpProxy = make_shared<string> (httpProxy);
}

string RuntimeOptions::httpsProxy() const {
  if(_httpsProxy) {
    return *_httpsProxy;
  }
  return "";
}

void RuntimeOptions::setHttpsProxy(string httpsProxy){
  _httpsProxy = make_shared<string> (httpsProxy);
}

string RuntimeOptions::noProxy() const {
  if(_noProxy) {
    return *_noProxy;
  }
  return "";
}

void RuntimeOptions::setNoProxy(string noProxy){
  _noProxy = make_shared<string> (noProxy);
}

int64_t RuntimeOptions::maxIdleConns() const {
  if(_maxIdleConns) {
    return *_maxIdleConns;
  }
  return 0;
}

void RuntimeOptions::setMaxIdleConns(int64_t maxIdleConns){
  _maxIdleConns = make_shared<int64_t> (maxIdleConns);
}

string RuntimeOptions::localAddr() const {
  if(_localAddr) {
    return *_localAddr;
  }
  return "";
}

void RuntimeOptions::setLocalAddr(string localAddr){
  _localAddr = make_shared<string> (localAddr);
}

string RuntimeOptions::socks5Proxy() const {
  if(_socks5Proxy) {
    return *_socks5Proxy;
  }
  return "";
}

void RuntimeOptions::setSocks5Proxy(string socks5Proxy){
  _socks5Proxy = make_shared<string> (socks5Proxy);
}

string RuntimeOptions::socks5NetWork() const {
  if(_socks5NetWork) {
    return *_socks5NetWork;
  }
  return "";
}

void RuntimeOptions::setSocks5NetWork(string socks5NetWork){
  _socks5NetWork = make_shared<string> (socks5NetWork);
}

bool RuntimeOptions::keepAlive() const {
  if(_keepAlive) {
    return *_keepAlive;
  }
  return false;
}

void RuntimeOptions::setKeepAlive(bool keepAlive){
  _keepAlive = make_shared<bool> (keepAlive);
}



