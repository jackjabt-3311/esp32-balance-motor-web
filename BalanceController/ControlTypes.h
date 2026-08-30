#pragma once
#include <Arduino.h>

constexpr uint16_t MAX_TARGET_RPM = 300;
constexpr uint32_t RAMP_DOWN_MS = 2000;
constexpr uint32_t HELMET_TIMEOUT_MS = 500;
constexpr uint32_t PEOPLE_STABLE_MS = 1000;

enum class MotorState : uint8_t {
  LOCKED, READY, ARMED, RUNNING, RAMPING_DOWN, FAULT, ESTOP
};

struct CommandResult {
  bool ok;
  const char* reason;
};

const char* motorStateName(MotorState state);
