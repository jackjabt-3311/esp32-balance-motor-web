#pragma once

#include <Arduino.h>
#include <FS.h>
#include <WebServer.h>

#include "ControlTypes.h"

struct DashboardSnapshot {
  float lc1;
  float lc2;
  float lc3;
  float lc4;
  float x;
  float y;
  int people;
  const char* helmet;
  bool motorAllowed;
  const char* lockReason;
  const char* motorState;
  uint16_t targetRpm;
  float actualRpm;
  float pwmPercent;
  bool encoderCalibrated;
  uint32_t pulsesPerRev;
  const char* fault;
};

using SnapshotCopyCallback = void (*)(DashboardSnapshot& snapshot);
using MotorCommandCallback = CommandResult (*)();
using RpmCommandCallback = CommandResult (*)(uint16_t rpm);
using CalibrationCommandCallback = CommandResult (*)();
using PulsesPerRevCallback = uint32_t (*)();

struct WebCallbacks {
  SnapshotCopyCallback copySnapshot;
  MotorCommandCallback motorOn;
  MotorCommandCallback motorOff;
  RpmCommandCallback motorRpm;
  MotorCommandCallback motorEstop;
  MotorCommandCallback motorReset;
  CalibrationCommandCallback calibrationStart;
  CalibrationCommandCallback calibrationFinish;
  PulsesPerRevCallback savedPulsesPerRev;
};

class WebInterface {
 public:
  static constexpr size_t MAX_UPLOAD_BYTES = 262144;

  explicit WebInterface(const WebCallbacks& callbacks);

  void begin();
  void handleClient();

 private:
  void handleData();
  void handleRoot();
  void handleMotorCommand(MotorCommandCallback command);
  void handleMotorRpm();
  void handleCalibrationStart();
  void handleCalibrationFinish();
  void handleUploadComplete();
  void handleUploadData();

  void sendCommandResult(const CommandResult& result);
  void sendJsonResult(
      int status,
      bool ok,
      const char* reason,
      bool includePulsesPerRev = false,
      uint32_t pulsesPerRev = 0);
  void failUpload(const char* reason);
  void cleanupUploadTemp();
  void resetUploadRequest();
  bool promoteUpload();

  static bool isStrictRpm(const String& value, uint16_t* rpm);
  static bool isHtmlFilename(const String& filename);
  bool isMultipartUploadRequest();
  static bool isMotorFullyStopped(const DashboardSnapshot& snapshot);
  static String jsonEscape(const char* value);

  WebServer server_;
  WebCallbacks callbacks_;
  bool fsReady_ = false;
  File uploadFile_;
  size_t uploadBytes_ = 0;
  bool uploadStarted_ = false;
  bool uploadFileEnded_ = false;
  bool uploadFileOpen_ = false;
  bool uploadFailed_ = false;
  bool uploadSucceeded_ = false;
  const char* uploadFailureReason_ = "upload_incomplete";
};
