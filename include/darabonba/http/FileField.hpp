#ifndef DARABONBA_HTTP_FILE_FIELD_H_
#define DARABONBA_HTTP_FILE_FIELD_H_

#include <curl/curl.h>
#include <darabonba/Core.hpp>
#include <darabonba/Model.hpp>
#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Darabonba {
namespace Http {

class FileField : public Model {
  friend void to_json(Darabonba::Json &j, const FileField &obj) {
    DARABONBA_PTR_TO_JSON(contentType, contentType_);
    DARABONBA_PTR_TO_JSON(content, content_);
    DARABONBA_PTR_TO_JSON(filename, filename_);
  }

  friend void from_json(const Darabonba::Json &j, FileField &obj) {
    DARABONBA_PTR_FROM_JSON(contentType, contentType_);
    DARABONBA_PTR_FROM_JSON(content, content_);
    DARABONBA_PTR_FROM_JSON(filename, filename_);
  }

public:
  FileField() = default;
  FileField(const FileField &) = default;
  FileField(FileField &&) = default;
  FileField(const Darabonba::Json &obj) { from_json(obj, *this); }

  virtual ~FileField() = default;

  FileField &operator=(const FileField &) = default;
  FileField &operator=(FileField &&) = default;

  virtual void validate() const override {}

  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  }

  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  }

  virtual bool empty() const override {
    return contentType_ == nullptr && content_ == nullptr &&
           filename_ == nullptr;
  }

  bool hasContentType() const { return this->contentType_ != nullptr; }
  std::string contentType() const {
    DARABONBA_PTR_GET_DEFAULT(contentType_, "");
  }
  FileField &setContentType(const std::string &contentType) {
    DARABONBA_PTR_SET_VALUE(contentType_, contentType);
  }
  FileField &setContentType(std::string &&contentType) {
    DARABONBA_PTR_SET_RVALUE(contentType_, contentType);
  }

  bool hasContent() const { return this->content_ != nullptr; }
  std::string content() const { DARABONBA_PTR_GET_DEFAULT(content_, ""); }
  FileField &setContent(const std::string &content) {
    DARABONBA_PTR_SET_VALUE(content_, content);
  }
  FileField &setContent(std::string &&content) {
    DARABONBA_PTR_SET_RVALUE(content_, content);
  }

  bool hasFilename() const { return this->filename_ != nullptr; }
  std::string filename() const { DARABONBA_PTR_GET_DEFAULT(filename_, ""); }
  FileField &setFilename(const std::string &filename) {
    DARABONBA_PTR_SET_VALUE(filename_, filename);
  }
  FileField &setFilename(std::string &&filename) {
    DARABONBA_PTR_SET_RVALUE(filename_, filename);
  }


protected:
  // the name of the file
  std::shared_ptr<std::string> filename_ = nullptr;

  // the MIME of the file
  std::shared_ptr<std::string> contentType_ = nullptr;

  // the content of the file
  std::shared_ptr<std::string> content_ = nullptr;
};

class FileFormStream : public IStream, public std::vector<FileField> {
public:
  FileFormStream() = default;
  FileFormStream(curl_mime *mime) : mime_(mime) {}

  virtual ~FileFormStream() { curl_mime_free(mime_); }

  /**
   * @note This is a specail IStream class whose read method is implemented by
   * libcurl
   */
  virtual size_t read(char *buffer, size_t expectSize) override {
    return expectSize;
  }

  curl_mime *mime() const { return mime_; }
  void setMine(curl_mime *mine) { mime_ = mine; }

protected:
  curl_mime *mime_ = nullptr;
};

} // namespace Http
} // namespace Darabonba
#endif