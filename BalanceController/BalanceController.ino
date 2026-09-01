#include <Arduino.h>
#include <HX711.h>
#include <WiFi.h>

#include "EncoderService.h"
#include "HelmetReceiver.h"
#include "LoadFeatureModel.h"
#include "MotorController.h"
#include "SafetyGate.h"
#include "WebInterface.h"

extern "C" void score(double*, double*);

namespace {
constexpr char AP_SSID[] = "ESP32_Balance";
constexpr char AP_PASSWORD[] = "12345678";

constexpr uint8_t LC1_DOUT = 34;
constexpr uint8_t LC1_SCK = 16;
constexpr uint8_t LC2_DOUT = 36;
constexpr uint8_t LC2_SCK = 18;
constexpr uint8_t LC3_DOUT = 35;
constexpr uint8_t LC3_SCK = 17;
constexpr uint8_t LC4_DOUT = 39;
constexpr uint8_t LC4_SCK = 19;

constexpr float CALIBRATION_FACTOR = 1.0f;
constexpr uint32_t DATA_INTERVAL = 50;
constexpr float NOISE_THRESHOLD = 100000.0f;
constexpr uint32_t TARE_TIMEOUT_MS = 3000;
constexpr uint8_t TARE_REQUIRED_SAMPLES = 10;
constexpr uint32_t HX711_READY_TIMEOUT_MS = 500;
constexpr size_t WINDOW_SIZE = LoadFeatureModel::WINDOW_SIZE;
constexpr size_t FEATURE_COUNT = LoadFeatureModel::FEATURE_COUNT;
constexpr size_t CLASS_COUNT = 3;

HX711 loadCell1;
HX711 loadCell2;
HX711 loadCell3;
HX711 loadCell4;

SafetyGate safety;
EncoderService encoder;
MotorController motor;
HelmetReceiver helmetReceiver;

DashboardSnapshot dashboardSnapshot{};
SafetySnapshot latestSafetySnapshot{
    false, false, false, false, false, false, "helmet_signal_lost"};

float loadPercentages[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float centerX = 0.0f;
float centerY = 0.0f;
int people = -1;
float currentWeights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
bool loadSubsystemFault = false;

LoadFeatureModel::Window loadHistory = {};
size_t historyIndex = 0;
size_t historyCount = 0;
uint32_t lastDataMs = 0;
uint32_t lastLoadReadyMs[4] = {};

portMUX_TYPE helmetMailboxMux = portMUX_INITIALIZER_UNLOCKED;
bool pendingHelmetUpdate = false;
bool pendingHelmetWorn = false;
uint32_t pendingHelmetNowMs = 0;

CommandResult motorOnAdapter();
CommandResult motorOffAdapter();
CommandResult motorRpmAdapter(uint16_t rpm);
CommandResult motorEstopAdapter();
CommandResult motorResetAdapter();
CommandResult calibrationStartAdapter();
CommandResult calibrationFinishAdapter();
uint32_t savedPulsesPerRevAdapter();
void queueHelmetUpdate(bool worn, uint32_t nowMs);
void copyDashboardSnapshot(DashboardSnapshot& snapshot);

const WebCallbacks WEB_CALLBACKS{
    copyDashboardSnapshot,
    motorOnAdapter,
    motorOffAdapter,
    motorRpmAdapter,
    motorEstopAdapter,
    motorResetAdapter,
    calibrationStartAdapter,
    calibrationFinishAdapter,
    savedPulsesPerRevAdapter,
};
WebInterface web(WEB_CALLBACKS);

float nonnegative(float value) {
  return value > 0.0f ? value : 0.0f;
}

void clearLoadPresentation() {
  for (float& percentage : loadPercentages) percentage = 0.0f;
  centerX = 0.0f;
  centerY = 0.0f;
}

void latchLoadSubsystemFault(const char* reason, uint32_t nowMs) {
  if (loadSubsystemFault) return;
  loadSubsystemFault = true;
  people = -1;
  historyIndex = 0;
  historyCount = 0;
  clearLoadPresentation();
  safety.updatePeople(-1, nowMs);
  Serial.print("Load subsystem fault latched: ");
  Serial.println(reason);
}

bool tareLoadCellBounded(HX711& cell) {
  const uint32_t startedMs = millis();
  int64_t offsetSum = 0;
  uint8_t samples = 0;
  while (samples < TARE_REQUIRED_SAMPLES && millis() - startedMs < TARE_TIMEOUT_MS) {
    if (!cell.is_ready()) continue;
    offsetSum += cell.read();
    ++samples;
  }
  if (samples < TARE_REQUIRED_SAMPLES) return false;
  cell.set_offset(static_cast<long>(offsetSum / TARE_REQUIRED_SAMPLES));
  return true;
}

bool initializeLoadCell(HX711& cell, uint8_t dataPin, uint8_t clockPin,
                        const char* label) {
  cell.begin(dataPin, clockPin);
  cell.set_scale(CALIBRATION_FACTOR);
  if (tareLoadCellBounded(cell)) return true;
  Serial.print(label);
  Serial.println(" bounded tare failed; load input remains fail-safe");
  return false;
}

void setupLoadCells() {
  const bool lc1Ready = initializeLoadCell(loadCell1, LC1_DOUT, LC1_SCK, "LC1");
  const bool lc2Ready = initializeLoadCell(loadCell2, LC2_DOUT, LC2_SCK, "LC2");
  const bool lc3Ready = initializeLoadCell(loadCell3, LC3_DOUT, LC3_SCK, "LC3");
  const bool lc4Ready = initializeLoadCell(loadCell4, LC4_DOUT, LC4_SCK, "LC4");
  if (!lc1Ready || !lc2Ready || !lc3Ready || !lc4Ready) {
    latchLoadSubsystemFault("boot_tare_failed", millis());
    return;
  }
  const uint32_t initializedAtMs = millis();
  for (size_t index = 0; index < 4; ++index) lastLoadReadyMs[index] = initializedAtMs;
}

bool updateLoadIfReady(HX711& cell, float* weight, uint32_t* lastReadyMs,
                       uint32_t nowMs) {
  if (!cell.is_ready()) {
    // A 10 SPS HX711 is normally not ready for most 50 ms loop iterations.
    // Keep the last signed sample until an entire conversion watchdog expires.
    if (nowMs - *lastReadyMs >= HX711_READY_TIMEOUT_MS) {
      latchLoadSubsystemFault("runtime_ready_timeout", nowMs);
    }
    return !loadSubsystemFault;
  }
  const float reading = cell.get_units(1);
  if (!isfinite(reading)) {
    latchLoadSubsystemFault("runtime_invalid_reading", nowMs);
    return false;
  }
  *weight = reading;
  *lastReadyMs = nowMs;
  return true;
}

void appendHistory(const float loads[4]) {
  for (size_t corner = 0; corner < 4; ++corner) {
    loadHistory.samples[historyIndex][corner] = loads[corner];
  }
  historyIndex = (historyIndex + 1) % WINDOW_SIZE;
  if (historyCount < WINDOW_SIZE) ++historyCount;
}

void classifyWindow(uint32_t nowMs) {
  if (historyCount < WINDOW_SIZE) {
    people = -1;
    return;
  }

  const LoadFeatureModel::FeatureVector modelFeatures =
      LoadFeatureModel::calculateWindow(loadHistory, historyIndex);
  double features[FEATURE_COUNT] = {};
  // Do not reorder: model.c was trained against these exact 16 indices.
  features[0] = modelFeatures.values[0];
  features[1] = modelFeatures.values[1];
  features[2] = modelFeatures.values[2];
  features[3] = modelFeatures.values[3];
  features[4] = modelFeatures.values[4];
  features[5] = modelFeatures.values[5];
  features[6] = modelFeatures.values[6];
  features[7] = modelFeatures.values[7];
  features[8] = modelFeatures.values[8];
  features[9] = modelFeatures.values[9];
  features[10] = modelFeatures.values[10];
  features[11] = modelFeatures.values[11];
  features[12] = modelFeatures.values[12];
  features[13] = modelFeatures.values[13];
  features[14] = modelFeatures.values[14];
  features[15] = modelFeatures.values[15];

  double output_scores[CLASS_COUNT] = {};
  score(features, output_scores);
  int predictedPeople = 0;
  for (size_t candidate = 1; candidate < CLASS_COUNT; ++candidate) {
    if (output_scores[candidate] > output_scores[predictedPeople]) {
      predictedPeople = static_cast<int>(candidate);
    }
  }
  people = predictedPeople;
  safety.updatePeople(predictedPeople, nowMs);
}

void sampleLoadCellsAndClassifier(uint32_t nowMs) {
  if (loadSubsystemFault) {
    people = -1;
    clearLoadPresentation();
    return;
  }

  if (!updateLoadIfReady(loadCell1, &currentWeights[0], &lastLoadReadyMs[0], nowMs) ||
      !updateLoadIfReady(loadCell2, &currentWeights[1], &lastLoadReadyMs[1], nowMs) ||
      !updateLoadIfReady(loadCell3, &currentWeights[2], &lastLoadReadyMs[2], nowMs) ||
      !updateLoadIfReady(loadCell4, &currentWeights[3], &lastLoadReadyMs[3], nowMs)) {
    return;
  }
  const float loads[4] = {
      nonnegative(currentWeights[0]), nonnegative(currentWeights[1]),
      nonnegative(currentWeights[2]), nonnegative(currentWeights[3])};
  const float totalWeight = loads[0] + loads[1] + loads[2] + loads[3];
  if (totalWeight > NOISE_THRESHOLD) {
    for (size_t corner = 0; corner < 4; ++corner) {
      loadPercentages[corner] = loads[corner] * 100.0f / totalWeight;
    }
    const float left = loads[0] + loads[2];
    const float right = loads[1] + loads[3];
    const float front = loads[0] + loads[1];
    const float rear = loads[2] + loads[3];
    centerX = ((right - left) / totalWeight) * 100.0f;
    centerY = ((front - rear) / totalWeight) * 100.0f;
  } else {
    clearLoadPresentation();
  }
  appendHistory(currentWeights);
  classifyWindow(nowMs);
}

void queueHelmetUpdate(bool worn, uint32_t nowMs) {
  portENTER_CRITICAL(&helmetMailboxMux);
  pendingHelmetWorn = worn;
  pendingHelmetNowMs = nowMs;
  pendingHelmetUpdate = true;
  portEXIT_CRITICAL(&helmetMailboxMux);
}

void copyDashboardSnapshot(DashboardSnapshot& snapshot) {
  snapshot = dashboardSnapshot;
}

void consumeHelmetMailbox() {
  bool hasUpdate = false;
  bool worn = false;
  uint32_t receivedAtMs = 0;
  portENTER_CRITICAL(&helmetMailboxMux);
  hasUpdate = pendingHelmetUpdate;
  if (hasUpdate) {
    worn = pendingHelmetWorn;
    receivedAtMs = pendingHelmetNowMs;
    pendingHelmetUpdate = false;
  }
  portEXIT_CRITICAL(&helmetMailboxMux);
  if (hasUpdate) safety.noteHelmet(worn, receivedAtMs);
}

bool motorFaultLatched() {
  return motor.faultReason()[0] != '\0';
}

bool safetyFaultLatched() {
  return loadSubsystemFault || motorFaultLatched();
}

const char* dashboardFault() {
  const char* motorFault = motor.faultReason();
  if (motorFault[0] != '\0') return motorFault;
  return loadSubsystemFault ? "load_cell_fault" : "";
}

const char* helmetName(const SafetySnapshot& safetySnapshot) {
  if (!safetySnapshot.helmetFresh) return "signal_lost";
  return safetySnapshot.helmetWorn ? "worn" : "not_worn";
}

void updateDashboardSnapshot(const SafetySnapshot& safetySnapshot) {
  dashboardSnapshot.lc1 = loadPercentages[0];
  dashboardSnapshot.lc2 = loadPercentages[1];
  dashboardSnapshot.lc3 = loadPercentages[2];
  dashboardSnapshot.lc4 = loadPercentages[3];
  dashboardSnapshot.x = centerX;
  dashboardSnapshot.y = centerY;
  dashboardSnapshot.people = people;
  dashboardSnapshot.helmet = helmetName(safetySnapshot);
  dashboardSnapshot.motorAllowed = safetySnapshot.ready;
  dashboardSnapshot.lockReason = safetySnapshot.lockReason;
  dashboardSnapshot.motorState = motorStateName(motor.state());
  dashboardSnapshot.targetRpm = motor.targetRpm();
  dashboardSnapshot.actualRpm = encoder.rpm();
  dashboardSnapshot.pwmPercent = motor.pwmPercent();
  dashboardSnapshot.encoderCalibrated = encoder.isCalibrated();
  dashboardSnapshot.pulsesPerRev = encoder.pulsesPerRevolution();
  dashboardSnapshot.fault = dashboardFault();
}

CommandResult motorOnAdapter() { return motor.requestOn(latestSafetySnapshot); }
CommandResult motorOffAdapter() { return motor.requestOff(millis()); }
CommandResult motorRpmAdapter(uint16_t rpm) { return motor.setTargetRpm(rpm); }
CommandResult motorEstopAdapter() { return motor.requestEstop(); }
CommandResult motorResetAdapter() { return motor.requestReset(latestSafetySnapshot); }
CommandResult calibrationStartAdapter() { return encoder.startCalibration(); }
CommandResult calibrationFinishAdapter() { return encoder.finishCalibration(); }
uint32_t savedPulsesPerRevAdapter() { return encoder.pulsesPerRevolution(); }
}  // namespace

void setup() {
  Serial.begin(115200);
  motor.begin();
  encoder.begin();
  setupLoadCells();

  WiFi.mode(WIFI_AP_STA);
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD, WIFI_CHANNEL)) {
    Serial.println("SoftAP start failed; web and ESP-NOW remain unavailable");
  }
  if (!helmetReceiver.begin(queueHelmetUpdate)) {
    Serial.println("ESP-NOW receiver start failed; helmet gate remains locked");
  }
  web.begin();
  updateDashboardSnapshot(latestSafetySnapshot);
}

void loop() {
  const uint32_t nowMs = millis();
  web.handleClient();
  consumeHelmetMailbox();
  encoder.tick(nowMs);

  if (nowMs - lastDataMs >= DATA_INTERVAL) {
    lastDataMs = nowMs;
    sampleLoadCellsAndClassifier(nowMs);
  }

  const bool encoderReady = encoder.isCalibrated() && !encoder.isCalibrating();
  SafetySnapshot safetySnapshot = safety.evaluate(nowMs, encoderReady, safetyFaultLatched());
  motor.tick(nowMs, safetySnapshot, encoder.rpm());
  safetySnapshot = safety.evaluate(nowMs, encoderReady, safetyFaultLatched());
  latestSafetySnapshot = safetySnapshot;
  updateDashboardSnapshot(safetySnapshot);
}
