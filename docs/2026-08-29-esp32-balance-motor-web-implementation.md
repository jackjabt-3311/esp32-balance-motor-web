# ESP32 Balance Motor Web Control Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a local ESP32 dashboard that combines helmet status, passenger prediction, load distribution, LittleFS HTML replacement, and safe BTS7960 target-RPM control.

**Architecture:** Keep the supplied load-cell and classifier flow, but move safety, encoder, motor, ESP-NOW, and web concerns into focused Arduino-compatible C++ units. Firmware remains the authority for all safety decisions; LittleFS `index.html` is replaceable presentation code that communicates through JSON HTTP endpoints.

**Tech Stack:** ESP32 Arduino core, Arduino `WebServer`, `WiFi`, ESP-NOW, `LittleFS`, `Preferences`, HX711, C/C++, HTML/CSS/JavaScript, Python static contract tests.

**Spec:** `work/docs/superpowers/specs/2026-08-29-esp32-balance-motor-web-design.md`

## Global Constraints

- Local AP operation only; use AP+STA mode and Wi-Fi channel 1.
- One BTS7960-driven motor and one rotation direction only.
- RPWM GPIO 25, LPWM GPIO 26, R_EN GPIO 27, L_EN GPIO 14.
- Encoder A GPIO 32 and Encoder B GPIO 33.
- Target RPM range is 0 through 300; ON arms at zero RPM.
- Helmet packet timeout is 500 ms.
- Passenger count must remain exactly one for 1000 ms before start is allowed.
- Normal OFF or invalid safety prerequisite ramps down over 2000 ms.
- Emergency stop, boot/reset, invalid internal state, and encoder-stall fault stop immediately.
- Encoder-stall threshold: target at least 20 RPM, PWM at least 25%, actual RPM below 5 for 1000 ms.
- PID loop period is 50 ms; RPM calculation period is 100 ms.
- HTML upload is limited to 256 KiB and must use temporary-write then rename.
- Never automatically restart after a stop, ramp-down, recovered prerequisite, fault, or reset.
- Preserve the supplied `model.c` unless an ESP32 compilation error requires a compatibility-only change.
- Do not initialize a Git repository without user authorization. This workspace is currently not a Git repository.

## File Map

- Create `work/esp32-balance-controller/BalanceController/BalanceController.ino` — main setup/loop and existing sensor/ML integration.
- Copy `work/esp32-balance-controller/BalanceController/model.c` — supplied classifier implementation.
- Create `work/esp32-balance-controller/BalanceController/ControlTypes.h` — shared enums, state, command results, and JSON-facing names.
- Create `work/esp32-balance-controller/BalanceController/SafetyGate.h/.cpp` — helmet freshness and stable-one-passenger qualification.
- Create `work/esp32-balance-controller/BalanceController/EncoderService.h/.cpp` — encoder counting, RPM calculation, and one-turn calibration persistence.
- Create `work/esp32-balance-controller/BalanceController/MotorController.h/.cpp` — BTS7960 output, PID, state machine, ramp-down, stall, fault, and emergency stop.
- Create `work/esp32-balance-controller/BalanceController/HelmetReceiver.h/.cpp` — ESP-NOW receiver initialization and packet validation.
- Create `work/esp32-balance-controller/BalanceController/WebInterface.h/.cpp` — LittleFS serving, JSON API, commands, calibration, and atomic HTML upload.
- Create `work/esp32-balance-controller/BalanceController/data/index.html` — replaceable dashboard.
- Create `work/esp32-balance-controller/HelmetTransmitter/HelmetTransmitter.ino` — supplied helmet logic with fixed channel 1.
- Create `work/esp32-balance-controller/tests/test_contracts.py` — source/API/HTML contract and safety-constant checks.
- Create `work/esp32-balance-controller/tests/test_state_model.py` — executable reference-model tests for state transitions and timing.
- Create `work/esp32-balance-controller/README.md` — wiring, upload, calibration, first-run, PID tuning, and hardware validation.
- Package final user-facing files under `outputs/esp32-balance-controller/` only after verification.

---

