

#include <darabonba/Ini.hpp>
#include <darabonba/String.hpp>
#include <darabonba/Type.hpp>
#include <sstream>

namespace Darabonba {
Ini::Section::operator std::string() const {
  std::stringstream ss;
  ss << "[" << name_ << "]\n";
  for (const auto &p : *this)
    ss << p.first << "=" << p.second << "\n";
  return ss.str();
}

Ini Ini::parse(std::basic_istream<char> &content, CommentType type) {
  Ini obj;
  Ini::KeyValues *pSec;

  for (std::string line; !content.eof() && !content.bad();) {
    std::getline(content, line, '\n');
    line = String::trim(line);
    if (line.empty() || isCommentLine(line, type)) {
      continue;
    } else if (line[0] == '[') {
      // section name
      if (line.back() != ']')
        throw Darabonba::Exception("There is no ] in section name");
      auto secName = std::string(line.begin() + 1, line.end() - 1);
      // if(obj.has(secName))
      //   throw Darabonba::Exception("Duplicated section name.");
      obj.add(secName);
      pSec = &obj.data_[secName];
    } else {
      auto pos = line.find_first_of('=');
      if (pos == std::string::npos)
        throw Darabonba::Exception("There is no = in the key-value pair");
      else if (pSec == nullptr)
        throw Darabonba::Exception("The key-value pair has no section name.");
      else {
        auto name = String::trim(line.substr(0, pos)),
             value = String::trim(line.substr(pos + 1));
        pSec->emplace(std::move(name), std::move(value));
      }
    }
  }
  return obj;
}

Ini::operator std::string() const {
  std::stringstream ss;
  for (const auto &p1 : data_) {
    ss << '[' << p1.first << "]\n";
    for (const auto &p2 : p1.second)
      ss << p2.first << '=' << p2.second << '\n';
    ss << '\n';
  }
  return ss.str();
}

bool Ini::add(const std::string &sectionName) {
  if (sectionName.empty())
    return false;
  auto it = data_.find(sectionName);
  if (it != data_.end())
    return false;
  data_[sectionName] = {};
  return true;
}

bool Ini::remove(const std::string &sectionName) {
  auto it = data_.find(sectionName);
  if (it == data_.end())
    return false;
  data_.erase(it);
  return true;
}

Ini &Ini::update(const std::string &sectionName,
                 const Ini::KeyValues &keyValues) {
  auto it = data_.find(sectionName);
  if (it == data_.end())
    it->second = keyValues;
  else {
    auto &sec = it->second;
    for (const auto &p : keyValues)
      sec.emplace(p.first, p.second);
  }
  return *this;
}

Ini &Ini::update(const Section &section) {
  update(section.name(), section);
  return *this;
}

bool Ini::rename(const std::string &sectionName,
                 const std::string &newSectionName) {
  auto it = data_.find(sectionName);
  if (it == data_.end())
    return false;

  auto keyValues = std::move(it->second);
  data_.erase(it);
  data_[newSectionName] = std::move(keyValues);
  // C++ 17
  // auto nh = data_.extract(it);
  // nh.key() = newSectionName;
  // data_.insert(std::move(nh));
  return true;
}

Ini &Ini::set(const std::string &sectionName, const std::string &key,
              const std::string &value) {
  // if (data_.find(sectionName) == data_.end())
  //   // create according section
  //   data_[sectionName] = {};
  data_[sectionName][key] = value;
  return *this;
}

bool Ini::remove(const std::string &sectionName, const std::string &key) {
  auto itSec = data_.find(sectionName);
  if (itSec == data_.end())
    return false;
  auto &sec = itSec->second;
  auto itVal = sec.find(key);
  if (itVal == sec.end())
    return false;
  sec.erase(itVal);
  return true;
}

bool Ini::rename(const std::string &sectionName, const std::string &key,
                 const std::string &newKey) {
  auto itSec = data_.find(sectionName);
  if (itSec == data_.end())
    return false;
  auto &sec = itSec->second;
  auto itVal = sec.find(key);
  if (itVal == sec.end())
    return false;

  auto val = std::move(itVal->second);
  sec.erase(itVal);
  sec[newKey] = std::move(val);
  // C++ 17
  // auto nh = sec.extract(itVal);
  // nh.key() = newKey;
  // sec.insert(std::move(nh));
  return true;
}

bool Ini::has(const std::string &sectionName, const std::string &key) const {
  auto itSec = data_.find(sectionName);
  if (itSec == data_.end())
    return false;
  else
    return (itSec->second).count(key);
}

const std::string& Ini::get(const std::string &sectionName, const std::string &key,
                     const std::string &defaultValue) const {
  auto itSec = data_.find(sectionName);
  if (itSec == data_.end())
    return defaultValue;
  else {
    auto &sec = itSec->second;
    auto itVal = sec.find(key);
    if (itVal == sec.end())
      return defaultValue;
    else
      return itVal->second;
  }
}

} // namespace Darabonba
