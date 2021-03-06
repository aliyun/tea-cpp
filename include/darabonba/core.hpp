#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <cpprest/http_client.h>
#include <cpprest/streams.h>
#include <iostream>
#include <utility>

using namespace std;

namespace Darabonba {
class Model {
public:
  Model() = default;
  ~Model() = default;
  Model(const map<string, boost::any> &config) { _config = config; };
  static void validateRequired(const string &field_name,
                               const shared_ptr<boost::any> &field);
  static void validateMaxLength(const string &field_name,
                                const shared_ptr<string> &field, int val);
  static void validateMinLength(const string &field_name,
                                const shared_ptr<string> &field, int val);
  static void validateMaximum(const string &field_name,
                              shared_ptr<boost::any> field, boost::any val);
  static void validateMinimum(const string &field_name,
                              shared_ptr<boost::any> field, boost::any val);
  static void validatePattern(const string &field_name,
                              const shared_ptr<string> &field,
                              const string &val);
  virtual void validate() = 0;
  virtual map<string, boost::any> toMap() = 0;
  virtual void fromMap(map<string, boost::any> m) = 0;

  void call_validate() { validate(); };
  map<string, boost::any> call_toMap() { return toMap(); };
  void call_fromMap(map<string, boost::any> m) { fromMap(m); };

protected:
  map<string, string> _name;
  map<string, boost::any> _default;
  map<string, boost::any> _config;
};

class Stream {
public:
  Stream() = default;
  virtual ~Stream() = default;
  explicit Stream(const shared_ptr<fstream> &stream) { f_stream = stream; }
  explicit Stream(const shared_ptr<stringstream> &stream) {
    string_stream = stream;
  }
  explicit Stream(const shared_ptr<concurrency::streams::istream> &stream) {
    rest_stream = stream;
  }

  explicit Stream(concurrency::streams::istream stream) {
    rest_stream = make_shared<concurrency::streams::istream>(stream);
  }

  virtual bool empty() {
    if (f_stream || string_stream || rest_stream) {
      return false;
    }
    return true;
  }

  virtual string read() {
    string resp;
    if (f_stream) {
      f_stream->seekg(0, ios::end);
      streamsize size = f_stream->tellg();
      f_stream->seekg(0, ios::beg);
      char *buf = new char[size];

      f_stream->read(buf, size);
      resp = string(buf, size);
      delete[] buf;
    } else if (string_stream) {
      string_stream->seekg(0, ios::end);
      streamsize size = string_stream->tellg();
      string_stream->seekg(0, ios::beg);
      char *buf = new char[size];

      string_stream->read(buf, size);
      resp = string(buf, size);
      delete[] buf;
    } else if (rest_stream) {
      concurrency::streams::stringstreambuf stream_buf;
      rest_stream->read_to_end(stream_buf).get();
      resp = stream_buf.collection();
    }
    return resp;
  };
private:
  shared_ptr<fstream> f_stream;
  shared_ptr<stringstream> string_stream;
  shared_ptr<concurrency::streams::istream> rest_stream;
};

class Request {
public:
  Request();
  ~Request();

  string protocol = "https";
  string host;
  int port = 80;
  string method = "GET";
  string pathname;
  map<string, string> query;
  shared_ptr<Stream> body;
  map<string, string> headers;
};
class Response {
public:
  Response();
  ~Response();

  shared_ptr<Stream> body;
  int statusCode;
  string statusMessage;
  map<string, string> headers;
};
class Core {
public:
  static Response doAction(const boost::any &request,
                           const boost::any &runtime);
  static Response doAction(const boost::any &request);
  static bool allowRetry(const shared_ptr<boost::any> &retry,
                         const shared_ptr<int> &retry_times,
                         const shared_ptr<int> &now);
  static int getBackoffTime(const shared_ptr<boost::any> &backoff,
                            const shared_ptr<int> &retry_times);
  static void sleep(const shared_ptr<int> &sleep_time);
  static bool isRetryable(const exception &ex);
  static bool isRetryable(const boost::exception &ex);
  static web::http::client::http_client_config
  httpConfig(const Darabonba::Request &req, map<string, boost::any> runtime);
};
class Converter {
public:
  static map<string, boost::any> toGenericMap(const map<string, string> &m) {
    map<string, boost::any> data;
    for (const auto &it : m) {
      data[it.first] = boost::any(it.second);
    }
    return data;
  }

