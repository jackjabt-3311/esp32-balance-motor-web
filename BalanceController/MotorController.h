#pragma once

#include <Arduino.h>

#include "ControlTypes.h"
#include "SafetyGate.h"

class MotorController {
 public:
  static constexpr uint8_t RPWM_PIN = 25;
  static constexpr uint8_t LPWM_PIN = 26;
  static constexpr uint8_t R_EN_PIN = 27;
  static constexpr uint8_t L_EN_PIN = 14;
  static constexpr uint32_t PID_PERIOD_MS = 50;
  static constexpr float KP = 0.8f;
  static constexpr float KI = 0.15f;
  static constexpr float KD = 0.0f;
  static constexpr uint32_t STALL_TIMEOUT_MS = 1000;
  static constexpr uint16_t STALL_MIN_TARGET_RPM = 20;
  static constexpr float STALL_MIN_PWM_PERCENT = 25.0f;
  static constexpr float STALL_MAX_ACTUAL_RPM = 5.0f;

  void begin();
  void tick(uint32_t nowMs, const SafetySnapshot& safety, float actualRpm);
  CommandResult requestOn(const SafetySnapshot& safety);
  CommandResult requestOff(uint32_t nowMs);
  CommandResult requestEstop();
  CommandResult requestReset(const SafetySnapshot& safety);
  CommandResult setTargetRpm(uint16_t rpm);
  MotorState state() const;
  uint16_t targetRpm() const;
  float pwmPercent() const;
  const char* faultReason() const;

 private:
  static constexpr uint32_t PWM_FREQUENCY_HZ = 20000;
  static constexpr uint8_t PWM_RESOLUTION_BITS = 8;
  static constexpr uint8_t RPWM_CHANNEL = 0;
  static constexpr uint8_t LPWM_CHANNEL = 1;

  void attachPwm();
  void writePwm(uint8_t pin, uint8_t duty);
  void applyDuty(float percent);
  void forceStopOutputs();
  void enableDriver();
  void resetControllerHistory();
  void startRamp(uint32_t nowMs);
  void tickRamp(uint32_t nowMs, const SafetySnapshot& safety);
  void tickPid(uint32_t nowMs, float actualRpm);
  void tickStall(uint32_t nowMs, float actualRpm);
  void latchStallFault();

  MotorState state_ = MotorState::LOCKED;
  uint16_t targetRpm_ = 0;
  float pwmPercent_ = 0.0f;
  const char* faultReason_ = "";
  float integral_ = 0.0f;
  float previousError_ = 0.0f;
  uint32_t lastPidMs_ = 0;
  bool pidTimingStarted_ = false;
  uint32_t rampStartedMs_ = 0;
  float rampStartDuty_ = 0.0f;
  uint32_t stallStartedMs_ = 0;
  bool stallTiming_ = false;
};
