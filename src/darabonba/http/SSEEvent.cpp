#include <darabonba/http/SSEEvent.hpp>

// 构造函数
SSEEvent::SSEEvent(const std::string &id, const std::string &event, const std::string &data, int retry)
    : id_(id), event_(event), data_(data), retry_(retry) {}

// 验证方法
void SSEEvent::validate() const {
    validateRequired(id_, "id");
    validateRequired(event_, "event");
    validateRequired(data_, "data");
    validateRequired(std::to_string(retry_), "retry");
}

// 将对象转换为 map
std::map<std::string, std::string> SSEEvent::toMap() const {
    std::map<std::string, std::string> result;

    if (!id_.empty()) result["id"] = id_;
    if (!event_.empty()) result["event"] = event_;
    if (!data_.empty()) result["data"] = data_;
    result["retry"] = std::to_string(retry_);

    return result;
}

// 从 map 中更新对象
void SSEEvent::fromMap(const std::map<std::string, std::string> &m) {
    auto it = m.find("id");
    if (it != m.end()) setId(it->second);

    it = m.find("event");
    if (it != m.end()) setEvent(it->second);

    it = m.find("data");
    if (it != m.end()) setData(it->second);

    it = m.find("retry");
    if (it != m.end()) setRetry(std::stoi(it->second));
}

// Accessor methods
const std::string& SSEEvent::getId() const { return id_; }
void SSEEvent::setId(const std::string &id) { id_ = id; }

const std::string& SSEEvent::getEvent() const { return event_; }
void SSEEvent::setEvent(const std::string &event) { event_ = event; }

const std::string& SSEEvent::getData() const { return data_; }
void SSEEvent::setData(const std::string &data) { data_ = data; }

int SSEEvent::getRetry() const { return retry_; }
void SSEEvent::setRetry(int retry) { retry_ = retry; }
