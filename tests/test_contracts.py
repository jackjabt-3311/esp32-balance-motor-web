import hashlib
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
    "BalanceController/model.c",
    "BalanceController/BalanceController.ino",
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

    @unittest.expectedFailure
    def test_future_api_and_html_contract_lists_all_routes(self):
        html = (ROOT / "BalanceController/data/index.html").read_text(encoding="utf-8")
        for route in FUTURE_ROUTES:
            self.assertIn(route, html)


if __name__ == "__main__":
    unittest.main()
