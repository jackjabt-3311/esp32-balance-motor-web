#include "MotorController.h"

#include <math.h>

#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif
#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

namespace {
float clampPercent(float value) {
  if (value < 0.0f) return 0.0f;
  if (value > 100.0f) return 100.0f;
  return value;
}
}  // namespace

void MotorController::begin() {
  pinMode(RPWM_PIN, OUTPUT);
  pinMode(LPWM_PIN, OUTPUT);
  pinMode(R_EN_PIN, OUTPUT);
  pinMode(L_EN_PIN, OUTPUT);
  digitalWrite(RPWM_PIN, LOW);
  digitalWrite(LPWM_PIN, LOW);
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
  attachPwm();
  forceStopOutputs();
  targetRpm_ = 0;
  faultReason_ = "";
  resetControllerHistory();
  state_ = MotorState::READY;
}

void MotorController::tick(uint32_t nowMs, const SafetySnapshot& safety, float actualRpm) {
  if (state_ == MotorState::ESTOP || state_ == MotorState::FAULT) return;
  if (state_ == MotorState::RAMPING_DOWN) {
    tickRamp(nowMs, safety);
    return;
  }
  if ((state_ == MotorState::ARMED || state_ == MotorState::RUNNING) && !safety.ready) {
    startRamp(nowMs);
    return;
  }
  if (state_ == MotorState::READY || state_ == MotorState::LOCKED) {
    state_ = safety.ready ? MotorState::READY : MotorState::LOCKED;
    return;
  }
  if (state_ != MotorState::RUNNING) return;
  tickPid(nowMs, actualRpm);
  tickStall(nowMs, actualRpm);
}

CommandResult MotorController::requestOn(const SafetySnapshot& safety) {
  if (state_ == MotorState::RAMPING_DOWN) return {false, "ramping"};
  if (state_ == MotorState::ESTOP) return {false, "estop_latched"};
  if (state_ == MotorState::FAULT) return {false, "fault_latched"};
  if (state_ != MotorState::READY) return {false, "invalid_state"};
  if (!safety.ready) {
    state_ = MotorState::LOCKED;
    return {false, "safety_locked"};
  }
  targetRpm_ = 0;
  applyDuty(0.0f);
  resetControllerHistory();
  enableDriver();
  state_ = MotorState::ARMED;
  return {true, "armed"};
}

CommandResult MotorController::requestOff(uint32_t nowMs) {
  if (state_ == MotorState::RAMPING_DOWN) return {false, "ramping"};
  if (state_ == MotorState::ESTOP) return {false, "estop_latched"};
  if (state_ == MotorState::FAULT) return {false, "fault_latched"};
  if (state_ != MotorState::ARMED && state_ != MotorState::RUNNING) {
    return {false, "invalid_state"};
  }
  startRamp(nowMs);
  return {true, "ramping"};
}

CommandResult MotorController::requestEstop() {
  forceStopOutputs();
  targetRpm_ = 0;
  resetControllerHistory();
  stallTiming_ = false;
  faultReason_ = "estop";
  state_ = MotorState::ESTOP;
  return {true, "estop"};
}

CommandResult MotorController::requestReset(const SafetySnapshot& safety) {
  if (state_ == MotorState::RAMPING_DOWN) return {false, "ramping"};
  if (pwmPercent_ > 0.0f) return {false, "output_active"};
  if (!safety.prerequisitesReady) {
    return {false, "reset_prerequisites_not_ready"};
  }
  forceStopOutputs();
  targetRpm_ = 0;
  resetControllerHistory();
  stallTiming_ = false;
  faultReason_ = "";
  state_ = MotorState::READY;
  return {true, "ready"};
}

CommandResult MotorController::setTargetRpm(uint16_t rpm) {
  if (rpm > MAX_TARGET_RPM) return {false, "rpm_out_of_range"};
  if (state_ == MotorState::RAMPING_DOWN) return {false, "ramping"};
  if (state_ == MotorState::ESTOP) return {false, "estop_latched"};
  if (state_ == MotorState::FAULT) return {false, "fault_latched"};
  if (state_ != MotorState::ARMED && state_ != MotorState::RUNNING) {
    return {false, "invalid_state"};
  }
  const bool wasRunning = state_ == MotorState::RUNNING;
  targetRpm_ = rpm;
  if (rpm == 0) {
    applyDuty(0.0f);
    resetControllerHistory();
    stallTiming_ = false;
    state_ = MotorState::ARMED;
    return {true, "armed"};
  }
  if (!wasRunning) resetControllerHistory();
  state_ = MotorState::RUNNING;
  return {true, "running"};
}

MotorState MotorController::state() const { return state_; }
uint16_t MotorController::targetRpm() const { return targetRpm_; }
float MotorController::pwmPercent() const { return pwmPercent_; }
const char* MotorController::faultReason() const { return faultReason_; }

