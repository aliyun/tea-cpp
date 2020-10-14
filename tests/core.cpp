#include "gtest/gtest.h"
#include <boost/date_time.hpp>
#include <cpprest/http_client.h>
#include <cpprest/streams.h>
#include <darabonba/core.hpp>

using namespace std;
using namespace Darabonba;

int64_t timestamp() {
  boost::posix_time::ptime epoch(
      boost::gregorian::date(1970, boost::gregorian::Jan, 1));
  boost::posix_time::time_duration time_from_epoch =
      boost::posix_time::second_clock::universal_time() - epoch;
  return time_from_epoch.total_seconds();
}

TEST(tests_core, test_sleep) {
  int64_t start = timestamp();

  Core::sleep(make_shared<int>(1));
  int64_t end = timestamp();
  int du = boost::lexical_cast<int>(end - start);
  ASSERT_TRUE(du >= 1);
}

TEST(tests_core, test_getBackoffTime) {
  int res = Core::getBackoffTime(nullptr, make_shared<int>(3));
  ASSERT_EQ(0, res);

  string policy = "no";
  map<string, boost::any> backoff;
  backoff["policy"] = boost::any(policy);
  res = Core::getBackoffTime(make_shared<boost::any>(backoff),
                             make_shared<int>(3));
  ASSERT_EQ(0, res);

  policy = "yes";
  backoff["policy"] = boost::any(policy);
  res = Core::getBackoffTime(make_shared<boost::any>(backoff),
                             make_shared<int>(3));
  ASSERT_EQ(0, res);

  int period = 1;
  backoff["period"] = period;
  res = Core::getBackoffTime(make_shared<boost::any>(backoff),
                             make_shared<int>(3));
  ASSERT_EQ(1, res);

  period = -1;
  backoff["period"] = period;
  res = Core::getBackoffTime(make_shared<boost::any>(backoff),
                             make_shared<int>(3));
  ASSERT_EQ(3, res);
}

TEST(tests_core, test_isRetryable) {
  try {
    Darabonba::Request last_request;
    exception ex;
    BOOST_THROW_EXCEPTION(Darabonba::UnretryableError(last_request, ex));
  } catch (boost::exception &e) {
    ASSERT_TRUE(Darabonba::Core::isRetryable(e));
  }

  try {
    BOOST_THROW_EXCEPTION(Darabonba::Error());
  } catch (boost::exception &e) {
    ASSERT_TRUE(Darabonba::Core::isRetryable(e));
  }

  try {
    BOOST_THROW_EXCEPTION(std::exception());
  } catch (boost::exception &e) {
    ASSERT_FALSE(Darabonba::Core::isRetryable(e));
  }

  try {
    throw std::exception();
  } catch (std::exception &e) {
    ASSERT_FALSE(Darabonba::Core::isRetryable(e));
  }
}

TEST(tests_core, test_allowRetry) {
  ASSERT_FALSE(Darabonba::Core::allowRetry(nullptr, make_shared<int>(3),
                                           make_shared<int>(0)));
  map<string, boost::any> retry;
  ASSERT_FALSE(Darabonba::Core::allowRetry(make_shared<boost::any>(retry),
                                           make_shared<int>(3),
                                           make_shared<int>(0)));
  retry["foo"] = boost::any(string("bar"));
  ASSERT_FALSE(Darabonba::Core::allowRetry(make_shared<boost::any>(retry),
                                           make_shared<int>(3),
                                           make_shared<int>(0)));

  retry["maxAttempts"] = boost::any(0);
  ASSERT_FALSE(Darabonba::Core::allowRetry(make_shared<boost::any>(retry),
                                           make_shared<int>(3),
                                           make_shared<int>(0)));

  retry["maxAttempts"] = boost::any("");
  ASSERT_FALSE(Darabonba::Core::allowRetry(make_shared<boost::any>(retry),
                                           make_shared<int>(3),
                                           make_shared<int>(0)));

  retry["maxAttempts"] = boost::any(5);
  ASSERT_TRUE(Darabonba::Core::allowRetry(make_shared<boost::any>(retry),
                                          make_shared<int>(3),
                                          make_shared<int>(0)));
}

