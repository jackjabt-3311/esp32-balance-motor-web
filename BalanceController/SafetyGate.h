#pragma once

#include "ControlTypes.h"

struct SafetySnapshot {
  bool helmetWorn;
  bool helmetFresh;
  bool onePersonStable;
  bool encoderCalibrated;
  bool motorFault;
  bool ready;
  const char* lockReason;
};

class SafetyGate {
 public:
  void noteHelmet(bool worn, uint32_t nowMs);
  void updatePeople(int people, uint32_t nowMs);
  SafetySnapshot evaluate(
      uint32_t nowMs,
      bool encoderCalibrated,
      bool motorFault) const;

 private:
  bool helmetWorn_ = false;
  bool helmetSeen_ = false;
  uint32_t lastHelmetMs_ = 0;
  int people_ = 0;
  bool onePersonTiming_ = false;
  uint32_t onePersonSinceMs_ = 0;
};