void MotorController::attachPwm() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(RPWM_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcAttach(LPWM_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
#else
  ledcSetup(RPWM_CHANNEL, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(LPWM_CHANNEL, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcAttachPin(RPWM_PIN, RPWM_CHANNEL);
  ledcAttachPin(LPWM_PIN, LPWM_CHANNEL);
#endif
}

void MotorController::writePwm(uint8_t pin, uint8_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(pin, duty);
#else
  ledcWrite(pin == RPWM_PIN ? RPWM_CHANNEL : LPWM_CHANNEL, duty);
#endif
}

void MotorController::applyDuty(float percent) {
  pwmPercent_ = clampPercent(percent);
  const uint8_t rawDuty = static_cast<uint8_t>(lroundf(pwmPercent_ * 255.0f / 100.0f));
  writePwm(RPWM_PIN, rawDuty);
  writePwm(LPWM_PIN, 0);
}

void MotorController::forceStopOutputs() {
  writePwm(RPWM_PIN, 0);
  writePwm(LPWM_PIN, 0);
  digitalWrite(R_EN_PIN, LOW);
  digitalWrite(L_EN_PIN, LOW);
  pwmPercent_ = 0.0f;
}

void MotorController::enableDriver() {
  writePwm(LPWM_PIN, 0);
  digitalWrite(R_EN_PIN, HIGH);
  digitalWrite(L_EN_PIN, HIGH);
}

void MotorController::resetControllerHistory() {
  integral_ = 0.0f;
  previousError_ = 0.0f;
  lastPidMs_ = 0;
  pidTimingStarted_ = false;
}

void MotorController::startRamp(uint32_t nowMs) {
  rampStartedMs_ = nowMs;
  rampStartDuty_ = pwmPercent_;
  targetRpm_ = 0;
  stallTiming_ = false;
  resetControllerHistory();
  state_ = MotorState::RAMPING_DOWN;
}

void MotorController::tickRamp(uint32_t nowMs, const SafetySnapshot& safety) {
  const uint32_t elapsedMs = nowMs - rampStartedMs_;
  if (elapsedMs >= RAMP_DOWN_MS) {
    forceStopOutputs();
    targetRpm_ = 0;
    resetControllerHistory();
    state_ = safety.ready ? MotorState::READY : MotorState::LOCKED;
    return;
  }
  const float fraction = static_cast<float>(elapsedMs) / static_cast<float>(RAMP_DOWN_MS);
  applyDuty(rampStartDuty_ * (1.0f - fraction));
}

void MotorController::tickPid(uint32_t nowMs, float actualRpm) {
  if (!pidTimingStarted_) {
    pidTimingStarted_ = true;
    lastPidMs_ = nowMs;
    return;
  }
  const uint32_t elapsedMs = nowMs - lastPidMs_;
  if (elapsedMs < PID_PERIOD_MS) return;
  const float elapsedSeconds = static_cast<float>(elapsedMs) / 1000.0f;
  const float error = static_cast<float>(targetRpm_) - actualRpm;
  const float derivative = (error - previousError_) / elapsedSeconds;
  float candidateIntegral = integral_ + error * elapsedSeconds;
  if (candidateIntegral > 100.0f) candidateIntegral = 100.0f;
  if (candidateIntegral < 0.0f) candidateIntegral = 0.0f;
  const float candidateOutput =
      KP * error + KI * candidateIntegral + KD * derivative;
  const bool saturatingHigh = candidateOutput > 100.0f && error > 0.0f;
  const bool saturatingLow = candidateOutput < 0.0f && error < 0.0f;
  if (!saturatingHigh && !saturatingLow) {
    integral_ = candidateIntegral;
  }
  applyDuty(KP * error + KI * integral_ + KD * derivative);
  previousError_ = error;
  lastPidMs_ = nowMs;
}

void MotorController::tickStall(uint32_t nowMs, float actualRpm) {
  const bool stallCandidate = targetRpm_ >= STALL_MIN_TARGET_RPM &&
      pwmPercent_ >= STALL_MIN_PWM_PERCENT && actualRpm < STALL_MAX_ACTUAL_RPM;
  if (!stallCandidate) {
    stallTiming_ = false;
    return;
  }
  if (!stallTiming_) {
    stallTiming_ = true;
    stallStartedMs_ = nowMs;
    return;
  }
  if (nowMs - stallStartedMs_ >= STALL_TIMEOUT_MS) latchStallFault();
}

void MotorController::latchStallFault() {
  forceStopOutputs();
  targetRpm_ = 0;
  resetControllerHistory();
  stallTiming_ = false;
  faultReason_ = "encoder_stall";
  state_ = MotorState::FAULT;
}
