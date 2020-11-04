#include <darabonba/core.hpp>

template <typename T> bool can_cast(const boost::any &v) {
  return typeid(T) == v.type();
}

void json_encode(boost::any val, std::stringstream &ss) {
  if (can_cast<map<string, boost::any>>(val)) {
    map<string, boost::any> m = boost::any_cast<map<string, boost::any>>(val);
    ss << '{';
    if (!m.empty()) {
      int n = 0;
      for (const auto &it : m) {
        if (n != 0) {
          ss << ",";
        }
        ss << '"' << it.first << '"' << ':';
        json_encode(it.second, ss);
        n++;
      }
    }
    ss << '}';
  } else if (can_cast<vector<boost::any>>(val)) {
    vector<boost::any> v = boost::any_cast<vector<boost::any>>(val);
    ss << '[';
    if (!v.empty()) {
      int n = 0;
      for (const auto &it : v) {
        if (n != 0) {
          ss << ",";
        }
        json_encode(it, ss);
        n++;
      }
    }
    ss << ']';
  } else if (can_cast<int>(val)) {
    int i = boost::any_cast<int>(val);
    ss << std::to_string(i);
  } else if (can_cast<long>(val)) {
    long l = boost::any_cast<long>(val);
    ss << std::to_string(l);
  } else if (can_cast<double>(val)) {
    auto d = boost::any_cast<double>(val);
    ss << std::to_string(d);
  } else if (can_cast<float>(val)) {
    auto f = boost::any_cast<float>(val);
    ss << std::to_string(f);
  } else if (can_cast<string>(val)) {
    auto s = boost::any_cast<string>(val);
    ss << '"' << s << '"';
  } else if (can_cast<bool>(val)) {
    auto b = boost::any_cast<bool>(val);
    string c = b ? "true" : "false";
    ss << c;
  } else if (can_cast<const char *>(val)) {
    auto s = boost::any_cast<const char *>(val);
    ss << '"' << s << '"';
  } else if (can_cast<char *>(val)) {
    auto s = boost::any_cast<char *>(val);
    ss << '"' << s << '"';
  }
}

Darabonba::Error::Error() = default;
Darabonba::Error::Error(const boost::any &error_info) {
  if (typeid(map<string, string>) == error_info.type()) {
    resolve(boost::any_cast<map<string, string>>(error_info));
  } else if (typeid(shared_ptr<map<string, string>>) == error_info.type()) {
    shared_ptr<map<string, string>> mss =
        boost::any_cast<shared_ptr<map<string, string>>>(error_info);
    resolve(*mss);
  } else if (typeid(map<string, boost::any>) == error_info.type()) {
    resolve(boost::any_cast<map<string, boost::any>>(error_info));
  } else if (typeid(shared_ptr<map<string, boost::any>>) == error_info.type()) {
    shared_ptr<map<string, boost::any>> msa =
        boost::any_cast<shared_ptr<map<string, boost::any>>>(error_info);
    resolve(*msa);
  }
}
void Darabonba::Error::resolve(map<string, string> error_info) {
  if (error_info.find("message") != error_info.end()) {
    message = error_info.at("message");
  }
  if (error_info.find("code") != error_info.end()) {
    code = error_info.at("code");
  }
  if (error_info.find("data") != error_info.end()) {
    data = error_info.at("data");
  }
  if (error_info.find("name") != error_info.end()) {
    name = error_info.at("name");
  }
}

void Darabonba::Error::resolve(map<string, boost::any> error_info) {
  if (error_info.find("message") != error_info.end()) {
    message = boost::any_cast<string>(error_info.at("message"));
  }
  if (error_info.find("code") != error_info.end()) {
    code = boost::any_cast<string>(error_info.at("code"));
  }
  if (error_info.find("data") != error_info.end()) {
    if (typeid(string) == error_info.at("data").type()) {
      data = boost::any_cast<string>(error_info.at("data"));
    } else if (typeid(map<string, boost::any>) ==
               error_info.at("data").type()) {
      map<string, boost::any> m =
          boost::any_cast<map<string, boost::any>>(error_info.at("data"));
      std::stringstream s;
      json_encode(m, s);
      data = s.str();
    }
  }
  if (error_info.find("name") != error_info.end()) {
    name = boost::any_cast<string>(error_info.at("name"));
  }
}

Darabonba::Error Darabonba::UnretryableError::getLastException() {
  return _last_exception;
}

Darabonba::Request Darabonba::UnretryableError::getLastRequest() {
  return _last_request;
}
