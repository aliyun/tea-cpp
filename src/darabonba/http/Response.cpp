// #include <darabonba/Exception.hpp>
// #include <darabonba/http/MCurlResponse.hpp>
// #include <string>

// using std::string;

// namespace Darabonba {

// namespace Http {

// void MCurlResponseBody::write(char *ptr, size_t size) {
//   std::stringstream::write(ptr, size);
//   size_ += size;
//   if (size_ > MAX_SIZE) {
//     flow_ = false;
//   }
// }

// size_t MCurlResponseBody::readFromCurl() {
//   flow_ = true;
//   // clear
//   std::stringstream::str("");
//   size_ = 0;

//   curl_easy_pause(curl_, CURLPAUSE_CONT);
//   while (true) {
//     if (!flow_ || size_ > 0) {
//       break;
//     }
//     const CURLMsg *msg;
//     int msgs_in_queue;
//     while ((msg = curl_multi_info_read(mCurl_, &msgs_in_queue)) != nullptr) {
//       if (msg->msg == CURLMSG_DONE) {
//         if (msg->easy_handle == curl_)
//           return 0;
//       }
//     }
//     int running_handles;
//     CURLMcode mcode = curl_multi_perform(mCurl_, &running_handles);
//     if (mcode != CURLM_OK && mcode != CURLM_CALL_MULTI_PERFORM) {
//       string message =
//           string("curl failed, code_") + std::to_string((int)mcode);
//       throw RetryableException(
//           {{"code", "DaraRequestError"}, {"message", message}});
//     }
//   }
//   return size_;
// }
// } // namespace Http
// } // namespace Darabonba