### Task 1: Scaffold the Deliverable and Lock Shared Contracts

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/ControlTypes.h`
- Create: `work/esp32-balance-controller/tests/test_contracts.py`
- Create: `work/esp32-balance-controller/tests/test_state_model.py`
- Copy: supplied `model.c` to `work/esp32-balance-controller/BalanceController/model.c`

**Interfaces:**
- Produces: `enum class MotorState`, `struct CommandResult`, `motorStateName(MotorState)`, endpoint constants, and exact dashboard JSON field names.
- Consumes: no earlier task.

- [ ] **Step 1: Write the failing source-contract test**

```python
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SKETCH = ROOT / "BalanceController"

class ContractTests(unittest.TestCase):
    def test_required_files_and_constants_exist(self):
        expected = [
            "BalanceController.ino", "ControlTypes.h", "SafetyGate.cpp",
            "EncoderService.cpp", "MotorController.cpp", "HelmetReceiver.cpp",
            "WebInterface.cpp", "model.c", "data/index.html",
        ]
        self.assertTrue(all((SKETCH / item).exists() for item in expected))
        joined = "\n".join(p.read_text(encoding="utf-8", errors="ignore") for p in SKETCH.rglob("*.*") if p.is_file())
        for token in ["MAX_TARGET_RPM = 300", "RAMP_DOWN_MS = 2000", "HELMET_TIMEOUT_MS = 500", "PEOPLE_STABLE_MS = 1000"]:
            self.assertIn(token, joined)

    def test_api_and_html_contract_match(self):
        cpp = (SKETCH / "WebInterface.cpp").read_text(encoding="utf-8")
        html = (SKETCH / "data/index.html").read_text(encoding="utf-8")
        for route in ["/data", "/motor/on", "/motor/off", "/motor/rpm", "/motor/estop", "/motor/reset", "/encoder/calibration/start", "/encoder/calibration/finish", "/upload"]:
            self.assertIn(route, cpp)
            if route != "/upload":
                self.assertIn(route, html)
```

- [ ] **Step 2: Run the test and confirm it fails because implementation files do not exist**

Run: `python -m unittest discover -s work/esp32-balance-controller/tests -p "test_*.py" -v`

Expected: FAIL on missing files.

- [ ] **Step 3: Create the shared type contract**

```cpp
#pragma once
#include <Arduino.h>

constexpr uint16_t MAX_TARGET_RPM = 300;
constexpr uint32_t RAMP_DOWN_MS = 2000;
constexpr uint32_t HELMET_TIMEOUT_MS = 500;
constexpr uint32_t PEOPLE_STABLE_MS = 1000;

enum class MotorState : uint8_t { LOCKED, READY, ARMED, RUNNING, RAMPING_DOWN, FAULT, ESTOP };

struct CommandResult {
  bool ok;
  const char* reason;
};

const char* motorStateName(MotorState state);
```

Implement `motorStateName` with an exhaustive switch returning lowercase JSON values: `locked`, `ready`, `armed`, `running`, `ramping_down`, `fault`, `estop`; use `fault` for an impossible value.

- [ ] **Step 4: Copy the supplied classifier without modification and create empty compilation units named by the contract test**

Copy `model.c` from the supplied source attachment to `BalanceController/model.c` without modification. Create source files only through patch edits.

- [ ] **Step 5: Run the contract tests and record the remaining expected failures**

Run: `python -m unittest discover -s work/esp32-balance-controller/tests -p "test_*.py" -v`

Expected: file-presence assertions pass; endpoint/content assertions still fail until later tasks.

### Task 2: Implement and Test the Safety Gate

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/SafetyGate.h`
- Create: `work/esp32-balance-controller/BalanceController/SafetyGate.cpp`
- Modify: `work/esp32-balance-controller/tests/test_state_model.py`

**Interfaces:**
- Consumes: `HELMET_TIMEOUT_MS`, `PEOPLE_STABLE_MS` from `ControlTypes.h`.
- Produces: `SafetyGate::noteHelmet(bool,uint32_t)`, `SafetyGate::updatePeople(int,uint32_t)`, `SafetyGate::evaluate(uint32_t,bool,bool)`, and `SafetySnapshot`.

