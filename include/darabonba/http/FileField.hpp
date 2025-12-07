#ifndef DARABONBA_HTTP_FILE_FIELD_H_
#define DARABONBA_HTTP_FILE_FIELD_H_

#include <darabonba/Core.hpp>
#include <darabonba/Model.hpp>
#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Darabonba {
namespace Http {

bool isFileFiled(const nlohmann::json &jsonObject);

class FileField : public Model {
  friend void to_json(Darabonba::Json &j, const FileField &obj) {
    DARABONBA_PTR_TO_JSON(contentType, contentType_);
    DARABONBA_TO_JSON(content, content_);
    DARABONBA_PTR_TO_JSON(filename, filename_);
  }

  friend void from_json(const Darabonba::Json &j, FileField &obj) {
    DARABONBA_PTR_FROM_JSON(contentType, contentType_);
    DARABONBA_FROM_JSON(content, content_);
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
  std::shared_ptr<IStream> content() const { DARABONBA_GET(content_); }
  FileField &setContent(std::shared_ptr<IStream> content) {
    DARABONBA_SET_VALUE(content_, content);
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
  std::shared_ptr<IStream> content_ = nullptr;
};

class FileFormStream : public IStream {
public:
  FileFormStream() = default;
  FileFormStream(const Json obj) : form_(obj) {}

  virtual ~FileFormStream() { curl_mime_free(mime_); }

  /**
   * @note This is a specail IStream class whose read method is implemented by
   * libcurl
   */
  virtual size_t read(char *buffer, size_t expectSize) override {
    (void)buffer;
    return expectSize;
  }

  virtual bool isFinished() const override { return true; }

  const Json &form() const { return form_; }
  curl_mime *mime() const { return mime_; }
  void setMine(curl_mime *mine) { mime_ = mine; }
  void setBoundary(const std::string &boundary) { boundary_ = boundary; }

protected:
  curl_mime *mime_ = nullptr;
  Json form_ = nullptr;
  std::string boundary_;
};

} // namespace Http
} // namespace Darabonba
#endif