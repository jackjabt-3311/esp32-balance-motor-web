#pragma once

#include <Arduino.h>
#include <esp_arduino_version.h>
#include <esp_now.h>

// The main sketch must enter WIFI_AP_STA mode and start its soft AP on this
// channel before calling HelmetReceiver::begin().
constexpr uint8_t WIFI_CHANNEL = 1;

struct HelmetMessage {
  bool isTriggered;
};

using HelmetUpdateCallback = void (*)(bool worn, uint32_t nowMs);

class HelmetReceiver {
 public:
  bool begin(HelmetUpdateCallback callback);

 private:
  static void handlePacket(const uint8_t* data, int len);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  static void onReceive(
      const esp_now_recv_info_t* info,
      const uint8_t* data,
      int len);
#else
  static void onReceive(const uint8_t* mac, const uint8_t* data, int len);
#endif

  static HelmetUpdateCallback callback_;
};