- [ ] **Step 1: Write failing reference-model tests for qualification and loss**

```python
import unittest

class SafetyModel:
    HELMET_TIMEOUT = 500
    PEOPLE_STABLE = 1000
    def __init__(self):
        self.helmet_worn = False
        self.last_helmet_ms = None
        self.people_since_ms = None
    def helmet(self, worn, now):
        self.helmet_worn, self.last_helmet_ms = worn, now
    def people(self, count, now):
        if count == 1:
            if self.people_since_ms is None: self.people_since_ms = now
        else:
            self.people_since_ms = None
    def ready(self, now, calibrated=True, fault=False):
        fresh = self.last_helmet_ms is not None and now - self.last_helmet_ms <= self.HELMET_TIMEOUT
        stable = self.people_since_ms is not None and now - self.people_since_ms >= self.PEOPLE_STABLE
        return fresh and self.helmet_worn and stable and calibrated and not fault

class SafetyTests(unittest.TestCase):
    def test_requires_fresh_helmet_and_one_stable_second(self):
        s = SafetyModel(); s.helmet(True, 0); s.people(1, 0)
        self.assertFalse(s.ready(999)); s.helmet(True, 1000)
        self.assertTrue(s.ready(1000))
    def test_packet_timeout_and_people_change_revoke_ready(self):
        s = SafetyModel(); s.helmet(True, 1000); s.people(1, 0)
        self.assertTrue(s.ready(1000)); self.assertFalse(s.ready(1501))
        s.helmet(True, 1501); s.people(2, 1501)
        self.assertFalse(s.ready(1501))
```

- [ ] **Step 2: Run the reference-model tests**

Run: `python -m unittest discover -s work/esp32-balance-controller/tests -p "test_state_model.py" -v`

Expected: PASS, establishing the required timing behavior before C++ implementation.

- [ ] **Step 3: Implement `SafetyGate` with the same state transitions**

```cpp
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
  SafetySnapshot evaluate(uint32_t nowMs, bool encoderCalibrated, bool motorFault) const;
 private:
  bool helmetWorn_ = false;
  bool helmetSeen_ = false;
  uint32_t lastHelmetMs_ = 0;
  bool onePersonTiming_ = false;
  uint32_t onePersonSinceMs_ = 0;
};
```

Return lock reasons in this priority order: `helmet_signal_lost`, `helmet_not_worn`, `people_not_one`, `people_not_stable`, `encoder_not_calibrated`, `motor_fault`, `ready`.

- [ ] **Step 4: Extend static tests to verify constants and reason strings exist in the C++ implementation**

Add assertions for every lock reason and run:

`python -m unittest discover -s work/esp32-balance-controller/tests -v`

Expected: safety contract tests PASS.

### Task 3: Implement Encoder Measurement and Calibration

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/EncoderService.h`
- Create: `work/esp32-balance-controller/BalanceController/EncoderService.cpp`
- Modify: `work/esp32-balance-controller/tests/test_state_model.py`

**Interfaces:**
- Produces: `EncoderService::begin()`, `tick(uint32_t)`, `rpm()`, `isCalibrated()`, `startCalibration()`, `finishCalibration()`, `pulsesPerRevolution()`.
- Persists: NVS namespace `encoder`, key `ppr` as unsigned integer.

- [ ] **Step 1: Add failing RPM formula and calibration-validation tests**

```python
def rpm_from_edges(edges, elapsed_ms, ppr):
    if elapsed_ms <= 0 or ppr <= 0: return 0.0
    return edges * 60000.0 / (elapsed_ms * ppr)

class EncoderTests(unittest.TestCase):
    def test_rpm_formula(self):
        self.assertAlmostEqual(rpm_from_edges(100, 1000, 100), 60.0)
    def test_invalid_calibration_is_rejected(self):
        self.assertEqual(rpm_from_edges(100, 1000, 0), 0.0)
