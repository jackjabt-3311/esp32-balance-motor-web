#include "SafetyGate.h"

void SafetyGate::noteHelmet(bool worn, uint32_t nowMs) {
  helmetSeen_ = true;
  helmetWorn_ = worn;
  lastHelmetMs_ = nowMs;
}

void SafetyGate::updatePeople(int people, uint32_t nowMs) {
  people_ = people;
  if (people == 1) {
    if (!onePersonTiming_) {
      onePersonTiming_ = true;
      onePersonSinceMs_ = nowMs;
    }
    return;
  }

  onePersonTiming_ = false;
}

SafetySnapshot SafetyGate::evaluate(
    uint32_t nowMs,
    bool encoderCalibrated,
    bool motorFault) const {
  const bool helmetFresh =
      helmetSeen_ && (nowMs - lastHelmetMs_ <= HELMET_TIMEOUT_MS);
  const bool onePersonStable =
      people_ == 1 && onePersonTiming_ &&
      (nowMs - onePersonSinceMs_ >= PEOPLE_STABLE_MS);
  // Keep the fault-excluded conditions available to reset handling.  `ready`
  // intentionally includes motorFault, so it cannot safely authorize clearing
  // that very latch.
  const bool prerequisitesReady =
      helmetFresh && helmetWorn_ && onePersonStable && encoderCalibrated;
  const bool ready = prerequisitesReady && !motorFault;

  const char* lockReason = "ready";
  if (!helmetFresh) {
    lockReason = "helmet_signal_lost";
  } else if (!helmetWorn_) {
    lockReason = "helmet_not_worn";
  } else if (people_ != 1) {
    lockReason = "people_not_one";
  } else if (!onePersonStable) {
    lockReason = "people_not_stable";
  } else if (!encoderCalibrated) {
    lockReason = "encoder_not_calibrated";
  } else if (motorFault) {
    lockReason = "motor_fault";
  }

  return {
      helmetWorn_, helmetFresh, onePersonStable, encoderCalibrated, motorFault,
      prerequisitesReady, ready, lockReason};
}
