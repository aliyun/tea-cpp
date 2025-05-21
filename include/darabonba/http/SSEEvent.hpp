#ifndef SSE_EVENT_HPP
#define SSE_EVENT_HPP

#include <string>
#include <map>
#include <stdexcept>

// 假设 DaraModel 是一个已经定义的基类
class DaraModel {
public:
    virtual std::map<std::string, std::string> toMap() const = 0;
    virtual void validateRequired(const std::string &value, const std::string &fieldName) const = 0;
};

class SSEEvent : public DaraModel {
public:
    SSEEvent(const std::string &id = "", const std::string &event = "", const std::string &data = "", int retry = 0);

    void validate() const;
    std::map<std::string, std::string> toMap() const override;
    void fromMap(const std::map<std::string, std::string> &m);

    // Accessor methods
    const std::string& getId() const;
    void setId(const std::string &id);

    const std::string& getEvent() const;
    void setEvent(const std::string &event);

    const std::string& getData() const;
    void setData(const std::string &data);

    int getRetry() const;
    void setRetry(int retry);

private:
    std::string id_;
    std::string event_;
    std::string data_;
    int retry_;
};

#endif // SSE_EVENT_HPP
