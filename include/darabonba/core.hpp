#ifndef DARABONBA_CORE_H_
#define DARABONBA_CORE_H_

#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <cpprest/streams.h>
#include <iostream>

using namespace std;

namespace Darabonba {
class Model {
public:
  Model();
  ~Model();
  Model(const std::map<string, boost::any> &config);
  static void validateRequired(const string &field_name, boost::any *field,
                               bool val = false);
  static void validateMaxLength(const string &field_name, string *field,
                                int val);
  static void validateMinLength(const string &field_name, string *field,
                                int val);
  static void validateMaximum(const string &field_name, boost::any *field,
                              boost::any val);
  static void validateMinimum(const string &field_name, boost::any *field,
                              boost::any val);
  static void validatePattern(const string &field_name, string *field,
                              const string &val);
  virtual void validate(){};

  virtual map<string, boost::any> toMap() = 0;
  virtual void fromMap(map<string, boost::any> m) = 0;

protected:
  map<string, string> _name;
  map<string, boost::any> _config;
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
  string body;
  map<string, string> headers;
};
class Response {
public:
  Response();
  ~Response();

  concurrency::streams::ostream body;
  int statusCode;
};
class Core {
public:
  static Response doRequest(const Request &req,
                            map<string, boost::any> runtime);
  static Response doRequest(const Request &req);
  static bool allowRetry(boost::any *retry, int *retry_times, int *now);
  static int getBackoffTime(boost::any *backoff, int *retry_times);
  static void sleep(int *sleep_time);
  static bool isRetryable(std::exception *ex);
  static bool isRetryable(boost::exception *ex);
};
class Converter {
public:
  static map<string, string> *mapPointer(map<string, string> m);
  static map<string, boost::any> *mapPointer(map<string, boost::any> m);
  static map<string, string> merge(map<string, string> *m, ...);
  static map<string, boost::any> merge(map<string, boost::any> *m, ...);
  static string toString(boost::any val);
};
struct Error : virtual std::exception, virtual boost::exception {
public:
  Error();
  Error(map<string, string> error_info);
  Error(boost::any error_info);
  string message;
  string code;
  string data;
  string name;
};
struct UnretryableError : public Error {
public:
  UnretryableError(const Request &last_request,
                   const std::exception &last_exception) {
    _last_request = last_request;
    _last_exception = last_exception;
  }
  Darabonba::Request getLastRequest();
  std::exception getLastException();

private:
  Request _last_request;
  std::exception _last_exception;
};
} // namespace Darabonba

#endif