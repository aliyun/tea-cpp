#ifndef DARABONBA_CORE_TESTS_MOCK_H
#define DARABONBA_CORE_TESTS_MOCK_H

#include <darabonba/core.hpp>
#include <boost/any.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

using namespace Darabonba;
using namespace std;

class MockModel : public Model {
public:
  explicit MockModel(const std::map<string, boost::any> &config)
      : Model(config){
            fromMap(config);
        };
  MockModel(){};
  ~MockModel(){};
  void validate() override {}
  void fromMap(std::map<string, boost::any> m) override {
    if (m.empty()) {
      return;
    }
    if (m.find("stringfield") != m.end()) {
      stringfield = new string(boost::any_cast<string>(m.at("stringfield")));
    }
  }
  map<string, boost::any> toMap() override { return map<string, boost::any>(); }

  string *stringfield;
};
#endif // DARABONBA_CORE_TESTS_MOCK_H
