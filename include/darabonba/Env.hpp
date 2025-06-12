#ifndef DARABONBA_ENV_H_
#define DARABONBA_ENV_H_

#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Darabonba {
class Env {
public:
  static std::string getEnv(const std::string &key,
                            const std::string &defaultVal = "") {
    auto ret = std::getenv(key.c_str());
    if (ret == nullptr)
      return defaultVal;
    return ret;
  }
  /**
   * @ref https://stackoverflow.com/questions/54279450/putenv-warning-with-c
   */
  static void setEnv(const std::string &name, const std::string &value) {
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
  }

  static void unsetEnv(const std::string &name) {
#ifdef _WIN32
    _putenv_s(name.c_str(), nullptr);
#else
    unsetenv(name.c_str());
#endif
  }
};
} // namespace Darabonba
#endif