#include "WebInterface.h"

#include <cstring>

#include <LittleFS.h>

namespace {
constexpr char ACTIVE_PAGE[] = "/index.html";
constexpr char TEMP_PAGE[] = "/index.tmp";
constexpr char BACKUP_PAGE[] = "/index.bak";
constexpr int HTTP_STATUS_OK = 200;
constexpr int HTTP_STATUS_BAD_REQUEST = 400;
constexpr int HTTP_STATUS_CONFLICT = 409;
const char* REQUEST_HEADERS[] = {"Content-Type"};

const char RECOVERY_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><title>Controller recovery</title></head>
<body><h1>Controller recovery</h1><p>No dashboard page is available.</p>
<form action="/upload" method="post" enctype="multipart/form-data">
<label>Dashboard HTML <input type="file" name="file" accept=".html" required></label>
<button type="submit">Upload</button></form></body></html>)HTML";
}  // namespace

WebInterface::WebInterface(const WebCallbacks& callbacks)
    : server_(80), callbacks_(callbacks) {}

void WebInterface::begin() {
  fsReady_ = LittleFS.begin(false);
  server_.collectHeaders(REQUEST_HEADERS, 1);

  server_.on("/", HTTP_GET, [this]() { handleRoot(); });
  server_.on("/data", HTTP_GET, [this]() { handleData(); });
  server_.on("/motor/on", HTTP_POST,
             [this]() { handleMotorCommand(callbacks_.motorOn); });
  server_.on("/motor/off", HTTP_POST,
             [this]() { handleMotorCommand(callbacks_.motorOff); });
  server_.on("/motor/rpm", HTTP_POST, [this]() { handleMotorRpm(); });
  server_.on("/motor/estop", HTTP_POST,
             [this]() { handleMotorCommand(callbacks_.motorEstop); });
  server_.on("/motor/reset", HTTP_POST,
             [this]() { handleMotorCommand(callbacks_.motorReset); });
  server_.on("/encoder/calibration/start", HTTP_POST,
             [this]() { handleCalibrationStart(); });
  server_.on("/encoder/calibration/finish", HTTP_POST,
             [this]() { handleCalibrationFinish(); });
  server_.on("/upload", HTTP_POST, [this]() { handleUploadComplete(); },
             [this]() { handleUploadData(); });
  server_.begin();
}

void WebInterface::handleClient() {
  server_.handleClient();
}

void WebInterface::handleData() {
  if (callbacks_.copySnapshot == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }
  DashboardSnapshot snapshot{};
  callbacks_.copySnapshot(snapshot);

  String json;
  json.reserve(384);
  json += "{\"lc1\":"; json += String(snapshot.lc1, 3);
  json += ",\"lc2\":"; json += String(snapshot.lc2, 3);
  json += ",\"lc3\":"; json += String(snapshot.lc3, 3);
  json += ",\"lc4\":"; json += String(snapshot.lc4, 3);
  json += ",\"x\":"; json += String(snapshot.x, 3);
  json += ",\"y\":"; json += String(snapshot.y, 3);
  json += ",\"people\":"; json += String(snapshot.people);
  json += ",\"helmet\":\""; json += jsonEscape(snapshot.helmet); json += "\"";
  json += ",\"motorAllowed\":"; json += snapshot.motorAllowed ? "true" : "false";
  json += ",\"lockReason\":\""; json += jsonEscape(snapshot.lockReason); json += "\"";
  json += ",\"motorState\":\""; json += jsonEscape(snapshot.motorState); json += "\"";
  json += ",\"targetRpm\":"; json += String(snapshot.targetRpm);
  json += ",\"actualRpm\":"; json += String(snapshot.actualRpm, 3);
  json += ",\"pwmPercent\":"; json += String(snapshot.pwmPercent, 3);
  json += ",\"encoderCalibrated\":"; json += snapshot.encoderCalibrated ? "true" : "false";
  json += ",\"pulsesPerRev\":"; json += String(snapshot.pulsesPerRev);
  json += ",\"fault\":\""; json += jsonEscape(snapshot.fault); json += "\"}";
  server_.send(HTTP_STATUS_OK, "application/json; charset=utf-8", json);
}

void WebInterface::handleRoot() {
  if (fsReady_ && LittleFS.exists(ACTIVE_PAGE)) {
    File page = LittleFS.open(ACTIVE_PAGE, FILE_READ);
    if (page) {
      server_.streamFile(page, "text/html; charset=utf-8");
      page.close();
      return;
    }
  }
  server_.send(HTTP_STATUS_OK, "text/html; charset=utf-8", RECOVERY_PAGE);
}

void WebInterface::handleMotorCommand(MotorCommandCallback command) {
  if (command == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }
  sendCommandResult(command());
}

