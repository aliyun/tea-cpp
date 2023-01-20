#include <darabonba/core.hpp>

#include <fstream>
#include <utility>
#include <thread>

using namespace std;

void static check_MCUrl_init() {
  if(Darabonba::defaultMCUrl == nullptr) {
    curl_global_init(CURL_GLOBAL_ALL);
    Darabonba::defaultMCUrl = curl_multi_init();
  }
}

void Darabonba::Core::sleep(int sleep_time) {
  this_thread::sleep_for(chrono::seconds(sleep_time));
}

string Darabonba::Core::uuid() {
#if defined(_WIN32)
  char buf[GUID_LEN] = { 0 };
  GUID guid;

  if (CoCreateGuid(&guid))
  {
    return std::move(std::string(""));
  }

  sprintf(buf,
          "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
          guid.Data1, guid.Data2, guid.Data3,
          guid.Data4[0], guid.Data4[1], guid.Data4[2],
          guid.Data4[3], guid.Data4[4], guid.Data4[5],
          guid.Data4[6], guid.Data4[7]);

  return std::move(std::string(buf));
#else
  std::stringstream str;
  str.setf(std::ios_base::uppercase);
  uuid_t uu;
  uuid_generate( uu );

  str << std::hex << std::setfill('0')
      << std::setw(2) << (int)uu[0]
      << std::setw(2) << (int)uu[1]
      << std::setw(2) << (int)uu[2]
      << std::setw(2) << (int)uu[3]
      << "-"
      << std::setw(2) << (int)uu[4]
      << std::setw(2) << (int)uu[5]
      << "-"
      << std::setw(2) << (int)uu[6]
      << std::setw(2) << (int)uu[7]
      << "-"
      << std::setw(2) << (int)uu[8]
      << std::setw(2) << (int)uu[9]
      << "-"
      << std::setw(2) << (int)uu[10]
      << std::setw(2) << (int)uu[11]
      << std::setw(2) << (int)uu[12]
      << std::setw(2) << (int)uu[13]
      << std::setw(2) << (int)uu[14]
      << std::setw(2) << (int)uu[15];
  return str.str();
#endif
}

string Darabonba::Core::getEnv(const std::string env) {
#ifdef _WIN32
  char *buf = nullptr;
  size_t sz = 0;
  if (_dupenv_s(&buf, &sz, env.c_str()) == 0 && buf != nullptr) {
    std::string var(buf);
    free(buf);
    return var;
  } else {
    if (buf) {
      free(buf);
    }
    return std::string();
  }
#else
  char *var = getenv(env.c_str());
  if (var) {
    return std::string(var);
  }
  return std::string();
#endif
}

int Darabonba::Core::getBackoffTime(const json &backoff, int retry_times) {
  if (backoff.empty()) {
    return 0;
  }
  int rt = !retry_times ? 0 : retry_times;
  int back_off_time = 0;
  string policy = "no";
  if (backoff.find("policy") != backoff.end()) {
    policy = backoff["policy"].get<string>();
  }
  if (policy == "no") {
    return back_off_time;
  }

  if (backoff.find("period") != backoff.end()) {
    int period = backoff["period"].get<int>();
    back_off_time = period;
    if (back_off_time <= 0) {
      return rt;
    }
  }
  return back_off_time;
}

bool Darabonba::Core::allowRetry(const json &retry, int retry_times) {
  if (retry.empty()) {
    return false;
  }
  int maxAttempts = 0;
  if (retry.find("maxAttempts") != retry.end()) {
    maxAttempts = retry["maxAttempts"].get<int>();
    return maxAttempts >= retry_times;
  }
  return false;
}

json Darabonba::Core::merge(const json& jf, const json& jb) {
  json ret;
  ret.merge_patch(jf);
  ret.merge_patch(jb);
  return ret;
}

shared_ptr<Darabonba::Response> Darabonba::Core::doAction(const Darabonba::Request &request,
                                              const Darabonba::RuntimeOptions &runtime) {
  check_MCUrl_init();
  HttpClient client = HttpClient(defaultMCUrl);
  return client.doAction(request, runtime);
}

shared_ptr<Darabonba::Response> Darabonba::Core::doAction(const Darabonba::Request &request) {
  check_MCUrl_init();
  HttpClient client = HttpClient(defaultMCUrl);
  RuntimeOptions runtime = RuntimeOptions();
  return client.doAction(request, runtime);
}