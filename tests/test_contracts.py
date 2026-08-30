import hashlib
from pathlib import Path
import re
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_MODEL_SHA256 = "f6e136bd80e5003a7465616237539f3f37d3affbc3b7192fce4df2dd0182dd54"
FORBIDDEN_LOCAL_PATH = "C:" + chr(92) + "Users" + chr(92)
FORBIDDEN_RECEIVER_MAC = "04" + ":B2:47:54:D1:FC"

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

    def test_repository_text_excludes_local_paths_and_receiver_mac(self):
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
            if FORBIDDEN_LOCAL_PATH in text or FORBIDDEN_RECEIVER_MAC in text:
                violations.append(relative_path.as_posix())
        self.assertIn(".gitignore", scanned_paths)
        self.assertEqual(violations, [])

    @unittest.expectedFailure
    def test_future_api_and_html_contract_lists_all_routes(self):
        html = (ROOT / "BalanceController/data/index.html").read_text(encoding="utf-8")
        for route in FUTURE_ROUTES:
            self.assertIn(route, html)


if __name__ == "__main__":
    unittest.main()