void WebInterface::handleMotorRpm() {
  if (!server_.hasArg("value")) {
    sendJsonResult(HTTP_STATUS_BAD_REQUEST, false, "missing_rpm");
    return;
  }

  const String value = server_.arg("value");
  uint16_t rpm = 0;
  if (value.length() == 0 || !isStrictRpm(value, &rpm)) {
    sendJsonResult(HTTP_STATUS_BAD_REQUEST, false, "invalid_rpm");
    return;
  }
  if (callbacks_.motorRpm == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }
  sendCommandResult(callbacks_.motorRpm(rpm));
}

void WebInterface::handleCalibrationStart() {
  if (callbacks_.copySnapshot == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }
  DashboardSnapshot snapshot{};
  callbacks_.copySnapshot(snapshot);
  if (!isMotorFullyStopped(snapshot)) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "motor_not_stopped");
    return;
  }
  if (callbacks_.calibrationStart == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }
  sendCommandResult(callbacks_.calibrationStart());
}

void WebInterface::handleCalibrationFinish() {
  if (callbacks_.copySnapshot == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }
  DashboardSnapshot snapshot{};
  callbacks_.copySnapshot(snapshot);
  if (!isMotorFullyStopped(snapshot)) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "motor_not_stopped");
    return;
  }
  if (callbacks_.calibrationFinish == nullptr || callbacks_.savedPulsesPerRev == nullptr) {
    sendJsonResult(HTTP_STATUS_CONFLICT, false, "not_configured");
    return;
  }

  const CommandResult result = callbacks_.calibrationFinish();
  if (!result.ok) {
    sendCommandResult(result);
    return;
  }
  const uint32_t pulsesPerRev = callbacks_.savedPulsesPerRev();
  sendJsonResult(HTTP_STATUS_OK, true, result.reason, true, pulsesPerRev);
}

void WebInterface::handleUploadComplete() {
  if (!isMultipartUploadRequest()) {
    failUpload("invalid_upload_content_type");
  } else if (!uploadFailed_) {
    if (uploadStarted_ && uploadFileEnded_ && !uploadFileOpen_) {
      if (!promoteUpload()) {
        failUpload(uploadFailureReason_);
      } else {
        uploadSucceeded_ = true;
      }
    } else {
      failUpload("upload_incomplete");
    }
  }

  if (uploadSucceeded_) {
    sendJsonResult(HTTP_STATUS_OK, true, "upload_promoted");
  } else {
    sendJsonResult(HTTP_STATUS_BAD_REQUEST, false, uploadFailureReason_);
  }
  resetUploadRequest();
}

void WebInterface::handleUploadData() {
  if (!isMultipartUploadRequest()) {
    failUpload("invalid_upload_content_type");
    return;
  }
  HTTPUpload& upload = server_.upload();
  switch (upload.status) {
    case UPLOAD_FILE_START:
      if (uploadStarted_) {
        failUpload("multiple_files");
        return;
      }
      uploadStarted_ = true;
      uploadFileEnded_ = false;
      uploadBytes_ = 0;
      uploadFailed_ = false;
      uploadSucceeded_ = false;
      uploadFailureReason_ = "upload_incomplete";
      cleanupUploadTemp();
      if (!fsReady_) {
        failUpload("filesystem_unavailable");
      } else if (!isHtmlFilename(upload.filename)) {
        failUpload("invalid_file_type");
      } else {
        uploadFile_ = LittleFS.open(TEMP_PAGE, FILE_WRITE);
        if (!uploadFile_) {
          failUpload("temp_open_failed");
        } else {
          uploadFileOpen_ = true;
        }
      }
      break;

    case UPLOAD_FILE_WRITE:
      if (uploadFailed_ || !uploadFileOpen_) return;
      if (upload.currentSize > MAX_UPLOAD_BYTES - uploadBytes_) {
        failUpload("upload_too_large");
        return;
      }
      {
        const size_t written = uploadFile_.write(upload.buf, upload.currentSize);
        if (written != upload.currentSize) {
          failUpload("short_write");
          return;
        }
      }
      uploadBytes_ += upload.currentSize;
      break;

    case UPLOAD_FILE_END:
      if (uploadFailed_ || !uploadFileOpen_) return;
      uploadFile_.close();
      uploadFileOpen_ = false;
      uploadFileEnded_ = true;
      break;

    case UPLOAD_FILE_ABORTED:
      failUpload("upload_aborted");
      resetUploadRequest();
      break;

    default:
      break;
  }
}

void WebInterface::sendCommandResult(const CommandResult& result) {
  sendJsonResult(result.ok ? HTTP_STATUS_OK : HTTP_STATUS_CONFLICT, result.ok, result.reason);
}