TEST(tests_core, test_doAction) {
  Darabonba::Request req;
  req.method = "get";
  req.body = Darabonba::Converter::toStream("test");
  req.query["empty"] = string("");
  req.query["foo"] = string("bar");
  req.query["complex"] = string("evX6fNf^_lUdV#b$_w)B#4>:3|~#]f");
  req.headers["x-foo"] = string("x-bar");
  req.headers["host"] = string("www.aliyun.com");

  setenv("DEBUG", "1", 1);
  Response res = Darabonba::Core::doAction(req);
  ASSERT_TRUE(res.statusCode == 200 || res.statusCode == 302);
  setenv("DEBUG", "", 1);
}

TEST(tests_core, test_doAction_with_runtime) {
  map<string, boost::any> runtime;
  runtime["readTimeout"] = boost::any(10);
  runtime["connectTimeout"] = boost::any(10);
  Darabonba::Request req;
  req.method = "get";
  req.body = Darabonba::Converter::toStream("test");
  req.query["foo"] = string("bar");
  req.headers["x-foo"] = string("x-bar");
  req.headers["host"] = "www.aliyun.com";

  Response res = Darabonba::Core::doAction(req, runtime);
  ASSERT_TRUE(res.statusCode == 200 || res.statusCode == 302);
}

TEST(tests_core, test_doAction_with_special_url) {
  Darabonba::Request req;
  req.method = "GET";
  req.headers["host"] = string("localhost");
  req.port = 8088;
  req.protocol = "http";
  req.pathname = "/?foo=bar";
  req.query["a"] = "b";

  Response res = Darabonba::Core::doAction(req);
  ASSERT_TRUE(res.statusCode == 200 || res.statusCode == 302);

  req.pathname = "?foo=bar&";
  res = Darabonba::Core::doAction(req);
  ASSERT_TRUE(res.statusCode == 200 || res.statusCode == 302);
}

TEST(tests_core, test_http_config) {
  Darabonba::Request req;
  req.host = "a.com";
  map<string, boost::any> runtime = {
      {"noProxy", boost::any(string("example.com,a.com"))},
  };
  web::http::client::http_client_config cfg =
      Darabonba::Core::httpConfig(req, runtime);
  ASSERT_EQ(2, cfg.proxy().disabled);

  req.host = "b.com";
  req.protocol = "http";
  runtime["httpProxy"] = boost::any(string("http://127.0.0.1:1888"));
  cfg = Darabonba::Core::httpConfig(req, runtime);
  ASSERT_EQ(string("http://127.0.0.1:1888/"),
            cfg.proxy().address().to_string());

  req.protocol = "https";
  runtime["httpsProxy"] = boost::any(string("http://127.0.0.1:1666"));
  cfg = Darabonba::Core::httpConfig(req, runtime);
  ASSERT_EQ(string("http://127.0.0.1:1666/"),
            cfg.proxy().address().to_string());
}

TEST(tests_core, test_error) {
  map<string, boost::any> data = {{"requestId", "123456"}};
  map<string, boost::any> m = {
      {"code", string("success")},
      {"message", string("msg")},
      {"data", data},
  };
  try {
    BOOST_THROW_EXCEPTION(Darabonba::Error(m));
    assert(false);
  } catch (Darabonba::Error &e) {
    ASSERT_EQ(string("msg"), e.message);
    ASSERT_EQ(string("success"), e.code);
    ASSERT_EQ(string("{\"requestId\":\"123456\"}"), e.data);
  }
}

