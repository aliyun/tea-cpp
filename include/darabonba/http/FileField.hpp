#ifndef DARABONBA_HTTP_FILE_FIELD_H_
#define DARABONBA_HTTP_FILE_FIELD_H_

#include <algorithm>
#include <cstring>
#include <darabonba/Core.hpp>
#include <darabonba/Model.hpp>
#include <darabonba/Stream.hpp>
#include <darabonba/Type.hpp>
#include <memory>
#include <sstream>
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
  FileField(const Darabonba::Json &obj) { from_json(obj, *this); }

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
  std::string getContentType() const {
    DARABONBA_PTR_GET_DEFAULT(contentType_, "");
  }
  FileField &setContentType(const std::string &contentType) {
    DARABONBA_PTR_SET_VALUE(contentType_, contentType);
  }
  FileField &setContentType(std::string &&contentType) {
    DARABONBA_PTR_SET_RVALUE(contentType_, contentType);
  }

  bool hasContent() const { return this->content_ != nullptr; }
  std::shared_ptr<IStream> getContent() const { DARABONBA_GET(content_); }
  FileField &setContent(std::shared_ptr<IStream> content) {
    DARABONBA_SET_VALUE(content_, content);
  }

  bool hasFilename() const { return this->filename_ != nullptr; }
  std::string getFilename() const { DARABONBA_PTR_GET_DEFAULT(filename_, ""); }
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
  FileFormStream(const Json obj) : form_(obj) {
    if (form_.is_object()) {
      for (auto it = form_.begin(); it != form_.end(); ++it) {
        keys_.push_back(it.key());
      }
    }
  }

  virtual ~FileFormStream() { curl_mime_free(mime_); }

  /**
   * @note This is a specail IStream class whose read method is implemented by
   * libcurl
   */
  virtual size_t read(char *buffer, size_t expectSize) override {
    if (buffer == nullptr || expectSize == 0) {
      return 0;
    }

    // 如果正在流式读取文件内容
    if (streaming_ && streamingStream_) {
      size_t bytesRead = streamingStream_->read(buffer, expectSize);
      if (bytesRead > 0) {
        return bytesRead;
      }
      // 文件读取完成,添加\r\n
      streaming_ = false;
      streamingStream_ = nullptr;
      const char *fileEnd = "\r\n";
      size_t endLen = 2;
      if (endLen <= expectSize) {
        std::memcpy(buffer, fileEnd, endLen);
        index_++;
        return endLen;
      }
      return 0;
    }

    // 读取表单字段
    if (index_ < keys_.size()) {
      std::string name = keys_[index_];
      const auto &fieldValue = form_[name];

      // 检查是否为文件字段
      if (isFileFiled(fieldValue)) {
        streaming_ = true;
        streamingStream_ =
            fieldValue["content"].get<std::shared_ptr<IStream>>();
        std::string filename = fieldValue["filename"].get<std::string>();
        std::string contentType = fieldValue["contentType"].get<std::string>();

        std::ostringstream oss;
        oss << "--" << boundary_ << "\r\n"
            << "Content-Disposition: form-data; name=\"" << name
            << "\"; filename=\"" << filename << "\"\r\n"
            << "Content-Type: " << contentType << "\r\n\r\n";
        std::string header = oss.str();

        size_t copyLen = std::min(header.size(), expectSize);
        std::memcpy(buffer, header.c_str(), copyLen);
        return copyLen;
      } else {
        std::ostringstream oss;
        oss << "--" << boundary_ << "\r\n"
            << "Content-Disposition: form-data; name=\"" << name << "\"\r\n\r\n"
            << fieldValue.dump() << "\r\n";
        std::string formField = oss.str();

        size_t copyLen = std::min(formField.size(), expectSize);
        std::memcpy(buffer, formField.c_str(), copyLen);
        index_++;
        return copyLen;
      }
    } else if (index_ == keys_.size()) {
      std::string endStr = "--" + boundary_ + "--\r\n";
      size_t copyLen = std::min(endStr.size(), expectSize);
      std::memcpy(buffer, endStr.c_str(), copyLen);
      index_++;
      return copyLen;
    } else {
      return 0;
    }
  }

  virtual bool isFinished() const override { return index_ > keys_.size(); }

  const Json &getForm() const { return form_; }
  curl_mime *getMime() const { return mime_; }
  void setMine(curl_mime *mine) { mime_ = mine; }
  void setBoundary(const std::string &boundary) { boundary_ = boundary; }

protected:
  curl_mime *mime_ = nullptr;
  Json form_ = nullptr;
  std::string boundary_;
  std::vector<std::string> keys_;
  size_t index_ = 0;
  bool streaming_ = false;
  std::shared_ptr<IStream> streamingStream_ = nullptr;
};

} // namespace Http
} // namespace Darabonba
#endif