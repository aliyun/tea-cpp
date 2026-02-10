#ifndef DARABONBA_FILE_HPP
#define DARABONBA_FILE_HPP

#include <darabonba/Date.hpp>
#include <darabonba/Stream.hpp>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Darabonba {

/**
 * File class aligned with tea-python's File class
 * Provides file operations: exists, path, length, createTime, modifyTime, read, write
 */
class File {
public:
  /**
   * Constructor with file path
   * @param path The file path
   */
  explicit File(const std::string &path) : path_(path), file_(nullptr) {}

  /**
   * Destructor - closes file if open
   */
  ~File() {
    if (file_ && file_->is_open()) {
      file_->close();
    }
  }

  // Disable copy
  File(const File &) = delete;
  File &operator=(const File &) = delete;

  // Enable move
  File(File &&other) noexcept
      : path_(std::move(other.path_)), file_(std::move(other.file_)) {}

  File &operator=(File &&other) noexcept {
    if (this != &other) {
      if (file_ && file_->is_open()) {
        file_->close();
      }
      path_ = std::move(other.path_);
      file_ = std::move(other.file_);
    }
    return *this;
  }

  /**
   * Check if a file exists at the given path
   * @param path The file path to check
   * @return true if file exists, false otherwise
   */
  static bool exists(const std::string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
  }

  /**
   * Get the file path
   * @return The file path
   */
  std::string path() const { return path_; }

  /**
   * Get the file size in bytes
   * @return The file size
   */
  int64_t length() const {
    struct stat buffer;
    if (stat(path_.c_str(), &buffer) == 0) {
      return static_cast<int64_t>(buffer.st_size);
    }
    return 0;
  }

  /**
   * Get the file creation time
   * @return Date object representing creation time
   */
  Date createTime() const {
    struct stat buffer;
    if (stat(path_.c_str(), &buffer) == 0) {
#ifdef __APPLE__
      std::time_t ctime = static_cast<std::time_t>(buffer.st_birthtime);
#elif defined(_WIN32)
      std::time_t ctime = static_cast<std::time_t>(buffer.st_ctime);
#else
      // Linux doesn't have reliable creation time, use ctime as fallback
      std::time_t ctime = static_cast<std::time_t>(buffer.st_ctime);
#endif
      return Date(ctime, 0);
    }
    return Date();
  }

  /**
   * Get the file modification time
   * @return Date object representing modification time
   */
  Date modifyTime() const {
    struct stat buffer;
    if (stat(path_.c_str(), &buffer) == 0) {
      std::time_t mtime = static_cast<std::time_t>(buffer.st_mtime);
      return Date(mtime, 0);
    }
    return Date();
  }

  /**
   * Read bytes from the file
   * @param size Number of bytes to read
   * @return Bytes read from file (as vector<uint8_t>)
   */
  std::vector<uint8_t> read(int64_t size) {
    if (!file_) {
      file_ = std::make_unique<std::ifstream>(path_, std::ios::binary);
    }

    if (!file_->is_open() || !file_->good()) {
      return std::vector<uint8_t>();
    }

    std::vector<uint8_t> buffer;
    buffer.resize(static_cast<size_t>(size));
    file_->read(reinterpret_cast<char *>(buffer.data()), size);
    std::streamsize bytesRead = file_->gcount();

    if (bytesRead < size) {
      buffer.resize(static_cast<size_t>(bytesRead));
    }

    // Close file if we've reached the end
    if (file_->eof()) {
      file_->close();
      file_.reset();
    }

    return buffer;
  }

  /**
   * Write bytes to the file (append mode)
   * @param data Bytes to write
   */
  void write(const std::vector<uint8_t> &data) {
    std::ofstream outFile(path_, std::ios::binary | std::ios::app);
    if (outFile.is_open()) {
      outFile.write(reinterpret_cast<const char *>(data.data()), data.size());
      outFile.close();
    }
  }

  /**
   * Write string to the file (append mode)
   * @param data String to write
   */
  void write(const std::string &data) {
    std::ofstream outFile(path_, std::ios::binary | std::ios::app);
    if (outFile.is_open()) {
      outFile.write(data.c_str(), data.size());
      outFile.close();
    }
  }

  /**
   * Create a readable stream from a file path
   * @param path The file path
   * @return Shared pointer to IStream
   */
  static std::shared_ptr<IStream> createReadStream(const std::string &path) {
    return Stream::readFromFilePath(path);
  }

  /**
   * Create a writable stream for a file path
   * @param path The file path
   * @return Shared pointer to OFStream
   */
  static std::shared_ptr<OFStream> createWriteStream(const std::string &path) {
    auto stream = std::make_shared<OFStream>();
    stream->open(path, std::ios::binary | std::ios::app);
    return stream;
  }

private:
  std::string path_;
  std::unique_ptr<std::ifstream> file_;
};

} // namespace Darabonba

#endif // DARABONBA_FILE_HPP