  static shared_ptr<Stream> toStream(const vector<uint8_t> &val){
    string str(val.begin(), val.end());
    return make_shared<Stream>(make_shared<stringstream>(str));
  }

  static shared_ptr<Stream> toStream(const string &str) {
    return make_shared<Stream>(make_shared<stringstream>(str));
  }

  static shared_ptr<map<string, string>> mapPointer(map<string, string> m) {
    return make_shared<map<string, string>>(m);
  }

  static shared_ptr<map<string, boost::any>>
  mapPointer(map<string, boost::any> m) {
    return make_shared<map<string, boost::any>>(m);
  }

  static string toString(boost::any val) {
    if (typeid(string) == val.type()) {
      return boost::any_cast<string>(val);
    }

    if (typeid(shared_ptr<boost::any>) == val.type()) {
      shared_ptr<boost::any> val_ptr = boost::any_cast<shared_ptr<boost::any>>(val);
      if (val_ptr) {
        return boost::any_cast<string>(*val_ptr);
      }
    }

    return string("");
  }

  template <typename... Params>
  static map<string, string> merge(map<string, string> m,
                                   Params... parameters) {
    mergeMap(m, parameters...);
    return m;
  }

  template <typename... Params>
  static map<string, boost::any> merge(map<string, boost::any> m,
                                       Params... parameters) {
    mergeMap(m, parameters...);
    return m;
  }

private:
  template <typename... Params>
  static void mergeMap(map<string, string> &m, map<string, string> value,
                       Params... parameters) {
    assignMap(m, value);
    mergeMap(m, parameters...);
  }
  static void mergeMap(map<string, string> &m, map<string, string> value) {
    assignMap(m, value);
  }

  static void assignMap(map<string, string> &m, map<string, string> value) {
    for (auto it : value) {
      m.insert(pair<string, string>(it.first, it.second));
    }
  }

  template <typename... Params>
  static void mergeMap(map<string, boost::any> &m,
                       map<string, boost::any> value, Params... parameters) {
    assignMap(m, value);
    mergeMap(m, parameters...);
  }
  static void mergeMap(map<string, boost::any> &m,
                       map<string, boost::any> value) {
    assignMap(m, value);
  }

  static void assignMap(map<string, boost::any> &m,
                        map<string, boost::any> value) {
    for (auto it : value) {
      m.insert(pair<string, boost::any>(it.first, it.second));
    }
  }
};
struct Error : virtual exception, virtual boost::exception {
public:
  Error();
  Error(const boost::any &error_info);
  string message;
  string code;
  string data;
  string name;
  const char *what() const noexcept override { return message.c_str(); }

private:
  void resolve(map<string, string> error_info);
  void resolve(map<string, boost::any> error_info);
};
struct UnretryableError : public Error {
public:
  UnretryableError(const boost::any &last_request,
                   const boost::any &last_exception) {
    if (typeid(Request) == last_request.type()) {
      _last_request = boost::any_cast<Request>(last_request);
    } else if (typeid(shared_ptr<Request>) == last_request.type()) {
      shared_ptr<Request> req =
          boost::any_cast<shared_ptr<Request>>(last_request);
      _last_request = *req;
    }

    if (typeid(std::exception) == last_exception.type()) {
      std::exception e = boost::any_cast<std::exception>(last_exception);
      Error error;
      error.message = e.what();
      _last_exception = error;
    } else if (typeid(Error) == last_exception.type()) {
      Error e = boost::any_cast<Error>(last_exception);
      _last_exception = e;
    } else if (typeid(boost::exception) == last_exception.type()) {
      Error error;
      error.message = boost::diagnostic_information(last_exception);
      _last_exception = error;
    }
  }
  Darabonba::Request getLastRequest();
  Error getLastException();

private:
  Request _last_request;
  Error _last_exception;
};
} // namespace Darabonba

#endif