```

- [ ] **Step 2: Run tests and confirm the reference formula passes**

Run: `python -m unittest discover -s work/esp32-balance-controller/tests -p "test_state_model.py" -v`

Expected: PASS.

- [ ] **Step 3: Implement interrupt-safe edge counting and 100 ms RPM calculation**

Use GPIO 32 as the counted A edge with `attachInterrupt(..., RISING)` and read GPIO 33 only for direction diagnostics; one-direction RPM uses the absolute edge count. Protect counter transfer with `portENTER_CRITICAL_ISR` in the ISR and `portENTER_CRITICAL` in `tick`.

Implement:

```cpp
float newRpm = pulses * 60000.0f / (elapsedMs * pulsesPerRevolution_);
```

Update no faster than `RPM_SAMPLE_MS = 100` and return zero when uncalibrated.

- [ ] **Step 4: Implement stopped-state one-revolution calibration**

`startCalibration()` clears a separate pulse accumulator only when not already calibrating. `finishCalibration()` accepts counts from 1 through 100000, writes `ppr` with `Preferences`, and returns `{false,"invalid_pulse_count"}` otherwise. Motor code must pass a stopped-state check before exposing these calls through HTTP.

- [ ] **Step 5: Run contract tests**

Expected checks: GPIO 32/33, NVS namespace/key, 100 ms timing, nonzero validation, and RPM formula all PASS.

### Task 4: Implement BTS7960 PID and Motor State Machine

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/MotorController.h`
- Create: `work/esp32-balance-controller/BalanceController/MotorController.cpp`
- Modify: `work/esp32-balance-controller/tests/test_state_model.py`

**Interfaces:**
- Consumes: `MotorState`, `SafetySnapshot`, and measured RPM.
- Produces: `begin()`, `tick(nowMs,safety,actualRpm)`, `requestOn(safety)`, `requestOff(nowMs)`, `requestEstop()`, `requestReset(safety)`, `setTargetRpm(uint16_t)`, `state()`, `targetRpm()`, `pwmPercent()`, `faultReason()`.

- [ ] **Step 1: Add reference tests for ON, slider, ramp, relock, and no auto-restart**

```python
class MotorModel:
    def __init__(self): self.state="ready"; self.target=0; self.ramp_start=None
    def on(self, ready):
        if ready and self.state=="ready": self.state="armed"; return True
        return False
    def set_rpm(self, value):
        if self.state not in ("armed","running") or not 0 <= value <= 300: return False
        self.target=value; self.state="running" if value else "armed"; return True
    def lose_safety(self, now): self.state="ramping_down"; self.ramp_start=now; self.target=0
    def tick(self, now, ready):
        if self.state=="ramping_down" and now-self.ramp_start>=2000:
            self.state="ready" if ready else "locked"

class MotorTests(unittest.TestCase):
    def test_on_arms_at_zero_then_slider_runs(self):
        m=MotorModel(); self.assertTrue(m.on(True)); self.assertEqual(m.target,0)
        self.assertTrue(m.set_rpm(50)); self.assertEqual(m.state,"running")
    def test_safety_loss_finishes_ramp_without_restart(self):
        m=MotorModel(); m.on(True); m.set_rpm(100); m.lose_safety(10); m.tick(2010,True)
        self.assertEqual(m.state,"ready"); self.assertEqual(m.target,0)
```

- [ ] **Step 2: Run reference tests**

Run: `python -m unittest discover -s work/esp32-balance-controller/tests -p "test_state_model.py" -v`

Expected: PASS.

- [ ] **Step 3: Implement boot-safe BTS7960 output**

Set GPIO 25/26/27/14 as outputs and write both PWM channels and both enables LOW before any controller state can become ready. Keep LPWM at zero for all states. Use ESP32 LEDC through a compatibility wrapper that supports the installed Arduino-ESP32 core detected at compile time.

- [ ] **Step 4: Implement ON/OFF and two-second nonblocking ramp**

Record `rampStartMs_` and `rampStartDuty_`; compute:

```cpp
float progress = min(1.0f, float(nowMs - rampStartMs_) / 2000.0f);
float duty = rampStartDuty_ * (1.0f - progress);
```

