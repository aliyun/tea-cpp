#ifndef DARABONBA_CONSOLE_H_
#define DARABONBA_CONSOLE_H_

#include <iostream>
#include <string>

class Console {
public:
  static void log(const std::string &val) {
    std::cout << "[LOG] " << val << std::endl;
  }

  static void info(const std::string &val) {
    std::cout << "[INFO] " << val << std::endl;
  }

  static void warning(const std::string &val) {
    std::cout << "[WARNING] " << val << std::endl;
  }

  static void debug(const std::string &val) {
    std::cout << "[DEBUG] " << val << std::endl;
  }

  static void error(const std::string &val) {
    std::cerr << "[ERROR] " << val << std::endl;
  }
};

#endif