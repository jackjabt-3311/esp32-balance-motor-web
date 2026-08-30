#include "EncoderService.h"

#include <cstdlib>

volatile int32_t EncoderService::pulseCount_ = 0;
volatile uint32_t EncoderService::calibrationPulses_ = 0;
volatile bool EncoderService::calibrating_ = false;
portMUX_TYPE EncoderService::pulseMux_ = portMUX_INITIALIZER_UNLOCKED;

void EncoderService::begin() {
  pinMode(ENCA_PIN, INPUT);
  pinMode(ENCB_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA_PIN), onEncoderA, RISING);

  if (preferences_.begin("encoder", false)) {
    pulsesPerRevolution_ = preferences_.getUInt("ppr", 0);
    preferences_.end();
  }

  portENTER_CRITICAL(&pulseMux_);
  pulseCount_ = 0;
  calibrationPulses_ = 0;
  calibrating_ = false;
  portEXIT_CRITICAL(&pulseMux_);
  lastSampleMs_ = 0;
  lastPulseCount_ = 0;
  rpm_ = 0.0f;
}

void EncoderService::tick(uint32_t nowMs) {
  const uint32_t elapsedMs = nowMs - lastSampleMs_;
  if (elapsedMs == 0 || pulsesPerRevolution_ == 0) {
    rpm_ = 0.0f;
    return;
  }
  if (elapsedMs < RPM_SAMPLE_MS) {
    return;
  }

  int32_t pulses = 0;
  portENTER_CRITICAL(&pulseMux_);
  pulses = pulseCount_ - lastPulseCount_;
  lastPulseCount_ = pulseCount_;
  portEXIT_CRITICAL(&pulseMux_);

  lastSampleMs_ = nowMs;
  rpm_ = abs(pulses) * 60000.0f /
         (static_cast<float>(elapsedMs) * static_cast<float>(pulsesPerRevolution_));
}

float EncoderService::rpm() const {
  return rpm_;
}

bool EncoderService::isCalibrated() const {
  return pulsesPerRevolution_ != 0;
}

bool EncoderService::isCalibrating() const {
  bool calibrating = false;
  portENTER_CRITICAL(&pulseMux_);
  calibrating = calibrating_;
  portEXIT_CRITICAL(&pulseMux_);
  return calibrating;
}

CommandResult EncoderService::startCalibration() {
  portENTER_CRITICAL(&pulseMux_);
  if (calibrating_) {
    portEXIT_CRITICAL(&pulseMux_);
    return {false, "already_calibrating"};
  }
  calibrationPulses_ = 0;
  calibrating_ = true;
  portEXIT_CRITICAL(&pulseMux_);
  return {true, "calibration_started"};
}

CommandResult EncoderService::finishCalibration() {
  uint32_t calibrationPulses = 0;
  portENTER_CRITICAL(&pulseMux_);
  if (!calibrating_) {
    portEXIT_CRITICAL(&pulseMux_);
    return {false, "not_calibrating"};
  }
  calibrating_ = false;
  calibrationPulses = calibrationPulses_;
  lastPulseCount_ = pulseCount_;
  portEXIT_CRITICAL(&pulseMux_);
  lastSampleMs_ = millis();
  rpm_ = 0.0f;

  if (calibrationPulses < 1 || calibrationPulses > 100000) {
    return {false, "invalid_pulse_count"};
  }

  if (!preferences_.begin("encoder", false)) {
    return {false, "persistence_failed"};
  }
  const size_t bytesWritten = preferences_.putUInt("ppr", calibrationPulses);
  preferences_.end();
  if (bytesWritten == 0) {
    return {false, "persistence_failed"};
  }

  pulsesPerRevolution_ = calibrationPulses;
  return {true, "calibration_saved"};
}

uint32_t EncoderService::pulsesPerRevolution() const {
  return pulsesPerRevolution_;
}

void IRAM_ATTR EncoderService::onEncoderA() {
  const int32_t direction = digitalRead(ENCB_PIN) == HIGH ? -1 : 1;
  portENTER_CRITICAL_ISR(&pulseMux_);
  pulseCount_ += direction;
  if (calibrating_) {
    ++calibrationPulses_;
  }
  portEXIT_CRITICAL_ISR(&pulseMux_);
}