TEST(tests_error, test_UnretryableError) {
  Darabonba::Request req;
  req.host = "fake host";
  try {
    try {
      map<string, boost::any> m = {{"code", string("success")},
                                   {"message", string("msg")},
                                   {"data", string("test")},
                                   {"name", string("foo")}};
      BOOST_THROW_EXCEPTION(Darabonba::Error(m));
    } catch (Darabonba::Error &e) {
      BOOST_THROW_EXCEPTION(Darabonba::UnretryableError(req, e));
    }
  } catch (Darabonba::UnretryableError &e) {
    Darabonba::Request r = e.getLastRequest();
    Darabonba::Error ex = e.getLastException();
    ASSERT_EQ(string("fake host"), r.host);
    ASSERT_EQ(string("success"), ex.code);
    ASSERT_EQ(string("msg"), ex.message);
    ASSERT_EQ(string("test"), ex.data);
    ASSERT_EQ(string("foo"), ex.name);
  }
}

TEST(tests_error, test_UnretryableError2) {
  Darabonba::Request req;
  req.host = "fake host";
  try {
    try {
      map<string, string> m = {{"code", string("success")},
                               {"message", string("msg")},
                               {"data", string("test")},
                               {"name", string("foo")}};
      BOOST_THROW_EXCEPTION(Darabonba::Error(m));
    } catch (Darabonba::Error &e) {
      BOOST_THROW_EXCEPTION(Darabonba::UnretryableError(req, e));
    }
  } catch (Darabonba::UnretryableError &e) {
    Darabonba::Request r = e.getLastRequest();
    Darabonba::Error ex = e.getLastException();
    ASSERT_EQ(string("fake host"), r.host);
    ASSERT_EQ(string("success"), ex.code);
    ASSERT_EQ(string("msg"), ex.message);
    ASSERT_EQ(string("test"), ex.data);
    ASSERT_EQ(string("foo"), ex.name);
    ASSERT_STREQ(string("msg").c_str(), ex.what());
  }
}

TEST(tests_error, test_complex_error_info) {
  Darabonba::Request req;
  req.host = "fake host";
  try {
    try {
      const char *c = "const char*";
      float fl = 2.345;
      char ch[] = "char*";
      map<string, boost::any> m = {
          {"data", boost::any(map<string, boost::any>(
                       {{"foo", string("bar")},
                        {"vector", boost::any(vector<boost::any>({"a", "b"}))},
                        {"int", 123},
                        {"long", LONG_MAX},
                        {"double", 1.123},
                        {"float", fl},
                        {"bool", true},
                        {"const_char*", c},
                        {"char*", ch}}))},
      };
      BOOST_THROW_EXCEPTION(Darabonba::Error(m));
    } catch (Darabonba::Error &e) {
      BOOST_THROW_EXCEPTION(Darabonba::UnretryableError(req, e));
    }
  } catch (Darabonba::UnretryableError &e) {
    Darabonba::Request r = e.getLastRequest();
    Darabonba::Error ex = e.getLastException();
    ASSERT_EQ(
        string("{\"bool\":true,\"char*\":\"char*\",\"const_char*\":\"const "
               "char*\",\"double\":1.123000,\"float\":2.345000,\"foo\":bar,"
               "\"int\":123,\"long\":9223372036854775807,\"vector\":[\"a\","
               "\"b\"]}"),
        ex.data);
  }
}

TEST(tests_stream, test_daraStream) {
  Darabonba::Stream fs(make_shared<fstream>("test.txt", ios::in));
  Darabonba::Stream ss(make_shared<stringstream>("test stringstream"));
  shared_ptr<boost::any> s;
  s = shared_ptr<boost::any>(new boost::any(ss));
  if (typeid(Stream) == s->type()) {
    Darabonba::Stream st = boost::any_cast<Darabonba::Stream>(*s);
    ASSERT_EQ("test stringstream", st.read());
  } else {
    assert(false);
  }
  concurrency::streams::stringstreambuf buf("test restcpp stream");
  Darabonba::Stream rs(make_shared<concurrency::streams::istream>(buf));
  ASSERT_EQ("test restcpp stream", rs.read());
}
