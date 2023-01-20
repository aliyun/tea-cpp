#ifndef DARABONBA_INI_H_
#define DARABONBA_INI_H_

#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

namespace Darabonba {

class Ini {
public:
  using KeyValues = std::map<std::string, std::string>;
  class Section : public KeyValues {
  protected:
    std::string name_;

  public:
    Section() = default;

    Section(const std::string &name, const KeyValues &data = {})
        : KeyValues(data), name_(name) {}

    Section &operator=(const KeyValues &body) {
      *reinterpret_cast<KeyValues *>(this) = body;
      return *this;
    }

    const std::string &get(const std::string &key,
                           const std::string &defaultVal = "") const {
      auto it = this->find(key);
      if (it == this->end()) {
        return defaultVal;
      }
      return it->second;
    }

    const std::string &name() const { return name_; }

    Section &setName(const std::string &name) {
      name_ = name;
      return *this;
    }

    operator std::string() const;
  };

  enum class CommentType {
    SEMICOLON = ';',   // char ';'
    NUMBER_SIGN = '#', // char '#'
    POSSIBLE = '\0',   // char ';' or char '#'
  };

  Ini() = default;
  ~Ini() = default;

  /**
   * @brief Parse Ini from std::string_view
   */
  static Ini parse(std::basic_istream<char> &content,
                   CommentType type = CommentType::POSSIBLE);

  /**
   * @brief Convert Ini object into std::string
   */
  operator std::string() const;

  /**
   * @brief Update the Ini object by another Ini object.
   */
  Ini &update(const Ini &ini) {
    for (const auto &p : ini.data_) {
      update(p.first, p.second);
    }
    return *this;
  }

  /**
   * @brief Add a empty section.
   * @return Return false if the section exists.
   */
  bool add(const std::string &sectionName);

  /**
   * @brief Remove the section
   * @return Return false if the section doesn't exist.
   */
  bool remove(const std::string &sectionName);

  /**
   * @brief Update the Ini object by a specify Section object.
   */
  Ini &update(const Section &section);

  /**
   * @brief Rename the section.
   * @return Return false if the section doesn't exist.
   */
  bool rename(const std::string &sectionName,
              const std::string &newSectionName);

  /**
   * @brief Determine whether there is a corresponding section.
   */
  bool has(const std::string &sectionName) const {
    return data_.find(sectionName) != data_.end();
  }

  /**
   * @brief Get the Section Object
   */
  Section get(const std::string &sectionName,
              const Section &defaultVal = {}) const {
    auto it = data_.find(sectionName);
    if (it == data_.end()) {
      return defaultVal;
    }
    return Section(sectionName, it->second);
  }

  /**
   * @brief Set the value
   * @note The section or key will be created automatically if they don't exist.
   */
  Ini &set(const std::string &sectionName, const std::string &key,
           const std::string &value);

  /**
   * @brief Remove the value
   * @return Return false if the section or the key doesn't exist.
   */
  bool remove(const std::string &sectionName, const std::string &key);

  /**
   * @brief Rename the key
   * @return Return false if the section or the key doesn't exist.
   */
  bool rename(const std::string &sectionName, const std::string &key,
              const std::string &newKey);

  /**
   * @brief Determine whether there is a key.
   */
  bool has(const std::string &sectionName, const std::string &key) const;

  /**
   * @brief Get the value.
   */
  const std::string &get(const std::string &sectionName, const std::string &key,
                         const std::string &defaultValue = "") const;

protected:
  /**
   * @brief Update Section by key-values pair
   */
  Ini &update(const std::string &sectionName, const KeyValues &keyvalues);

  /**
   * @brief Determine whether is is a comment line.
   */
  static bool isCommentLine(const std::string &str, CommentType type) {
    if (type == CommentType::POSSIBLE)
      return str[0] == (char)CommentType::SEMICOLON ||
             str[0] == (char)CommentType::NUMBER_SIGN;
    else
      return str[0] == (char)type;
  }

  std::map<std::string, KeyValues> data_;
};

} // namespace Darabonba
#endif