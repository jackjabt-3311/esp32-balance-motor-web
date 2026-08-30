#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "ControlTypes.h"

class EncoderService {
 public:
  static constexpr uint8_t ENCA_PIN = 32;
  static constexpr uint8_t ENCB_PIN = 33;
  static constexpr uint32_t RPM_SAMPLE_MS = 100;

  void begin();
  void tick(uint32_t nowMs);
  float rpm() const;
  bool isCalibrated() const;
  bool isCalibrating() const;
  // The caller must ensure the motor is stopped before starting calibration.
  CommandResult startCalibration();
  CommandResult finishCalibration();
  uint32_t pulsesPerRevolution() const;

 private:
  static void IRAM_ATTR onEncoderA();
  static volatile int32_t pulseCount_;
  static volatile uint32_t calibrationPulses_;
  static volatile bool calibrating_;
  static portMUX_TYPE pulseMux_;

  Preferences preferences_;
  uint32_t pulsesPerRevolution_ = 0;
  uint32_t lastSampleMs_ = 0;
  int32_t lastPulseCount_ = 0;
  float rpm_ = 0.0f;
};
