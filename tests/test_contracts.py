import hashlib
import math
import os
from pathlib import Path
import re
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_MODEL_SHA256 = "f6e136bd80e5003a7465616237539f3f37d3affbc3b7192fce4df2dd0182dd54"
FORBIDDEN_LOCAL_PATH_PREFIX = "c:" + "/users/"

PLANNED_FILES = (
    "BalanceController/ControlTypes.h",
    "BalanceController/ControlTypes.cpp",
    "BalanceController/LoadFeatureModel.h",
    "BalanceController/model.c",
    "BalanceController/BalanceController.ino",
    "BalanceController/LocalConfig.example.h",
    "BalanceController/SafetyGate.h",
    "BalanceController/SafetyGate.cpp",
    "BalanceController/EncoderService.h",
    "BalanceController/EncoderService.cpp",
    "BalanceController/MotorController.h",
    "BalanceController/MotorController.cpp",
    "BalanceController/HelmetReceiver.h",
    "BalanceController/HelmetReceiver.cpp",
    "BalanceController/WebInterface.h",
    "BalanceController/WebInterface.cpp",
    "BalanceController/data/index.html",
    "tests/test_contracts.py",
    "tests/test_state_model.py",
    "HelmetTransmitter/HelmetTransmitter.ino",
    "HelmetTransmitter/LocalConfig.example.h",
)
FUTURE_ROUTES = (
    "/data", "/motor/on", "/motor/off", "/motor/rpm", "/motor/estop",
    "/motor/reset", "/encoder/calibration/start", "/encoder/calibration/finish",
    "/upload",
)