At progress 1.0, force duty zero, disable both enable pins, clear target RPM, and enter READY or LOCKED based on the current gate. Do not accept ON or RPM commands during ramp-down.

- [ ] **Step 5: Implement 50 ms PID with anti-windup**

Use conservative initial gains `Kp=0.8`, `Ki=0.15`, `Kd=0.0`; clamp integral to the duty range and reset it whenever stopped. Clamp PWM output to 0–100%. These gains are starting values only and must be labeled hardware-unverified in README.

- [ ] **Step 6: Implement immediate stop and latched errors**

Emergency stop immediately writes PWM zero and enables LOW, enters ESTOP, and clears target. Stall timing starts only when target >=20, PWM >=25%, and actual RPM <5; reaching 1000 ms enters FAULT with `encoder_stall`. Reset is accepted only while output is zero; it returns to READY only if the safety snapshot is ready, otherwise LOCKED.

- [ ] **Step 7: Extend static contract tests and run the full suite**

Assert pin numbers, PID/RPM periods, stall thresholds, `RAMPING_DOWN`, `FAULT`, and `ESTOP` occur in source. Expected: all motor reference and static checks PASS.

### Task 5: Add ESP-NOW Helmet Reception and Fix the Transmitter Channel

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/HelmetReceiver.h`
- Create: `work/esp32-balance-controller/BalanceController/HelmetReceiver.cpp`
- Create: `work/esp32-balance-controller/HelmetTransmitter/HelmetTransmitter.ino`
- Modify: `work/esp32-balance-controller/tests/test_contracts.py`

**Interfaces:**
- Produces: `HelmetReceiver::begin(callback)`, validated `HelmetMessage{bool isTriggered}`, and callback `(bool worn,uint32_t nowMs)`.
- Consumes: `SafetyGate::noteHelmet` through a main-sketch adapter callback.

- [ ] **Step 1: Add failing tests for fixed channel and payload validation**

Assert both sketches contain `WIFI_CHANNEL = 1`; receiver contains a length check against `sizeof(HelmetMessage)`; transmitter still sends every 100 ms.

- [ ] **Step 2: Create receiver initialization**

Main setup order must be:

```cpp
WiFi.mode(WIFI_AP_STA);
WiFi.softAP(ssid, password, WIFI_CHANNEL);
esp_now_init();
esp_now_register_recv_cb(...);
```

Provide Arduino-ESP32 2.x and 3.x receive-callback signatures behind version conditionals so either supported core can compile.

- [ ] **Step 3: Validate packets before updating safety state**

Reject null data and any length other than `sizeof(HelmetMessage)`. Copy bytes with `memcpy`, then invoke the registered application callback with `message.isTriggered` and `millis()`.

- [ ] **Step 4: Copy and minimally update the helmet sketch**

Preserve the three-sensor five-second logic and 100 ms transmission period. Add channel 1 selection before ESP-NOW initialization and set `peerInfo.channel = WIFI_CHANNEL` instead of zero.

- [ ] **Step 5: Run contract tests**

Expected: channel, payload, timeout, and send-period checks PASS.

### Task 6: Implement LittleFS Web API and Atomic HTML Upload

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/WebInterface.h`
- Create: `work/esp32-balance-controller/BalanceController/WebInterface.cpp`
- Modify: `work/esp32-balance-controller/tests/test_contracts.py`

**Interfaces:**
- Consumes: callbacks for current dashboard state, motor commands, and encoder calibration.
- Produces: routes listed in the spec and `WebInterface::begin()`, `handleClient()`.

- [ ] **Step 1: Add failing endpoint and upload-safety assertions**

Test for every exact route, `HTTP_POST` on mutations, `262144` byte limit, temporary `/index.tmp`, final `/index.html`, and a recovery-page string containing an upload form.

- [ ] **Step 2: Implement LittleFS mount and root serving**

Call `LittleFS.begin(false)` so mount failure never formats automatically. Serve `/index.html` with UTF-8 `text/html`; if missing, serve a compiled recovery page that links to or contains the upload form.

- [ ] **Step 3: Implement `/data` with a stable JSON schema**

