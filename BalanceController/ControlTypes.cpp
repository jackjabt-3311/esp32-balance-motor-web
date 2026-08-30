#include "ControlTypes.h"

const char* motorStateName(MotorState state) {
  switch (state) {
    case MotorState::LOCKED: return "locked";
    case MotorState::READY: return "ready";
    case MotorState::ARMED: return "armed";
    case MotorState::RUNNING: return "running";
    case MotorState::RAMPING_DOWN: return "ramping_down";
    case MotorState::FAULT: return "fault";
    case MotorState::ESTOP: return "estop";
  }
  return "fault";
}

