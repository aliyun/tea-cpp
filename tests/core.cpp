#include "gtest/gtest.h"
#include <boost/date_time.hpp>
#include <darabonba/core.hpp>

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
  Core::sleep(new int(1));
  int64_t end = timestamp();
  int du = boost::lexical_cast<int>(end - start);
  ASSERT_TRUE(du >= 1);
}

TEST(tests_core, test_getBackoffTime) {
  string policy = "no";
  map<string, boost::any> backoff;
  backoff["policy"] = boost::any(policy);
  int res = Core::getBackoffTime(new boost::any(backoff), new int(3));
  ASSERT_EQ(0, res);

  policy = "yes";
  backoff["policy"] = boost::any(policy);
  res = Core::getBackoffTime(new boost::any(backoff), new int(3));
  ASSERT_EQ(0, res);

  int period = 1;
  backoff["period"] = period;
  res = Core::getBackoffTime(new boost::any(backoff), new int(3));
  ASSERT_EQ(1, res);

  period = -1;
  backoff["period"] = period;
  res = Core::getBackoffTime(new boost::any(backoff), new int(3));
  ASSERT_EQ(3, res);
}

TEST(tests_core, test_isRetryable) {
  try {
    Darabonba::Request last_request;
    exception ex;
    BOOST_THROW_EXCEPTION(Darabonba::UnretryableError(last_request, ex));
  } catch (boost::exception &e) {
    ASSERT_TRUE(Darabonba::Core::isRetryable(&e));
  }

  try {
    BOOST_THROW_EXCEPTION(Darabonba::Error());
  } catch (boost::exception &e) {
    ASSERT_TRUE(Darabonba::Core::isRetryable(&e));
  }

  try {
    BOOST_THROW_EXCEPTION(std::exception());
  } catch (boost::exception &e) {
    ASSERT_FALSE(Darabonba::Core::isRetryable(&e));
  }

  try {
    throw std::exception();
  } catch (std::exception &e) {
    ASSERT_FALSE(Darabonba::Core::isRetryable(&e));
  }
}

TEST(tests_core, test_allowRetry) {
  map<string, boost::any> retry;
  ASSERT_FALSE(Darabonba::Core::allowRetry(new boost::any(retry), new int(3),
                                           new int(0)));

  retry["maxAttempts"] = boost::any(0);
  ASSERT_FALSE(Darabonba::Core::allowRetry(new boost::any(retry), new int(3),
                                           new int(0)));

  retry["maxAttempts"] = boost::any("");
  ASSERT_FALSE(Darabonba::Core::allowRetry(new boost::any(retry), new int(3),
                                           new int(0)));

  retry["maxAttempts"] = boost::any(5);
  ASSERT_TRUE(Darabonba::Core::allowRetry(new boost::any(retry), new int(3),
                                          new int(0)));
}

TEST(tests_core, test_doRequset) {
  Darabonba::Request req;
  req.method = "get";
  req.body = "test";
  req.query["foo"] = string("bar");
  req.headers["x-foo"] = string("x-bar");
  req.headers["host"] = "example.com";

  Response res = Darabonba::Core::doRequest(req);
  ASSERT_EQ(200, res.statusCode);
}

TEST(tests_core, test_doRequest_with_runtime) {
  map<string, boost::any> runtime;
  runtime["readTimeout"] = boost::any(10);
  runtime["connectTimeout"] = boost::any(10);
  Darabonba::Request req;
  req.method = "get";
  req.body = "test";
  req.query["foo"] = string("bar");
  req.headers["x-foo"] = string("x-bar");
  req.headers["host"] = "example.com";

  Response res = Darabonba::Core::doRequest(req, runtime);
  ASSERT_EQ(200, res.statusCode);
}