Return these exact top-level fields:

```json
{
  "lc1":0.0,"lc2":0.0,"lc3":0.0,"lc4":0.0,"x":0.0,"y":0.0,
  "people":-1,
  "helmet":"signal_lost",
  "motorAllowed":false,"lockReason":"helmet_signal_lost",
  "motorState":"locked","targetRpm":0,"actualRpm":0.0,"pwmPercent":0.0,
  "encoderCalibrated":false,"pulsesPerRev":0,
  "fault":""
}
```

Build the JSON from a copied snapshot so asynchronous helmet updates cannot create a partially inconsistent response.

- [ ] **Step 4: Implement command endpoints with validation**

Use HTTP 200 for accepted commands, 400 for malformed values, 409 for state/safety rejection, and JSON `{ "ok": false, "reason": "..." }`. Parse `/motor/rpm` as an integer and reject missing, nonnumeric, negative, or greater-than-300 input without altering the target.

- [ ] **Step 5: Implement calibration endpoints**

Reject start/finish unless the motor output is fully stopped. Return the saved `pulsesPerRev` after successful finish and leave prior valid calibration unchanged after an invalid attempt.

- [ ] **Step 6: Implement atomic HTML upload**

Accept only a file named with `.html`, stop at 262144 bytes, write `/index.tmp`, close it, delete the old `/index.html` only after a successful completed write, then rename the temporary file. On abort or error, close and remove only `/index.tmp`; never remove the working page first.

- [ ] **Step 7: Run full contract tests**

Expected: all route, schema, method, validation, recovery, and atomic-upload checks PASS.