def esp32_cpp14_compiler():
    """Return the installed ESP32 compiler used for header-only constexpr checks."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    if not local_app_data:
        raise RuntimeError("LOCALAPPDATA is required to locate the installed ESP32 compiler")
    tool_root = Path(local_app_data) / "Arduino15" / "packages" / "esp32" / "tools" / "esp-x32"
    candidates = sorted(tool_root.glob("*/bin/xtensa-esp32-elf-g++.exe"))
    if not candidates:
        raise RuntimeError("installed ESP32 xtensa C++ compiler was not found")
    return candidates[-1]


class UploadLifecycleReference:
    """Executable request lifecycle for the recoverable upload boundary."""

    def __init__(self):
        self.active = "old-page"
        self.backup = None
        self.temp = None
        self.started = False
        self.ended = False
        self.failed = False
        self.reason = "upload_incomplete"
        self.upload_reads = 0
        self.temp_content = None

    @staticmethod
    def is_multipart(content_type):
        if content_type is None:
            return False
        media_type = content_type.split(";", 1)[0].strip()
        return media_type.startswith("multipart/") and media_type == "multipart/form-data"

    def start_file(self, content_type):
        if not self.is_multipart(content_type):
            self.failed, self.reason = True, "invalid_upload_content_type"
            return
        self.upload_reads += 1
        if self.started:
            self.failed, self.reason, self.temp = True, "multiple_files", None
            return
        self.started, self.temp = True, "new-page"
        self.temp_content = "<!doctype html><html><body>new page</body></html>"

    def end_file(self):
        if not self.failed and self.started:
            self.ended = True

    def abort(self):
        self.temp = self.temp_content = None
        self.started = self.ended = self.failed = False
        self.reason = "upload_incomplete"

    def finalize(self, content_type):
        if not self.is_multipart(content_type):
            self.failed, self.reason = True, "invalid_upload_content_type"
        if not self.failed and self.started and self.ended and self.temp_content == "":
            self.failed, self.reason = True, "empty_html"
        if not self.failed and self.started and self.ended and not self.is_valid_html(self.temp_content):
            self.failed, self.reason = True, "invalid_html"
        if not self.failed and self.started and self.ended:
            self.backup, self.active, self.temp = self.active, self.temp, None
            self.temp_content = None
            self.backup = None
            status, reason = 200, "upload_promoted"
        else:
            self.temp = self.temp_content = None
            status, reason = 400, self.reason
        self.started = self.ended = self.failed = False
        self.reason = "upload_incomplete"
        return status, reason

    @staticmethod
    def is_valid_html(content):
        if not content:
            return False
        document = content.lstrip(" \t\r\n\f").lower()
        return document.startswith("<!doctype html") or document.startswith("<html")


class SharedContractTests(unittest.TestCase):
    def test_all_planned_files_exist(self):
        missing = [path for path in PLANNED_FILES if not (ROOT / path).is_file()]
        self.assertEqual(missing, [])

    def test_shared_constants_are_present_with_exact_tokens(self):
        source = "\n".join(path.read_text(encoding="utf-8") for path in ROOT.rglob("*.h"))
        self.assertIn("constexpr uint16_t MAX_TARGET_RPM = 300;", source)
        self.assertIn("constexpr uint32_t RAMP_DOWN_MS = 2000;", source)
        self.assertIn("constexpr uint32_t HELMET_TIMEOUT_MS = 500;", source)
        self.assertIn("constexpr uint32_t PEOPLE_STABLE_MS = 1000;", source)

    def test_model_copy_is_byte_identical_to_supplied_classifier(self):
        copied_hash = hashlib.sha256((ROOT / "BalanceController/model.c").read_bytes()).hexdigest()
        self.assertEqual(copied_hash, EXPECTED_MODEL_SHA256)

    def test_state_names_cover_all_json_values(self):
        source = (ROOT / "BalanceController/ControlTypes.cpp").read_text(encoding="utf-8")
        for state_name in ("locked", "ready", "armed", "running", "ramping_down", "fault", "estop"):
            self.assertIn(f'"{state_name}"', source)

    def test_safety_gate_declares_required_public_interface(self):
        header = (ROOT / "BalanceController/SafetyGate.h").read_text(encoding="utf-8")
        for field in (
            r"bool\s+helmetWorn\s*;",
            r"bool\s+helmetFresh\s*;",
            r"bool\s+onePersonStable\s*;",
            r"bool\s+encoderCalibrated\s*;",
            r"bool\s+motorFault\s*;",
            r"bool\s+ready\s*;",
            r"const\s+char\*\s+lockReason\s*;",
        ):
            self.assertRegex(header, field)
        for signature in (
            r"struct\s+SafetySnapshot\s*\{",
            r"void\s+noteHelmet\s*\(\s*bool\s+worn\s*,\s*uint32_t\s+nowMs\s*\)",
            r"void\s+updatePeople\s*\(\s*int\s+people\s*,\s*uint32_t\s+nowMs\s*\)",
            r"SafetySnapshot\s+evaluate\s*\(\s*uint32_t\s+nowMs\s*,\s*bool\s+encoderCalibrated\s*,\s*bool\s+motorFault\s*\)\s*const",
        ):
            self.assertRegex(header, signature)

    def test_safety_gate_uses_shared_time_constants_and_all_lock_reasons(self):
        source = (ROOT / "BalanceController/SafetyGate.cpp").read_text(encoding="utf-8")
        self.assertIn("HELMET_TIMEOUT_MS", source)
        self.assertIn("PEOPLE_STABLE_MS", source)
        for reason in (
            "helmet_signal_lost",
            "helmet_not_worn",
            "people_not_one",
            "people_not_stable",
            "encoder_not_calibrated",
            "motor_fault",
            "ready",
        ):
            self.assertIn(f'"{reason}"', source)

    def test_encoder_service_declares_required_pins_and_sampling_period(self):
        header = (ROOT / "BalanceController/EncoderService.h").read_text(encoding="utf-8")
        for token in (
            "static constexpr uint8_t ENCA_PIN = 32;",
            "static constexpr uint8_t ENCB_PIN = 33;",
            "static constexpr uint32_t RPM_SAMPLE_MS = 100;",
        ):
            self.assertIn(token, header)

    def test_encoder_service_uses_preferences_critical_sections_and_rising_interrupt(self):
        source = (ROOT / "BalanceController/EncoderService.cpp").read_text(encoding="utf-8")
        for token in (
            'begin("encoder", false)',
            'getUInt("ppr", 0)',
            'putUInt("ppr",',
            "portENTER_CRITICAL_ISR(&pulseMux_)",
            "portEXIT_CRITICAL_ISR(&pulseMux_)",
            "portENTER_CRITICAL(&pulseMux_)",
            "portEXIT_CRITICAL(&pulseMux_)",
            "attachInterrupt(digitalPinToInterrupt(ENCA_PIN), onEncoderA, RISING)",
        ):
            self.assertIn(token, source)

    def test_encoder_service_uses_absolute_rpm_formula_and_calibration_bounds(self):
        source = (ROOT / "BalanceController/EncoderService.cpp").read_text(encoding="utf-8")
        for token in (
            "abs(pulses)",
            "60000.0f",
            "static_cast<float>(elapsedMs) * static_cast<float>(pulsesPerRevolution_)",
            "RPM_SAMPLE_MS",
        ):
            self.assertIn(token, source)
        finish_body = source.split("CommandResult EncoderService::finishCalibration()", 1)[1].split(
            "uint32_t EncoderService::pulsesPerRevolution()", 1
        )[0]
        self.assertIn("calibrationPulses < 1", finish_body)
        self.assertIn("calibrationPulses > 100000", finish_body)
        self.assertNotIn("calibrationPulses_ <", finish_body)
        self.assertNotIn("calibrationPulses_ >", finish_body)

    def test_encoder_service_rebases_after_saved_calibration_and_checks_persistence(self):
        source = (ROOT / "BalanceController/EncoderService.cpp").read_text(encoding="utf-8")
        finish_body = source.split("CommandResult EncoderService::finishCalibration()", 1)[1].split(
            "uint32_t EncoderService::pulsesPerRevolution()", 1
        )[0]
        for token in (
            'if (!preferences_.begin("encoder", false))',
            'preferences_.putUInt("ppr", calibrationPulses)',
            "if (bytesWritten == 0)",
            "lastPulseCount_ = pulseCount_",
            "lastSampleMs_ = millis()",
            "rpm_ = 0.0f",
            '"persistence_failed"',
        ):
            self.assertIn(token, finish_body)
        self.assertLess(finish_body.index("lastPulseCount_ = pulseCount_"), finish_body.index("if (calibrationPulses < 1"))
        self.assertLess(finish_body.index("lastSampleMs_ = millis()"), finish_body.index("if (calibrationPulses < 1"))

    def test_encoder_service_has_all_calibration_result_reasons(self):
        source = (ROOT / "BalanceController/EncoderService.cpp").read_text(encoding="utf-8")
        for reason in (
            "calibration_started",
            "already_calibrating",
            "not_calibrating",
            "invalid_pulse_count",
            "calibration_saved",
            "persistence_failed",
        ):
            self.assertIn(f'"{reason}"', source)

    def test_motor_controller_declares_required_interface_and_hardware(self):
        header = (ROOT / "BalanceController/MotorController.h").read_text(encoding="utf-8")
        for token in (
            "static constexpr uint8_t RPWM_PIN = 25;",
            "static constexpr uint8_t LPWM_PIN = 26;",
            "static constexpr uint8_t R_EN_PIN = 27;",
            "static constexpr uint8_t L_EN_PIN = 14;",
            "static constexpr uint32_t PID_PERIOD_MS = 50;",
            "static constexpr float KP = 0.8f;",
            "static constexpr float KI = 0.15f;",
            "static constexpr float KD = 0.0f;",
        ):
            self.assertIn(token, header)
        for signature in (
            r"void\s+begin\s*\(\s*\)",
            r"void\s+tick\s*\(\s*uint32_t\s+nowMs\s*,\s*const\s+SafetySnapshot&\s+safety\s*,\s*float\s+actualRpm\s*\)",
            r"CommandResult\s+requestOn\s*\(\s*const\s+SafetySnapshot&\s+safety\s*\)",
            r"CommandResult\s+requestOff\s*\(\s*uint32_t\s+nowMs\s*\)",
            r"CommandResult\s+requestEstop\s*\(\s*\)",
            r"CommandResult\s+requestReset\s*\(\s*const\s+SafetySnapshot&\s+safety\s*\)",
            r"CommandResult\s+setTargetRpm\s*\(\s*uint16_t\s+rpm\s*\)",
        ):
            self.assertRegex(header, signature)

    def test_motor_boot_sequence_forces_outputs_low_before_ready(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        begin_body = source.split("void MotorController::begin()", 1)[1].split(
            "void MotorController::tick", 1
        )[0]
        ready_at = begin_body.index("MotorState::READY")
        for token in (
            "digitalWrite(RPWM_PIN, LOW)",
            "digitalWrite(LPWM_PIN, LOW)",
            "digitalWrite(R_EN_PIN, LOW)",
            "digitalWrite(L_EN_PIN, LOW)",
        ):
            self.assertLess(begin_body.index(token), ready_at)

    def test_motor_output_is_one_direction_and_immediate_stop_disables_both_enables(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        lpwm_arguments = re.findall(r"writePwm\s*\(\s*LPWM_PIN\s*,\s*([^)]*)\)", source)
        self.assertGreater(len(lpwm_arguments), 0)
        self.assertEqual({argument.strip() for argument in lpwm_arguments}, {"0"})
        stop_body = source.split("void MotorController::forceStopOutputs()", 1)[1].split(
            "void MotorController::enableDriver", 1
        )[0]
        for token in (
            "writePwm(RPWM_PIN, 0)",
            "writePwm(LPWM_PIN, 0)",
            "digitalWrite(R_EN_PIN, LOW)",
            "digitalWrite(L_EN_PIN, LOW)",
        ):
            self.assertIn(token, stop_body)

    def test_motor_contains_ramp_pid_stall_and_latched_state_branches(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        header = (ROOT / "BalanceController/MotorController.h").read_text(encoding="utf-8")
        for token in (
            "RAMP_DOWN_MS",
            "PID_PERIOD_MS",
            "STALL_TIMEOUT_MS = 1000",
            "STALL_MIN_TARGET_RPM = 20",
            "STALL_MIN_PWM_PERCENT = 25.0f",
            "STALL_MAX_ACTUAL_RPM = 5.0f",
            "MotorState::RAMPING_DOWN",
            "MotorState::FAULT",
            "MotorState::ESTOP",
            '"encoder_stall"',
        ):
            self.assertIn(token, source + header)
        self.assertIn("nowMs -", source)

    def test_motor_supports_arduino_esp32_ledc_v2_and_v3(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        for token in (
            "ESP_ARDUINO_VERSION_MAJOR >= 3",
            "ledcAttach(",
            "ledcSetup(",
            "ledcAttachPin(",
            "ledcWrite(",
        ):
            self.assertIn(token, source)

    def test_motor_reset_rejects_ramping_before_zero_output_check(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        reset_body = source.split("CommandResult MotorController::requestReset", 1)[1].split(
            "CommandResult MotorController::setTargetRpm", 1
        )[0]
        ramp_check = 'state_ == MotorState::RAMPING_DOWN'
        output_check = 'pwmPercent_ > 0.0f'
        self.assertIn(ramp_check, reset_body)
        self.assertIn('return {false, "ramping"}', reset_body)
        self.assertLess(reset_body.index(ramp_check), reset_body.index(output_check))

    def test_reset_requires_non_motor_safety_prerequisites_before_clearing_latch(self):
        header = (ROOT / "BalanceController/SafetyGate.h").read_text(encoding="utf-8")
        safety = (ROOT / "BalanceController/SafetyGate.cpp").read_text(encoding="utf-8")
        motor = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        reset_body = motor.split("CommandResult MotorController::requestReset", 1)[1].split(
            "CommandResult MotorController::setTargetRpm", 1
        )[0]
        self.assertIn("bool prerequisitesReady", header)
        self.assertIn("const bool prerequisitesReady", safety)
        self.assertIn("!safety.prerequisitesReady", reset_body)
        self.assertIn('return {false, "reset_prerequisites_not_ready"}', reset_body)
        self.assertLess(reset_body.index("!safety.prerequisitesReady"), reset_body.index("faultReason_ = \"\""))

    def test_motor_pid_uses_conditional_integration_to_prevent_windup(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        pid_body = source.split("void MotorController::tickPid", 1)[1].split(
            "void MotorController::tickStall", 1
        )[0]
        for token in (
            "candidateIntegral",
            "candidateOutput",
            "saturatingHigh",
            "saturatingLow",
            "!saturatingHigh && !saturatingLow",
        ):
            self.assertIn(token, pid_body)
        self.assertLess(pid_body.index("candidateOutput"), pid_body.index("!saturatingHigh && !saturatingLow"))
        self.assertLess(pid_body.index("!saturatingHigh && !saturatingLow"), pid_body.index("integral_ = candidateIntegral"))

    def test_motor_pid_explicitly_bounds_stored_integral_to_duty_range(self):
        source = (ROOT / "BalanceController/MotorController.cpp").read_text(encoding="utf-8")
        pid_body = source.split("void MotorController::tickPid", 1)[1].split(
            "void MotorController::tickStall", 1
        )[0]
        self.assertIn("candidateIntegral > 100.0f", pid_body)
        self.assertIn("candidateIntegral < 0.0f", pid_body)
        self.assertIn("candidateIntegral = 100.0f", pid_body)
        self.assertIn("candidateIntegral = 0.0f", pid_body)
        self.assertLess(pid_body.index("candidateIntegral = 100.0f"), pid_body.index("candidateOutput"))
        self.assertLess(pid_body.index("integral_ = candidateIntegral"), pid_body.index("applyDuty("))

    def test_repository_text_excludes_local_absolute_paths(self):
        result = subprocess.run(
            ["git", "ls-files", "-z"],
            cwd=ROOT,
            check=True,
            capture_output=True,
        )
        tracked_paths = [Path(path) for path in result.stdout.decode("utf-8").split("\0") if path]
        scanned_paths = set()
        violations = []
        for relative_path in tracked_paths:
            if relative_path.as_posix() == "BalanceController/model.c":
                continue
            path = ROOT / relative_path
            if not path.is_file():
                continue
            try:
                text = path.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                continue
            scanned_paths.add(relative_path.as_posix())
            normalized_text = text.replace(chr(92), "/").lower()
            if FORBIDDEN_LOCAL_PATH_PREFIX in normalized_text:
                violations.append(relative_path.as_posix())
        self.assertIn(".gitignore", scanned_paths)
        self.assertEqual(violations, [])

    def test_local_path_scan_normalizes_slashes_and_case(self):
        variants = (
            "C:" + "/Users/example/file",
            "c:" + "/users/example/file",
            "C:" + chr(92) + "USERS" + chr(92) + "example" + chr(92) + "file",
        )
        for text in variants:
            normalized_text = text.replace(chr(92), "/").lower()
            self.assertIn(FORBIDDEN_LOCAL_PATH_PREFIX, normalized_text)

    def test_helmet_receiver_cleans_callback_when_registration_fails(self):
        source = (ROOT / "BalanceController/HelmetReceiver.cpp").read_text(encoding="utf-8")
        registration_failure = re.search(
            r"if\s*\(\s*esp_now_register_recv_cb\(onReceive\)\s*!=\s*ESP_OK\s*\)\s*\{(?P<body>.*?)\}",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(registration_failure)
        self.assertIn("callback_ = nullptr;", registration_failure.group("body"))
        self.assertRegex(registration_failure.group("body"), r"return\s+false\s*;")

    def test_helmet_receiver_validates_messages_and_forwards_wear_state(self):
        header = (ROOT / "BalanceController/HelmetReceiver.h").read_text(encoding="utf-8")
        source = (ROOT / "BalanceController/HelmetReceiver.cpp").read_text(encoding="utf-8")
        self.assertIn("constexpr uint8_t WIFI_CHANNEL = 1;", header)
        for signature in (
            r"struct\s+HelmetMessage\s*\{\s*bool\s+isTriggered\s*;\s*\}",
            r"using\s+HelmetUpdateCallback\s*=\s*void\s*\(\*\)\s*\(\s*bool\s+worn\s*,\s*uint32_t\s+nowMs\s*\)",
            r"bool\s+begin\s*\(\s*HelmetUpdateCallback\s+callback\s*\)",
        ):
            self.assertRegex(header, signature)
        for token in (
            "data == nullptr",
            "len != sizeof(HelmetMessage)",
            "memcpy(&message, data, sizeof(message))",
            "callback_(message.isTriggered, millis())",
            "esp_now_init()",
            "esp_now_register_recv_cb",
        ):
            self.assertIn(token, source)

    def test_helmet_receiver_supports_arduino_esp32_v2_and_v3_callbacks(self):
        source = (ROOT / "BalanceController/HelmetReceiver.cpp").read_text(encoding="utf-8")
        for token in (
            "ESP_ARDUINO_VERSION_MAJOR >= 3",
            "const esp_now_recv_info_t*",
            "const uint8_t* mac",
            "int len",
        ):
            self.assertIn(token, source)

    def test_helmet_transmitter_keeps_three_sensor_wear_logic_and_interval(self):
        source = (ROOT / "HelmetTransmitter/HelmetTransmitter.ino").read_text(encoding="utf-8")
        for token in (
            "SENSOR_1_PIN = 21",
            "SENSOR_2_PIN = 35",
            "SENSOR_3_PIN = 33",
            "pinMode(SENSOR_1_PIN, INPUT)",
            "pinMode(SENSOR_2_PIN, INPUT)",
            "pinMode(SENSOR_3_PIN, INPUT)",
            "SENSOR_ACTIVE_MS = 5000",
            "SEND_INTERVAL_MS = 100",
            "sensor1Active && sensor2Active && sensor3Active",
            "nowMs - lastSendMs >= SEND_INTERVAL_MS",
            "nowMs - sensor1LastHighMs <= SENSOR_ACTIVE_MS",
            "nowMs - sensor2LastHighMs <= SENSOR_ACTIVE_MS",
            "nowMs - sensor3LastHighMs <= SENSOR_ACTIVE_MS",
        ):
            self.assertIn(token, source)

    def test_helmet_transmitter_configures_matching_channel_before_esp_now(self):
        receiver = (ROOT / "BalanceController/HelmetReceiver.h").read_text(encoding="utf-8")
        source = (ROOT / "HelmetTransmitter/HelmetTransmitter.ino").read_text(encoding="utf-8")
        example = (ROOT / "HelmetTransmitter/LocalConfig.example.h").read_text(encoding="utf-8")
        ignored = (ROOT / ".gitignore").read_text(encoding="utf-8")
        self.assertIn("WIFI_CHANNEL = 1", source)
        self.assertIn("WiFi.mode(WIFI_STA)", source)
        self.assertIn("esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE)", source)
        self.assertLess(source.index("esp_wifi_set_channel"), source.index("esp_now_init"))
        self.assertIn("peerInfo.channel = WIFI_CHANNEL", source)
        self.assertIn("memset(&peerInfo, 0, sizeof(peerInfo))", source)
        self.assertIn("receiverMac", source)
        self.assertIn("HelmetTransmitter/LocalConfig.h", ignored)
        self.assertNotIn(FORBIDDEN_LOCAL_PATH_PREFIX, example.lower())
        self.assertIn("WIFI_CHANNEL = 1", receiver)

    def test_main_ap_credentials_are_local_and_not_tracked(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        example = (ROOT / "BalanceController/LocalConfig.example.h").read_text(encoding="utf-8")
        ignored = (ROOT / ".gitignore").read_text(encoding="utf-8")
        tracked_text = "\n".join(
            path.read_text(encoding="utf-8", errors="ignore")
            for path in ROOT.rglob("*")
            if path.is_file() and ".git" not in path.parts and ".build" not in path.parts
            and "__pycache__" not in path.parts and path.name != "LocalConfig.h"
        )
        self.assertIn('#include "LocalConfig.h"', source)
        self.assertNotIn("constexpr char AP_PASSWORD", source)
        self.assertIn("BalanceController/LocalConfig.h", ignored)
        self.assertIn("CHANGE_ME_8_CHARS", example)
        forbidden_default = "1234" + "5678"
        self.assertNotIn(forbidden_default, tracked_text)

    def test_web_interface_declares_callback_adapters_and_exact_snapshot_fields(self):
        header = (ROOT / "BalanceController/WebInterface.h").read_text(encoding="utf-8")
        for field in (
            "float lc1", "float lc2", "float lc3", "float lc4", "float x", "float y",
            "int people", "const char* helmet", "bool motorAllowed", "const char* lockReason",
            "const char* motorState", "uint16_t targetRpm", "float actualRpm",
            "float pwmPercent", "bool encoderCalibrated", "uint32_t pulsesPerRev",
            "const char* fault",
        ):
            self.assertIn(field, header)
        for token in (
            "SnapshotCopyCallback", "MotorCommandCallback", "RpmCommandCallback",
            "CalibrationCommandCallback", "PulsesPerRevCallback", "WebCallbacks",
            "void begin()", "void handleClient()",
        ):
            self.assertIn(token, header)

    def test_web_interface_registers_every_exact_route_with_post_mutations(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        for route in ("/", "/data"):
            self.assertRegex(source, rf'server_\.on\(\s*"{re.escape(route)}"\s*,\s*HTTP_GET')
        for route in FUTURE_ROUTES[1:]:
            self.assertRegex(source, rf'server_\.on\(\s*"{re.escape(route)}"\s*,\s*HTTP_POST')

    def test_web_data_json_uses_one_snapshot_copy_and_all_contract_keys(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        marker = "void WebInterface::handleData()"
        self.assertIn(marker, source)
        data_body = source.split(marker, 1)[1].split(
            "void WebInterface::handleRoot()", 1
        )[0]
        self.assertEqual(data_body.count("callbacks_.copySnapshot(snapshot)"), 1)
        for key in (
            "lc1", "lc2", "lc3", "lc4", "x", "y", "people", "helmet",
            "motorAllowed", "lockReason", "motorState", "targetRpm", "actualRpm",
            "pwmPercent", "encoderCalibrated", "pulsesPerRev", "fault",
        ):
            self.assertIn(f'\\"{key}\\"', data_body)
        self.assertIn("jsonEscape", data_body)

    def test_web_command_result_mapping_and_strict_rpm_validation_are_explicit(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        marker = "void WebInterface::handleMotorRpm()"
        self.assertIn(marker, source)
        rpm_body = source.split(marker, 1)[1].split(
            "void WebInterface::handleCalibrationStart()", 1
        )[0]
        for token in (
            'server_.hasArg("value")', "value.length() == 0", "isStrictRpm",
            "HTTP_STATUS_BAD_REQUEST", "callbacks_.motorRpm",
        ):
            self.assertIn(token, rpm_body)
        strict_parser = source.split("bool WebInterface::isStrictRpm", 1)[1].split(
            "bool WebInterface::isHtmlFilename", 1
        )[0]
        self.assertIn("MAX_TARGET_RPM", strict_parser)
        self.assertIn("character < '0' || character > '9'", strict_parser)
        self.assertIn("HTTP_STATUS_OK", source)
        self.assertIn("HTTP_STATUS_CONFLICT", source)
        self.assertNotIn("HTTP_OK", source)
        self.assertNotIn("HTTP_CONFLICT", source)
        self.assertNotIn("HTTP_BAD_REQUEST", source)
        self.assertIn('\\"ok\\"', source)
        self.assertIn('\\"reason\\"', source)

    def test_web_calibration_gates_on_stopped_snapshot_and_returns_saved_ppr(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        for name, callback in (
            ("handleCalibrationStart", "calibrationStart"),
            ("handleCalibrationFinish", "calibrationFinish"),
        ):
            marker = f"void WebInterface::{name}()"
            self.assertIn(marker, source)
            body = source.split(marker, 1)[1]
            self.assertIn("copySnapshot", body)
            self.assertIn("isMotorFullyStopped", body)
            self.assertIn(callback, body)
        finish_marker = "void WebInterface::handleCalibrationFinish()"
        finish_body = source.split(finish_marker, 1)[1].split(
            "void WebInterface::handleUploadComplete()", 1
        )[0]
        self.assertIn("pulsesPerRev", finish_body)
        self.assertIn("savedPulsesPerRev", finish_body)

    def test_web_mounts_without_format_and_serves_utf8_recovery_upload_form(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        self.assertIn("LittleFS.begin(false)", source)
        self.assertNotIn("LittleFS.begin(true)", source)
        self.assertIn('"text/html; charset=utf-8"', source)
        self.assertIn('action="/upload"', source)
        self.assertIn('enctype="multipart/form-data"', source)
        self.assertIn('accept=".html"', source)
        handle_body = source.split("void WebInterface::handleClient()", 1)[1]
        self.assertIn("server_.handleClient()", handle_body)
        self.assertNotIn("delay(", handle_body)

    def test_web_upload_uses_bounded_temp_transaction_and_short_write_checks(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        header = (ROOT / "BalanceController/WebInterface.h").read_text(encoding="utf-8")
        self.assertIn("262144", source + header)
        self.assertIn('"/index.tmp"', source)
        self.assertIn('"/index.html"', source)
        self.assertIn('"/index.bak"', source)
        self.assertIn("MAX_UPLOAD_BYTES - uploadBytes_", source)
        self.assertIn("written != upload.currentSize", source)
        self.assertIn("toLowerCase", source)
        self.assertIn('endsWith(".html")', source)
        self.assertIn("UPLOAD_FILE_START", source)
        self.assertIn("UPLOAD_FILE_WRITE", source)
        self.assertIn("UPLOAD_FILE_END", source)
        self.assertIn("UPLOAD_FILE_ABORTED", source)

    def test_web_upload_promotes_with_backup_restore_and_temp_only_error_cleanup(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        promote_marker = "bool WebInterface::promoteUpload()"
        self.assertIn(promote_marker, source)
        promote_body = source.split(promote_marker, 1)[1].split(
            "bool WebInterface::isStrictRpm", 1
        )[0]
        self.assertLess(promote_body.index('LittleFS.remove("/index.bak")'),
                        promote_body.index('LittleFS.rename("/index.html", "/index.bak")'))
        self.assertLess(promote_body.index('LittleFS.rename("/index.html", "/index.bak")'),
                        promote_body.index('LittleFS.rename("/index.tmp", "/index.html")'))
        self.assertIn('LittleFS.rename("/index.bak", "/index.html")', promote_body)
        self.assertLess(promote_body.index('LittleFS.rename("/index.tmp", "/index.html")'),
                        promote_body.rindex('LittleFS.remove("/index.bak")'))
        cleanup_marker = "void WebInterface::cleanupUploadTemp()"
        self.assertIn(cleanup_marker, source)
        cleanup_body = source.split(cleanup_marker, 1)[1].split(
            "bool WebInterface::promoteUpload()", 1
        )[0]
        self.assertIn('LittleFS.remove("/index.tmp")', cleanup_body)
        self.assertNotIn('LittleFS.remove("/index.html")', cleanup_body)
        self.assertNotIn('LittleFS.remove("/index.bak")', cleanup_body)

    def test_web_upload_rejects_raw_plain_and_json_before_reading_upload_state(self):
        for content_type in (None, "text/plain", "application/json", "Multipart/Form-Data; boundary=abc"):
            lifecycle = UploadLifecycleReference()
            self.assertEqual(lifecycle.finalize(content_type), (400, "invalid_upload_content_type"))
            self.assertEqual(lifecycle.upload_reads, 0)
            self.assertEqual(lifecycle.active, "old-page")

        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        begin_body = source.split("void WebInterface::begin()", 1)[1].split(
            "void WebInterface::handleClient()", 1
        )[0]
        self.assertIn("collectHeaders", begin_body)
        upload_body = source.split("void WebInterface::handleUploadData()", 1)[1].split(
            "void WebInterface::sendCommandResult", 1
        )[0]
        self.assertIn("isMultipartUploadRequest", upload_body)
        self.assertLess(upload_body.index("isMultipartUploadRequest"),
                        upload_body.index("server_.upload()"))
        multipart_guard = source.split("bool WebInterface::isMultipartUploadRequest", 1)[1].split(
            "bool WebInterface::isMotorFullyStopped", 1
        )[0]
        self.assertIn('startsWith("multipart/")', multipart_guard)
        self.assertIn('== "multipart/form-data"', multipart_guard)
        self.assertNotIn("equalsIgnoreCase", multipart_guard)

    def test_web_upload_defers_promotion_until_final_and_rejects_post_end_abort_or_second_file(self):
        successful = UploadLifecycleReference()
        successful.start_file("multipart/form-data; boundary=abc")
        successful.end_file()
        self.assertEqual((successful.active, successful.temp), ("old-page", "new-page"))
        self.assertEqual(successful.finalize("multipart/form-data; boundary=abc"), (200, "upload_promoted"))
        self.assertEqual(successful.active, "new-page")

        second_file = UploadLifecycleReference()
        second_file.start_file("multipart/form-data; boundary=abc")
        second_file.end_file()
        second_file.start_file("multipart/form-data; boundary=abc")
        self.assertEqual(second_file.finalize("multipart/form-data; boundary=abc")[0], 400)
        self.assertEqual((second_file.active, second_file.temp), ("old-page", None))

        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        end_body = source.split("case UPLOAD_FILE_END:", 1)[1].split(
            "case UPLOAD_FILE_ABORTED:", 1
        )[0]
        self.assertIn("uploadFileEnded_ = true", end_body)
        self.assertNotIn("promoteUpload", end_body)
        final_body = source.split("void WebInterface::handleUploadComplete()", 1)[1].split(
            "void WebInterface::handleUploadData()", 1
        )[0]
        self.assertIn("promoteUpload", final_body)
        self.assertIn("uploadStarted_ && uploadFileEnded_", final_body)

    def test_web_upload_and_root_require_bounded_recognizable_html(self):
        empty = UploadLifecycleReference()
        empty.start_file("multipart/form-data; boundary=abc")
        empty.end_file()
        empty.temp_content = ""
        self.assertEqual(empty.finalize("multipart/form-data; boundary=abc"), (400, "empty_html"))
        self.assertEqual(empty.active, "old-page")

        invalid = UploadLifecycleReference()
        invalid.start_file("multipart/form-data; boundary=abc")
        invalid.end_file()
        invalid.temp_content = "not an html document"
        self.assertEqual(invalid.finalize("multipart/form-data; boundary=abc"), (400, "invalid_html"))
        self.assertEqual(invalid.active, "old-page")

        valid = UploadLifecycleReference()
        valid.start_file("multipart/form-data; boundary=abc")
        valid.end_file()
        valid.temp_content = " \n<!doctype html><html><body>ok</body></html>"
        self.assertEqual(valid.finalize("multipart/form-data; boundary=abc"), (200, "upload_promoted"))
        self.assertEqual(valid.active, "new-page")

        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        header = (ROOT / "BalanceController/WebInterface.h").read_text(encoding="utf-8")
        self.assertIn("HTML_VALIDATION_BYTES", header)
        self.assertIn("isValidHtmlFile(TEMP_PAGE)", source)
        self.assertIn('failUpload("empty_html")', source)
        self.assertIn('failUpload("invalid_html")', source)
        root_body = source.split("void WebInterface::handleRoot()", 1)[1].split(
            "void WebInterface::handleMotorCommand", 1
        )[0]
        self.assertIn("isValidHtmlFile(ACTIVE_PAGE)", root_body)
        self.assertIn("<!doctype html", source.lower())
        self.assertIn('startsWith("<html")', source)

    def test_every_command_response_contains_one_coherent_current_state(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        response_body = source.split("void WebInterface::sendJsonResult", 1)[1].split(
            "void WebInterface::failUpload", 1
        )[0]
        self.assertIn("callbacks_.copySnapshot(snapshot)", response_body)
        self.assertEqual(response_body.count("callbacks_.copySnapshot(snapshot)"), 1)
        for key in ("state", "motorState", "targetRpm", "actualRpm", "motorAllowed", "ready", "fault"):
            self.assertIn(f'\\"{key}\\"', response_body)

    def test_command_response_refreshes_snapshot_after_adapter_mutation_before_copy(self):
        header = (ROOT / "BalanceController/WebInterface.h").read_text(encoding="utf-8")
        web = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        main = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")

        self.assertIn("SnapshotRefreshCallback", header)
        self.assertIn("refreshSnapshot", header)
        response_body = web.split("void WebInterface::sendJsonResult", 1)[1].split(
            "void WebInterface::failUpload", 1
        )[0]
        refresh_at = response_body.index("callbacks_.refreshSnapshot()")
        copy_at = response_body.index("callbacks_.copySnapshot(snapshot)")
        self.assertLess(refresh_at, copy_at)

        # C++ evaluates command() before entering sendCommandResult(), whose
        # sendJsonResult call performs the refresh above. Check every mutable
        # adapter path keeps that production ordering without a second call.
        for handler, callback in (
            ("handleMotorCommand", "command()"),
            ("handleMotorRpm", "callbacks_.motorRpm(rpm)"),
            ("handleCalibrationStart", "callbacks_.calibrationStart()"),
            ("handleCalibrationFinish", "callbacks_.calibrationFinish()"),
        ):
            handler_body = web.split(f"void WebInterface::{handler}", 1)[1]
            self.assertIn(callback, handler_body)
        self.assertEqual(web.count("callbacks_.refreshSnapshot()"), 1)

        refresh_body = main.split("void refreshDashboardSnapshot() {", 1)[1].split(
            "CommandResult motorOnAdapter", 1
        )[0]
        self.assertIn("safety.evaluate", refresh_body)
        self.assertIn("updateDashboardSnapshot", refresh_body)
        self.assertLess(refresh_body.index("safety.evaluate"), refresh_body.index("updateDashboardSnapshot"))
        callback_initializer = main.split("const WebCallbacks WEB_CALLBACKS", 1)[1].split(
            "WebInterface web", 1
        )[0]
        self.assertLess(callback_initializer.index("refreshDashboardSnapshot"),
                        callback_initializer.index("motorOnAdapter"))

    def test_web_upload_abort_resets_without_final_response_then_allows_fresh_upload(self):
        lifecycle = UploadLifecycleReference()
        lifecycle.start_file("multipart/form-data; boundary=abc")
        lifecycle.end_file()
        lifecycle.abort()
        self.assertEqual((lifecycle.active, lifecycle.temp), ("old-page", None))

        lifecycle.start_file("multipart/form-data; boundary=def")
        lifecycle.end_file()
        self.assertEqual(lifecycle.finalize("multipart/form-data; boundary=def"),
                         (200, "upload_promoted"))
        self.assertEqual(lifecycle.active, "new-page")

        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        abort_body = source.split("case UPLOAD_FILE_ABORTED:", 1)[1].split(
            "default:", 1
        )[0]
        self.assertIn("resetUploadRequest()", abort_body)
        self.assertLess(abort_body.index("failUpload(\"upload_aborted\")"),
                        abort_body.index("resetUploadRequest()"))

    def test_webserver_v2_header_types_and_nonconst_access_contract(self):
        header = (ROOT / "BalanceController/WebInterface.h").read_text(encoding="utf-8")
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        self.assertIn("bool isMultipartUploadRequest();", header)
        self.assertNotIn("bool isMultipartUploadRequest() const;", header)
        self.assertIn("const char* REQUEST_HEADERS[]", source)
        self.assertNotIn("const char* const REQUEST_HEADERS[]", source)
        self.assertIn("server_.collectHeaders(REQUEST_HEADERS, 1)", source)

    def test_web_upload_final_response_resets_lifecycle_before_consecutive_no_file_post(self):
        lifecycle = UploadLifecycleReference()
        lifecycle.start_file("multipart/form-data; boundary=abc")
        lifecycle.end_file()
        self.assertEqual(lifecycle.finalize("multipart/form-data; boundary=abc")[0], 200)
        self.assertEqual(lifecycle.finalize("multipart/form-data; boundary=abc"),
                         (400, "upload_incomplete"))

        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        final_body = source.split("void WebInterface::handleUploadComplete()", 1)[1].split(
            "void WebInterface::handleUploadData()", 1
        )[0]
        self.assertIn("resetUploadRequest", final_body)
        reset_body = source.split("void WebInterface::resetUploadRequest()", 1)[1].split(
            "bool WebInterface::promoteUpload()", 1
        )[0]
        for token in ("uploadBytes_ = 0", "uploadStarted_ = false", "uploadFileEnded_ = false",
                      "uploadFailed_ = false", 'uploadFailureReason_ = "upload_incomplete"'):
            self.assertIn(token, reset_body)

    def test_web_upload_reports_rollback_failure_without_deleting_backup(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        promote_body = source.split("bool WebInterface::promoteUpload()", 1)[1].split(
            "bool WebInterface::isStrictRpm", 1
        )[0]
        rollback = 'LittleFS.rename("/index.bak", "/index.html")'
        self.assertIn(rollback, promote_body)
        self.assertIn(f"if (!{rollback})", promote_body)
        self.assertIn('uploadFailureReason_ = "rollback_failed"', promote_body)

    def test_web_snapshot_handlers_reject_missing_copy_callback_without_dereference(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        for handler, boundary in (
            ("handleData", "void WebInterface::handleRoot()"),
            ("handleCalibrationStart", "void WebInterface::handleCalibrationFinish()"),
            ("handleCalibrationFinish", "void WebInterface::handleUploadComplete()"),
        ):
            marker = f"void WebInterface::{handler}()"
            body = source.split(marker, 1)[1].split(boundary, 1)[0]
            self.assertIn("callbacks_.copySnapshot == nullptr", body)
            self.assertLess(body.index("callbacks_.copySnapshot == nullptr"),
                            body.index("callbacks_.copySnapshot(snapshot)"))
            self.assertIn("not_configured", body)

    def test_dashboard_html_contract_is_offline_safe_and_controls_firmware_api(self):
        html = (ROOT / "BalanceController/data/index.html").read_text(encoding="utf-8")

        # Removing any of these IDs breaks a real status/control boundary.
        for element_id in (
            "helmetStatus", "peopleCount", "motorAllowed", "lockReason",
            "lc1", "lc2", "lc3", "lc4", "dot", "motorToggle", "rpmSlider",
            "targetRpm", "actualRpm", "motorState", "estopButton", "faultReset",
            "calibrationStart", "calibrationFinish", "htmlUpload", "commandMessage",
        ):
            self.assertRegex(html, rf'id=["\']{element_id}["\']')
        for route in FUTURE_ROUTES:
            self.assertIn(route, html)

        # The dashboard must send commands to the firmware rather than deriving authority.
        self.assertRegex(html, r'fetch\("/data"\s*(?:,|\))')
        self.assertRegex(html, r"setTimeout\s*\(\s*pollData\s*,\s*100\s*\)")
        self.assertRegex(html, r'motorAllowed\s*===\s*true\s*&&\s*state\s*===\s*["\']ready["\']')
        self.assertRegex(html, r'["\']armed["\']\s*\|\|\s*state\s*===\s*["\']running["\']')
        self.assertRegex(html, r'id=["\']rpmSlider["\'][^>]*min=["\']0["\'][^>]*max=["\']300["\'][^>]*step=["\']1["\']')
        self.assertIn("FormData", html)
        self.assertIn('form.append("file"', html)
        self.assertIn("window.location.replace(\"/\")", html)
        self.assertIn("response.ok && body.ok === true", html)
        self.assertIn("수집 중", html)
        self.assertIn("출력축/바퀴를 정확히 한 바퀴", html)
        self.assertIn("commandMessage", html)
        self.assertRegex(html, r'id=["\']faultReset["\'][^>]*\bdisabled\b')
        self.assertRegex(html, r'id=["\']calibrationStart["\'][^>]*\bdisabled\b')
        self.assertRegex(html, r'id=["\']calibrationFinish["\'][^>]*\bdisabled\b')
        self.assertRegex(html, r'<label[^>]+for=["\']htmlUpload["\']')
        self.assertIn("AbortController", html)
        self.assertIn("DATA_TIMEOUT_MS", html)
        self.assertIn("controller.abort()", html)
        self.assertIn("signal: controller.signal", html)
        self.assertIn("renderFailSafe", html)
        self.assertIn("setNormalControlsDisabled(true)", html)
        self.assertIn('refreshData({ force: true, reconcileRpm: endpoint === "/motor/rpm" })', html)
        self.assertIn("dataGeneration", html)
        self.assertIn("rpmInteracting", html)
        self.assertNotRegex(html, r"https?://|//[a-zA-Z0-9._-]+")
        self.assertNotRegex(html, r"<script[^>]+\bsrc=|<link[^>]+\bhref=|\bimport\s*(?:\(|[^a-zA-Z])")

    def test_main_sketch_declares_exact_hx711_inputs_and_sampling_contract(self):
        main = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        helper = (ROOT / "BalanceController/LoadFeatureModel.h").read_text(encoding="utf-8")
        source = main + "\n" + helper
        for token in (
            "#include <HX711.h>",
            "LC1_DOUT = 34", "LC1_SCK = 16",
            "LC2_DOUT = 36", "LC2_SCK = 18",
            "LC3_DOUT = 35", "LC3_SCK = 17",
            "LC4_DOUT = 39", "LC4_SCK = 19",
            "DATA_INTERVAL = 50", "NOISE_THRESHOLD = 100000.0f",
            "WINDOW_SIZE = 20",
        ):
            self.assertIn(token, source)

    def test_main_sketch_preserves_classifier_feature_order_and_collecting_state(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        self.assertIn("extern \"C\" void score(double*, double*);", source)
        self.assertIn("score(features, output_scores)", source)
        self.assertIn("int people = -1", source)
        for index in range(16):
            self.assertRegex(source, rf"features\s*\[\s*{index}\s*\]")
        feature_positions = [source.index(f"features[{index}]") for index in range(16)]
        self.assertEqual(feature_positions, sorted(feature_positions))
        self.assertIn("safety.updatePeople(predictedPeople, nowMs)", source)

    def test_main_sketch_boots_outputs_then_ap_channel_then_espnow_then_web(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        setup_body = source.split("void setup()", 1)[1].split("void loop()", 1)[0]
        ordered = (
            "Serial.begin", "motor.begin()", "encoder.begin()", "setupLoadCells()",
            "WiFi.mode(WIFI_AP_STA)", "WiFi.softAP", "helmetReceiver.begin", "web.begin()",
        )
        positions = [setup_body.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("WiFi.softAP(AP_SSID, AP_PASSWORD, WIFI_CHANNEL)", setup_body)

    def test_main_sketch_uses_callback_mailbox_and_nonblocking_loop_services(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        callback_match = re.search(
            r"void queueHelmetUpdate\s*\(bool worn, uint32_t nowMs\)\s*\{(?P<body>.*?)\n\}",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(callback_match)
        callback = callback_match.group("body")
        for token in (
            "portENTER_CRITICAL", "pendingHelmetWorn", "pendingHelmetNowMs",
            "pendingHelmetUpdate = true", "portEXIT_CRITICAL",
        ):
            self.assertIn(token, callback)
        self.assertNotIn("safety.noteHelmet", callback)
        self.assertNotIn("motor.", callback)
        self.assertNotIn("web.", callback)
        loop_body = source.split("void loop()", 1)[1]
        for token in (
            "web.handleClient()", "consumeHelmetMailbox()", "encoder.tick(nowMs)",
            "safety.evaluate", "motor.tick(nowMs, safetySnapshot, encoder.rpm())",
            "updateDashboardSnapshot(safetySnapshot)",
        ):
            self.assertIn(token, loop_body)
        self.assertNotIn("delay(", loop_body)
        self.assertIn("nowMs - lastDataMs >= DATA_INTERVAL", loop_body)

    def test_main_sketch_calibration_lock_and_dashboard_mapping_are_firmware_owned(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        self.assertIn("encoder.isCalibrated() && !encoder.isCalibrating()", source)
        self.assertIn("motor.faultReason()", source)
        for field in (
            "dashboardSnapshot.lc1 =", "dashboardSnapshot.lc2 =", "dashboardSnapshot.lc3 =", "dashboardSnapshot.lc4 =",
            "dashboardSnapshot.x =", "dashboardSnapshot.y =", "dashboardSnapshot.people =", "dashboardSnapshot.helmet =",
            "dashboardSnapshot.motorAllowed = safetySnapshot.ready", "dashboardSnapshot.lockReason = safetySnapshot.lockReason",
            "dashboardSnapshot.motorState = motorStateName(motor.state())", "dashboardSnapshot.targetRpm = motor.targetRpm()",
            "dashboardSnapshot.actualRpm = encoder.rpm()", "dashboardSnapshot.pwmPercent = motor.pwmPercent()",
            "dashboardSnapshot.encoderCalibrated = encoder.isCalibrated()", "dashboardSnapshot.pulsesPerRev = encoder.pulsesPerRevolution()",
            "dashboardSnapshot.fault = dashboardFault()",
        ):
            self.assertIn(field, source)

    def test_main_sketch_debounces_runtime_load_readiness_before_history_or_score(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        if "bool updateLoadIfReady" not in source:
            self.fail("runtime HX711 readiness helper must report failure")
        update_body = source.split("bool updateLoadIfReady", 1)[1].split(
            "void appendHistory", 1
        )[0]
        for token in (
            "HX711_READY_TIMEOUT_MS = 500", "uint32_t* lastReadyMs", "nowMs - *lastReadyMs",
            "if (!cell.is_ready())", "latchLoadSubsystemFault", "return !loadSubsystemFault",
        ):
            self.assertIn(token, source + update_body)
        self.assertIn("nowMs - *lastReadyMs >= HX711_READY_TIMEOUT_MS", update_body)
        sample_body = source.split("void sampleLoadCellsAndClassifier", 1)[1].split(
            "void queueHelmetUpdate", 1
        )[0]
        readiness_failure = "!updateLoadIfReady(loadCell1, &currentWeights[0], &lastLoadReadyMs[0], nowMs)"
        self.assertIn(readiness_failure, sample_body)
        self.assertLess(sample_body.index(readiness_failure), sample_body.index("appendHistory(currentWeights)"))
        self.assertLess(sample_body.index(readiness_failure), sample_body.index("classifyWindow(nowMs)"))
        self.assertIn("lastLoadReadyMs[4]", source)
        self.assertRegex(source, r"safety\.updatePeople\(\s*-1\s*,")

    def test_main_sketch_uses_bounded_manual_tare_without_hx711_tare_call(self):
        source = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        if "bool tareLoadCellBounded" not in source:
            self.fail("load-cell tare must be explicitly bounded")
        tare_body = source.split("bool tareLoadCellBounded", 1)[1].split(
            "bool initializeLoadCell", 1
        )[0]
        for token in (
            "TARE_TIMEOUT_MS", "TARE_REQUIRED_SAMPLES", "millis() - startedMs < TARE_TIMEOUT_MS",
            "cell.is_ready()", "cell.read()", "cell.set_offset(", "samples < TARE_REQUIRED_SAMPLES",
        ):
            self.assertIn(token, tare_body)
        self.assertNotIn("cell.tare()", source)

    def test_web_interface_uses_portable_numeric_http_statuses(self):
        source = (ROOT / "BalanceController/WebInterface.cpp").read_text(encoding="utf-8")
        for token in (
            "constexpr int HTTP_STATUS_OK = 200;",
            "constexpr int HTTP_STATUS_BAD_REQUEST = 400;",
            "constexpr int HTTP_STATUS_CONFLICT = 409;",
        ):
            self.assertIn(token, source)

    def test_main_sketch_uses_production_feature_helper_with_compile_time_golden_checks(self):
        main = (ROOT / "BalanceController/BalanceController.ino").read_text(encoding="utf-8")
        helper = (ROOT / "BalanceController/LoadFeatureModel.h").read_text(encoding="utf-8")
        self.assertIn('#include "LoadFeatureModel.h"', main)
        self.assertIn("LoadFeatureModel::calculateWindow", main)
        for token in (
            "static_assert", "SIGNED_LOW_GOLDEN", "NEGATIVE_DENOMINATOR_GOLDEN",
            "VARYING_SIGNED_GOLDEN", "280.0", "700.0", "FEATURE_EPSILON", "values[15]",
        ):
            self.assertIn(token, helper)
        for golden_features in ("SIGNED_LOW_GOLDEN_FEATURES", "VARYING_SIGNED_GOLDEN_FEATURES"):
            for index in range(16):
                self.assertIn(f"{golden_features}.values[{index}]", helper)
        self.assertRegex(helper, r"NEGATIVE_DENOMINATOR_GOLDEN_FEATURES\.values\[10\]")
        self.assertRegex(helper, r"NEGATIVE_DENOMINATOR_GOLDEN_FEATURES\.values\[12\]")
        self.assertIn('"cx mean"', helper)
        self.assertIn('"cy mean"', helper)
        self.assertIn('"varying cx std is nonzero"', helper)

    def test_production_feature_header_compiles_varying_golden_and_sqrt_against_real_values(self):
        compiler = esp32_cpp14_compiler()
        sqrt_inputs = (1e-12, 1e-6, 0.25, 1.0, 2.0, 4.0, 1e6, 64e12, 1e18)
        sqrt_assertions = "\n".join(
            "static_assert(LoadFeatureModel::approximatelyEqual("
            f"LoadFeatureModel::squareRoot({value:.17g}), {math.sqrt(value):.17g}, "
            f"{max(1e-15, math.sqrt(value) * 1e-12):.17g}), \"sqrt {value:.17g}\");"
            for value in sqrt_inputs
        )
        source = f'''#include "BalanceController/LoadFeatureModel.h"
constexpr LoadFeatureModel::FeatureVector varying =
    LoadFeatureModel::calculateWindow(LoadFeatureModel::VARYING_SIGNED_GOLDEN, 0);
static_assert(LoadFeatureModel::approximatelyEqual(varying.values[1], 15000000.0, 1e-3),
              "varying total std");
static_assert(LoadFeatureModel::approximatelyEqual(varying.values[3], 8000000.0, 1e-3),
              "varying w1 std");
static_assert(LoadFeatureModel::approximatelyEqual(varying.values[11], 1.0, 1e-12),
              "varying front rear correlation");
{sqrt_assertions}
int main() {{ return 0; }}
'''
        cxx11_source = '''#include "BalanceController/LoadFeatureModel.h"
int main() {
  return LoadFeatureModel::squareRoot(64000000000000.0) > 0.0 ? 0 : 1;
}
'''
        for standard, compile_source in (("gnu++11", cxx11_source), ("gnu++14", source)):
            with self.subTest(standard=standard):
                result = subprocess.run(
                    [str(compiler), f"-std={standard}", "-x", "c++", "-fsyntax-only", "-I", str(ROOT), "-"],
                    input=compile_source,
                    text=True,
                    capture_output=True,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
