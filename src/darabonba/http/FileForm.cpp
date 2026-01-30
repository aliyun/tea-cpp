#include <darabonba/http/FileField.hpp>

namespace Darabonba {
namespace Http {
  bool isFileFiled(const nlohmann::json& jsonObject) {
    // 检查需要的字段是否存在
    if (jsonObject.contains("content") && jsonObject.contains("contentType") && jsonObject.contains("filename")) {
      try {
        // 检查 content 是否为有效的 std::shared_ptr<IStream>
        auto content = jsonObject.at("content").get<std::shared_ptr<IStream>>();
        if (!content) {
          return false;
        }

        // 检查 contentType 和 filename 是否为 string 类型
        std::string contentType = jsonObject.at("contentType").get<std::string>();
        std::string filename = jsonObject.at("filename").get<std::string>();

        return true; // 所有条件满足
      }
      catch (const nlohmann::json::type_error &e) {
        std::cerr << "Type error: " << e.what() << std::endl;
        return false;
      }
      catch (const nlohmann::json::out_of_range &e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return false;
      }
    }
    return false;
  }
} // namespace Http
} // namespace Darabonba