void WebInterface::sendJsonResult(
    int status,
    bool ok,
    const char* reason,
    bool includePulsesPerRev,
    uint32_t pulsesPerRev) {
  String json = "{\"ok\":";
  json += ok ? "true" : "false";
  json += ",\"reason\":\"";
  json += jsonEscape(reason);
  json += "\"";
  if (includePulsesPerRev) {
    json += ",\"pulsesPerRev\":";
    json += String(pulsesPerRev);
  }
  json += "}";
  server_.send(status, "application/json; charset=utf-8", json);
}

void WebInterface::failUpload(const char* reason) {
  if (uploadFileOpen_) {
    uploadFile_.close();
    uploadFileOpen_ = false;
  }
  uploadFailed_ = true;
  uploadSucceeded_ = false;
  uploadFailureReason_ = reason;
  cleanupUploadTemp();
}

void WebInterface::cleanupUploadTemp() {
  if (uploadFileOpen_) {
    uploadFile_.close();
    uploadFileOpen_ = false;
  }
  if (fsReady_ && LittleFS.exists(TEMP_PAGE)) {
    LittleFS.remove("/index.tmp");
  }
}

void WebInterface::resetUploadRequest() {
  if (uploadFileOpen_) {
    uploadFile_.close();
  }
  uploadFileOpen_ = false;
  uploadBytes_ = 0;
  uploadStarted_ = false;
  uploadFileEnded_ = false;
  uploadFailed_ = false;
  uploadSucceeded_ = false;
  uploadFailureReason_ = "upload_incomplete";
}

bool WebInterface::promoteUpload() {
  if (LittleFS.exists(BACKUP_PAGE) && !LittleFS.remove("/index.bak")) {
    uploadFailureReason_ = "backup_remove_failed";
    return false;
  }

  bool activeMovedToBackup = false;
  if (LittleFS.exists(ACTIVE_PAGE)) {
    if (!LittleFS.rename("/index.html", "/index.bak")) {
      uploadFailureReason_ = "backup_create_failed";
      return false;
    }
    activeMovedToBackup = true;
  }

  if (!LittleFS.rename("/index.tmp", "/index.html")) {
    if (activeMovedToBackup) {
      if (!LittleFS.rename("/index.bak", "/index.html")) {
        uploadFailureReason_ = "rollback_failed";
        return false;
      }
    }
    uploadFailureReason_ = "promotion_failed";
    return false;
  }

  if (LittleFS.exists(BACKUP_PAGE)) {
    LittleFS.remove("/index.bak");
  }
  return true;
}

bool WebInterface::isStrictRpm(const String& value, uint16_t* rpm) {
  if (rpm == nullptr || value.length() == 0) return false;
  uint32_t parsed = 0;
  for (size_t index = 0; index < value.length(); ++index) {
    const char character = value.charAt(index);
    if (character < '0' || character > '9') return false;
    const uint8_t digit = static_cast<uint8_t>(character - '0');
    if (parsed > (MAX_TARGET_RPM - digit) / 10) return false;
    parsed = parsed * 10 + digit;
  }
  if (parsed > MAX_TARGET_RPM) return false;
  *rpm = static_cast<uint16_t>(parsed);
  return true;
}

bool WebInterface::isHtmlFilename(const String& filename) {
  String normalized = filename;
  normalized.toLowerCase();
  return normalized.endsWith(".html");
}

bool WebInterface::isMultipartUploadRequest() {
  if (!server_.hasHeader("Content-Type")) return false;
  const String contentType = server_.header("Content-Type");
  if (!contentType.startsWith("multipart/")) return false;
  String mediaType = contentType;
  const int parametersAt = mediaType.indexOf(';');
  if (parametersAt >= 0) {
    mediaType = mediaType.substring(0, static_cast<unsigned int>(parametersAt));
  }
  mediaType.trim();
  return mediaType == "multipart/form-data";
}

bool WebInterface::isMotorFullyStopped(const DashboardSnapshot& snapshot) {
  if (snapshot.pwmPercent != 0.0f) return false;
  const char* state = snapshot.motorState == nullptr ? "" : snapshot.motorState;
  return strcmp(state, "armed") != 0 && strcmp(state, "running") != 0 &&
         strcmp(state, "ramping_down") != 0;
}

String WebInterface::jsonEscape(const char* value) {
  String escaped;
  if (value == nullptr) return escaped;
  for (const unsigned char* character = reinterpret_cast<const unsigned char*>(value);
       *character != '\0'; ++character) {
    switch (*character) {
      case '"': escaped += "\\\""; break;
      case '\\': escaped += "\\\\"; break;
      case '\b': escaped += "\\b"; break;
      case '\f': escaped += "\\f"; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default:
        if (*character < 0x20) {
          char encoded[7];
          snprintf(encoded, sizeof(encoded), "\\u%04x", *character);
          escaped += encoded;
        } else {
          escaped += static_cast<char>(*character);
        }
        break;
    }
  }
  return escaped;
}
