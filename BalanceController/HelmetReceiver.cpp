#include "HelmetReceiver.h"

#include <esp_now.h>
#include <esp_arduino_version.h>
#include <cstring>

HelmetUpdateCallback HelmetReceiver::callback_ = nullptr;

bool HelmetReceiver::begin(HelmetUpdateCallback callback) {
  if (callback == nullptr) {
    return false;
  }

  callback_ = callback;
  if (esp_now_init() != ESP_OK) {
    callback_ = nullptr;
    return false;
  }

  if (esp_now_register_recv_cb(onReceive) != ESP_OK) {
    callback_ = nullptr;
    return false;
  }
  return true;
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void HelmetReceiver::onReceive(
    const esp_now_recv_info_t* info,
    const uint8_t* data,
    int len) {
  (void)info;
  handlePacket(data, len);
}
#else
void HelmetReceiver::onReceive(const uint8_t* mac, const uint8_t* data, int len) {
  (void)mac;
  handlePacket(data, len);
}
#endif

void HelmetReceiver::handlePacket(const uint8_t* data, int len) {
  if (data == nullptr || len != sizeof(HelmetMessage) || callback_ == nullptr) {
    return;
  }

  HelmetMessage message;
  memcpy(&message, data, sizeof(message));
  callback_(message.isTriggered, millis());
}
