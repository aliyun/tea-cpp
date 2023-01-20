#include <darabonba/http/SSEEvent.hpp>
#include <nlohmann/json.hpp>

namespace Darabonba {
namespace Http {

// void SSEEvent::validate() const {
//     // Add any specific validation logic here
//     if (!id_ || !event_ || !data_ || !retry_) {
//         throw std::invalid_argument("Fields cannot be null");
//     }
// }

const std::string &SSEEvent::getId() const { return *id_; }
void SSEEvent::setId(const std::string &id) { *id_ = id; }

const std::string &SSEEvent::getEvent() const { return *event_; }
void SSEEvent::setEvent(const std::string &event) { *event_ = event; }

const std::string &SSEEvent::getData() const { return *data_; }
void SSEEvent::setData(const std::string &data) { *data_ = data; }

// int SSEEvent::getRetry() const { return *retry_; }
void SSEEvent::setRetry(int retry) { *retry_ = retry; }

} // namespace Http
} // namespace Darabonba