### Task 7: Build the Replaceable Dashboard

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/data/index.html`
- Modify: `work/esp32-balance-controller/tests/test_contracts.py`

**Interfaces:**
- Consumes: `/data` JSON and all POST routes from Task 6.
- Produces: responsive display and controls; no firmware authority.

- [ ] **Step 1: Add failing DOM contract assertions**

Require IDs `helmetStatus`, `peopleCount`, `motorAllowed`, `lc1`, `lc2`, `lc3`, `lc4`, `dot`, `motorToggle`, `rpmSlider`, `targetRpm`, `actualRpm`, `motorState`, `estopButton`, `faultReset`, `calibrationStart`, `calibrationFinish`, and `htmlUpload`.

- [ ] **Step 2: Create semantic responsive layout**

Use plain HTML/CSS/JavaScript without CDN dependencies so the page works on the offline ESP32 AP. Preserve the four-quadrant board visualization and center-of-gravity dot. Add status cards and a motor panel sized for mobile touch.

- [ ] **Step 3: Render state from `/data` every 100 ms**

Map helmet values to Korean labels, display collecting state for `people == -1`, update LC values and dot, show target/actual RPM, and show the firmware-provided lock reason. Disable ON unless `motorAllowed && motorState == "ready"`; disable the slider unless state is `armed` or `running`.

- [ ] **Step 4: Implement explicit control requests**

ON sends `/motor/on`, OFF sends `/motor/off`, slider sends debounced `/motor/rpm?value=N`, emergency stop sends `/motor/estop`, and reset sends `/motor/reset`. After every response, refresh `/data`; display rejected command reasons visibly.

- [ ] **Step 5: Add encoder calibration and HTML upload UI**

Calibration instructions must say: stop motor, lift wheel, press start, rotate exactly one output revolution, press finish. Upload uses multipart POST to `/upload`, accepts `.html`, and reloads `/` only after a successful response.

- [ ] **Step 6: Run contract tests and manually inspect the HTML in a local browser if available**

Expected: DOM/API contract tests PASS; browser has no external-resource requests and no JavaScript console syntax errors.

### Task 8: Integrate Existing Load Cells and Classifier in the Main Sketch

**Files:**
- Create: `work/esp32-balance-controller/BalanceController/BalanceController.ino`
- Reference: the supplied source attachment containing `1.ino`.
- Modify: `work/esp32-balance-controller/tests/test_contracts.py`

**Interfaces:**
- Consumes: all services from Tasks 2–7 and `score(double*,double*)` from `model.c`.
- Produces: complete `setup()` and nonblocking `loop()`.

- [ ] **Step 1: Add failing integration assertions**

Require main source to call `safety.updatePeople`, `motor.tick`, `encoder.tick`, `web.handleClient`, and preserve `score(features, output_scores)`, `DATA_INTERVAL = 50`, four HX711 pins, and all 16 feature indices.

- [ ] **Step 2: Port the supplied sensor and model code without changing feature order**

Copy the four HX711 setup, 20-sample history buffer, feature calculation, classifier call, percentage calculation, and center-of-gravity calculation. Keep the existing classifier's 16 feature indices exactly 0 through 15.

- [ ] **Step 3: Initialize subsystems in a boot-safe order**

Order: serial; motor output disabled; encoder; load cells; `WIFI_AP_STA` and channel-1 AP; ESP-NOW receiver; LittleFS/web. Do not enable BTS7960 during setup.

- [ ] **Step 4: Implement a nonblocking main loop**

Every iteration handles web clients, encoder, safety evaluation, and motor tick. Load cells/classifier update on the existing 50 ms schedule. ESP-NOW receive callbacks only copy/update small state and never perform web or motor I/O.

- [ ] **Step 5: Build one immutable dashboard snapshot per `/data` request**

Snapshot fields and names must match Task 6. `people` remains `-1` until the classifier buffer is filled. Helmet callback data is converted to worn/not-worn/signal-lost through `SafetyGate`, not read directly by HTML.

- [ ] **Step 6: Run all Python contract and reference tests**

Run: `python -m unittest discover -s work/esp32-balance-controller/tests -p "test_*.py" -v`

Expected: PASS with no missing file, constant, endpoint, DOM, or integration contracts.

### Task 9: Compile, Document Hardware Limits, and Package

**Files:**
- Create: `work/esp32-balance-controller/README.md`
- Copy verified package to: `outputs/esp32-balance-controller/`

**Interfaces:**
- Consumes: complete project.
- Produces: user-facing Arduino folders, instructions, test evidence, and explicit unverified hardware items.

- [ ] **Step 1: Search for an installed Arduino toolchain outside PATH**

Check common Windows locations for Arduino IDE 2 bundled `arduino-cli.exe` and ESP32 packages. Do not install or download anything silently.

- [ ] **Step 2: Compile when a usable toolchain is available**

Use the discovered ESP32 FQBN matching the user's board. A typical command shape is:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 work\esp32-balance-controller\BalanceController
arduino-cli compile --fqbn esp32:esp32:esp32 work\esp32-balance-controller\HelmetTransmitter
```

Record the exact FQBN, Arduino-ESP32 version, exit code, flash/RAM usage, and any warnings. If the board variant cannot be discovered, do not invent it; report compilation as blocked on the exact board selection.

- [ ] **Step 3: If no toolchain exists, run the strongest available verification and label compilation unverified**

Run all Python tests, calculate SHA-256 hashes for deliverable sources, and search for blocking delays in the main control path. A missing compiler is not permission to claim the sketch compiles.

- [ ] **Step 4: Write README setup instructions**

Include exact wiring, shared-ground warning, separate 12 V motor power, channel 1, AP connection, initial firmware upload, LittleFS fallback/upload flow, one-turn encoder calibration, low-speed first run, PID tuning caution, ON/slider/OFF behavior, two-second safety ramp, and immediate emergency-stop/fault behavior. State that the AP password belongs in ignored local configuration, must be at least eight characters, and must not be published or reused.

- [ ] **Step 5: Perform final spec-to-package audit**

Check every spec section 1–15 against a file or test. Confirm `model.c` hash matches the source unless a documented compatibility edit was required. Confirm no temporary upload file, generated binary, credentials file, or unrelated user file enters the package.

- [ ] **Step 6: Package verified sources**

Copy the complete `BalanceController`, `HelmetTransmitter`, tests, README, design spec, implementation plan, and verification log to `outputs/esp32-balance-controller/`. Re-run tests against the packaged copy and report software-verified versus hardware-unverified results separately.
