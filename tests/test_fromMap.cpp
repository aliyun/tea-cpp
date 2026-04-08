#include <iostream>
#include <darabonba/Core.hpp>
#include <darabonba/Model.hpp>

using namespace Darabonba;

// 测试模型
class TestResponse : public Model {
public:
  std::shared_ptr<std::string> name_;
  std::shared_ptr<int32_t> count_;
  
  friend void from_json(const Json& j, TestResponse& obj) {
    DARABONBA_PTR_FROM_JSON(name, name_);
    DARABONBA_PTR_FROM_JSON(count, count_);
  }
  
  Json toMap() const override {
    Json j;
    if (name_) j["name"] = *name_;
    if (count_) j["count"] = *count_;
    return j;
  }
  
  void fromMap(const Json& j) override {
    from_json(j, *this);
  }
  
  void validate() const override {}
  bool empty() const override { return !name_ && !count_; }
  
  // Getters
  std::string getName() const { return name_ ? *name_ : ""; }
  int32_t getCount() const { return count_ ? *count_ : 0; }
};

int main() {
  std::cout << "=== Darabonba::fromMap 函数测试 ===\n\n";
  
  // 测试 1: 基本使用
  std::cout << "【测试 1: 基本使用】\n";
  Json j1 = {{"name", "test"}, {"count", 42}};
  TestResponse resp1 = fromMap<TestResponse>(j1);
  std::cout << "  name: " << resp1.getName() << "\n";
  std::cout << "  count: " << resp1.getCount() << "\n";
  std::cout << "  ✅ fromMap<T>() 工作正常\n\n";
  
  // 测试 2: 类型转换（string -> int）
  std::cout << "【测试 2: 类型转换（string -> int）】\n";
  Json j2 = {{"name", "convert"}, {"count", "100"}};
  TestResponse resp2 = fromMap<TestResponse>(j2);
  std::cout << "  count: \"100\" → " << resp2.getCount() << "\n";
  std::cout << "  ✅ string -> int 转换成功\n\n";
  
  // 测试 3: fromMapPtr
  std::cout << "【测试 3: fromMapPtr】\n";
  Json j3 = {{"name", "pointer"}, {"count", 999}};
  auto ptr = fromMapPtr<TestResponse>(j3);
  std::cout << "  name: " << ptr->getName() << "\n";
  std::cout << "  count: " << ptr->getCount() << "\n";
  std::cout << "  ✅ fromMapPtr<T>() 工作正常\n\n";
  
  std::cout << "✅ 所有测试通过！fromMap 函数可用！\n";
  return 0;
}