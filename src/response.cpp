#include <darabonba/core.hpp>

void Darabonba::ResponseBody::write(char *ptr, size_t size)  {
  _os->write(ptr, size);
  _size += size;
  if(_size > 6000) {
    _flow = false;
  }
}

size_t Darabonba::ResponseBody::read() {
  _flow = true;
  _os->str("");
  _size = 0;
  curl_easy_pause(this->_curl, CURLPAUSE_CONT);
  while(true) {
    if(!_flow || _size > 0 ) {
      break;
    }
    const CURLMsg *msg;
    int msgs_in_queue;
    while ((msg = curl_multi_info_read(_mcurl, &msgs_in_queue)) != NULL) {
      if (msg->msg == CURLMSG_DONE) {
        if (msg->easy_handle == _curl)
          return 0;
      }
    }
    int running_handles;
    CURLMcode mcode = curl_multi_perform(_mcurl, &running_handles);
    if (mcode != CURLM_OK && mcode != CURLM_CALL_MULTI_PERFORM) {
      string message = string("curl failed, code") + to_string((int)mcode);
      throw RetryableError::New("DaraRequestError", message);
    }
  }
  return _size;
}
