#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <cstring>

#include "LocalConfig.h"

constexpr uint8_t WIFI_CHANNEL = 1;
constexpr uint8_t SENSOR_1_PIN = 21;
constexpr uint8_t SENSOR_2_PIN = 35;
constexpr uint8_t SENSOR_3_PIN = 33;
constexpr uint32_t SENSOR_ACTIVE_MS = 5000;
constexpr uint32_t SEND_INTERVAL_MS = 100;

struct HelmetMessage {
  bool isTriggered;
};

esp_now_peer_info_t peerInfo;
uint32_t sensor1LastHighMs = 0;
uint32_t sensor2LastHighMs = 0;
uint32_t sensor3LastHighMs = 0;
uint32_t lastSendMs = 0;
bool sensor1Seen = false;
bool sensor2Seen = false;
bool sensor3Seen = false;
bool lastReportedWorn = false;

void updateSensor(uint8_t pin, uint32_t& lastHighMs, bool& seen, uint32_t nowMs) {
  if (digitalRead(pin) == HIGH) {
    lastHighMs = nowMs;
    seen = true;
  }
}

bool sendHelmetState(bool worn) {
  const HelmetMessage message = {worn};
  return esp_now_send(receiverMac, reinterpret_cast<const uint8_t*>(&message),
                      sizeof(message)) == ESP_OK;
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_1_PIN, INPUT);
  pinMode(SENSOR_2_PIN, INPUT);
  pinMode(SENSOR_3_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  if (esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Failed to set Wi-Fi channel");
    return;
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiverMac, sizeof(peerInfo.peer_addr));
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP-NOW peer registration failed");
  }
}

void loop() {
  const uint32_t nowMs = millis();
  updateSensor(SENSOR_1_PIN, sensor1LastHighMs, sensor1Seen, nowMs);
  updateSensor(SENSOR_2_PIN, sensor2LastHighMs, sensor2Seen, nowMs);
  updateSensor(SENSOR_3_PIN, sensor3LastHighMs, sensor3Seen, nowMs);

  const bool sensor1Active = sensor1Seen && nowMs - sensor1LastHighMs <= SENSOR_ACTIVE_MS;
  const bool sensor2Active = sensor2Seen && nowMs - sensor2LastHighMs <= SENSOR_ACTIVE_MS;
  const bool sensor3Active = sensor3Seen && nowMs - sensor3LastHighMs <= SENSOR_ACTIVE_MS;
  const bool worn = sensor1Active && sensor2Active && sensor3Active;

  if (worn != lastReportedWorn) {
    Serial.printf("Helmet worn: %s\n", worn ? "true" : "false");
    lastReportedWorn = worn;
  }

  if (nowMs - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = nowMs;
    if (!sendHelmetState(worn)) {
      Serial.println("ESP-NOW send failed");
    }
  }
}
