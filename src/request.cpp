#include <darabonba/core.hpp>

Darabonba::Request::Request() {}
Darabonba::Request::~Request() {}


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

const std::string Darabonba::Request::url() const {
  std::string url = "";
  std::string host = _host;
  if (_headers.find("host") != _headers.end()) {
    auto hostIt = _headers.find("host");
    host = hostIt->second;
  }
  url += _protocol + "://" + host;
  if (!_pathname.empty()) {
    url +=  _pathname;
  }
  if (!_query.empty()) {
    if (url.find("?") != std::string::npos) {
      url += "&";
    } else {
      url += "?";
    }
    for (auto it=_query.begin(); it!=_query.end(); ++it) {
      std::string key = it->first;
      std::string val = it->second;
      if (val.empty()) {
        continue;
      }
      url += key + "=" + url_encode(val) + "&";
    }
    size_t strIndex = url.size();
    url.substr(0, strIndex - 1);
  }
  return url;
}