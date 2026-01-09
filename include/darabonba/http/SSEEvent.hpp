#ifndef SSE_EVENT_HPP
#define SSE_EVENT_HPP

#include <darabonba/Model.hpp>
#include <darabonba/Type.hpp>
#include <string>

using json = nlohmann::json;

namespace Darabonba{
  namespace Http {
    class SSEEvent : public Darabonba::Model {
    public:
      friend void to_json(Darabonba::Json &j, const SSEEvent &obj) {
        DARABONBA_PTR_TO_JSON(id, id_);
        DARABONBA_PTR_TO_JSON(data, event_);
        DARABONBA_PTR_TO_JSON(data, data_);
        DARABONBA_PTR_TO_JSON(retry, retry_);
      };

      friend void from_json(const Darabonba::Json &j, SSEEvent &obj) {
        DARABONBA_PTR_FROM_JSON(id, id_);
        DARABONBA_PTR_FROM_JSON(event, event_);
        DARABONBA_PTR_FROM_JSON(data, data_);
        DARABONBA_PTR_FROM_JSON(retry, retry_);
      };

      SSEEvent(const std::string &id = "", const std::string &event = "", const std::string &data = "", int retry = 0);

      SSEEvent(const Darabonba::Json &obj) { from_json(obj, *this); };

      SSEEvent &operator=(const SSEEvent &) = default;

      void validate() const {};

      virtual bool empty() const override {};

      virtual Darabonba::Json toMap() const override {
        Darabonba::Json obj;
        to_json(obj, *this);
        return obj;
      };

      virtual void fromMap(const Darabonba::Json &obj) override {
        from_json(obj, *this);
        validate();
      };

      // Accessor methods
      const std::string &getId() const;

      void setId(const std::string &id);

      const std::string &getEvent() const;

      void setEvent(const std::string &event);

      const std::string &getData() const;

      void setData(const std::string &data);

      int getRetry() const;

      void setRetry(int retry);

    private:
      std::shared_ptr<std::string> id_;
      std::shared_ptr<std::string> event_;
      std::shared_ptr<std::string> data_;
      std::shared_ptr<int> retry_;
    };
  }
}

#endif // SSE_EVENT_HPP
