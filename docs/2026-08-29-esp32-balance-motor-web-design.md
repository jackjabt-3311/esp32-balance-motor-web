# ESP32 Balance Board Web and Motor Control Design

Date: 2026-08-29

## 1. Goal

Extend the existing ESP32 balance-board firmware so that a local web page can:

- show helmet-wearing state received from a second ESP32 over ESP-NOW;
- show predicted passenger count and four-corner load distribution;
- arm and stop a one-direction DC motor driven by a BTS7960;
- set a target speed from 0 to 300 RPM and show measured RPM;
- replace the web UI by uploading `index.html` to LittleFS without reflashing firmware.

The motor may be armed only when the helmet is worn and the predicted passenger count has remained exactly one for one continuous second.

## 2. Existing Inputs

### Main ESP32

- Four HX711 load cells and the existing `model.c` classifier.
- Existing access point: `ESP32_Balance`.
- Existing JSON endpoint: `/data`.
- Existing load-cell pins remain unchanged.

### Helmet ESP32

- Sends this ESP-NOW payload every 100 ms:

```cpp
struct_message {
  bool isTriggered;
}
```

- Configure the main ESP32 receiver address as `<ESP32_STA_MAC_FROM_LOCAL_CONFIG>`.
  The real STA MAC belongs in an untracked local configuration, not this repository.
- Both boards will use Wi-Fi channel 1 so ESP-NOW and the main board's soft AP remain aligned.

### Motor and encoder

- Motor: 12 V JGA25-371, nominal no-load speed 350 RPM.
- Driver: BTS7960.
- One-direction operation only.
- Motor control pins:
  - RPWM: GPIO 25
  - LPWM: GPIO 26
  - R_EN: GPIO 27
  - L_EN: GPIO 14
- Encoder pins:
  - Hall A: GPIO 32
  - Hall B: GPIO 33
- Hall supply is 3.3 V and all logic grounds must be common.

## 3. System Architecture

The main ESP32 runs four cooperating parts:

1. Sensor and classifier loop: reads the four load cells, updates load percentages and center of gravity, and runs the existing passenger classifier.
2. Safety-state loop: receives helmet packets, applies signal freshness and passenger-count stability rules, and derives whether motor arming is allowed.
3. Motor-control loop: measures encoder RPM, runs closed-loop target-speed control, and drives the BTS7960.
4. Local web server: serves `index.html` from LittleFS, exposes state/control endpoints, and accepts a replacement HTML file.

The web page is presentation and user input only. Every safety rule is enforced again in firmware; changing HTML cannot bypass it.

## 4. Safety State Machine

### States

- `LOCKED`: motor stopped; start command unavailable because prerequisites are not satisfied.
- `READY`: helmet is worn, helmet signal is fresh, one passenger has been predicted continuously for one second, encoder is calibrated, and no motor fault exists.
- `ARMED`: user pressed ON; target RPM input is enabled. Initial target RPM is zero.
- `RUNNING`: armed with target RPM above zero; PID controls the motor.
- `RAMPING_DOWN`: target output is reduced linearly to zero over two seconds.
- `FAULT`: motor is immediately disabled and cannot restart until the fault is cleared while stopped.
- `ESTOP`: emergency stop is latched; motor is immediately disabled and requires an explicit reset while safety conditions are valid.

### Transitions

- `LOCKED -> READY`: all prerequisites become valid.
- `READY -> ARMED`: a valid ON request is received.
- `ARMED -> RUNNING`: target RPM becomes greater than zero.
- `RUNNING -> ARMED`: target RPM becomes zero without disarming.
- `ARMED/RUNNING -> RAMPING_DOWN`: normal OFF request or a safety prerequisite becomes invalid.
- `RAMPING_DOWN -> LOCKED/READY`: two-second ramp finishes. The motor never restarts automatically.
- Any active state -> `FAULT`: encoder stall or other detected motor-control fault.
- Any state -> `ESTOP`: emergency-stop request.

The ON button and RPM slider are disabled immediately when a prerequisite becomes invalid. The current PWM command then ramps down over two seconds. If prerequisites recover during ramp-down, the ramp continues to completion and the user must press ON again.

### Immediate-stop cases

- Emergency-stop request.
- ESP32 boot/reset, where output pins initialize disabled.
- Encoder-stall fault: meaningful PWM is commanded but no encoder motion is detected for the configured timeout.
- Internal invalid-state or invalid-control-data detection.

## 5. Helmet Reception

- The main board uses `WIFI_AP_STA`, starts its soft AP on channel 1, and initializes ESP-NOW on that channel.
- Incoming packets are accepted only when their payload length matches the expected structure.
- The last packet time is recorded for every valid packet.
- Helmet state is `WORN` only when `isTriggered` is true and the packet is fresh.
- If no valid packet arrives for 500 ms, the state becomes `SIGNAL_LOST` and the motor prerequisite becomes false.
- The supplied helmet firmware will be updated to force channel 1 and to avoid relying on an unspecified current channel.

## 6. Passenger Prerequisite

- `people == 1` must remain true continuously for 1000 ms before the prerequisite becomes valid.
- Any value other than one resets the qualification timer immediately.
- During the classifier's initial buffer-fill period, passenger state is unavailable and motor start is locked.
- A change away from one while armed or running initiates the two-second ramp-down.

## 7. Motor and RPM Control

- LPWM remains zero because only one direction is used.
- R_EN and L_EN are enabled only while the motor is armed, running, or completing a controlled ramp-down.
- RPWM is the controlled PWM output.
- The accepted target range is 0 to 300 RPM. Out-of-range and nonnumeric requests are rejected without changing the current target.
- ON does not cause motion by itself: it arms the controller with target RPM zero.
- The user moves the slider after ON to command a target RPM.
- OFF reduces the active PWM command to zero linearly over 2000 ms, then disables both enable outputs.
- The PID loop runs every 50 ms using measured encoder RPM. Initial gains are conservative constants and will require physical tuning under the real load.
- Encoder edges are counted in an interrupt-safe counter, while RPM calculation and PID run outside the ISR.
- An encoder-stall fault is raised when target RPM is at least 20, PWM duty is at least 25%, and measured RPM remains below 5 for one continuous second.

## 8. Encoder Calibration

The cited product listing does not specify encoder pulses per output-shaft revolution, so firmware must not assume a model-wide value.

Calibration is allowed only while the motor is stopped:

1. User starts calibration from the settings area.
2. Firmware clears a temporary encoder count.
3. User rotates the output wheel exactly one full revolution by hand.
4. User completes calibration.
5. Firmware validates that the count is nonzero and plausible, then stores pulses per output revolution in ESP32 Preferences/NVS.

Motor arming remains locked until a valid calibration value exists. The settings area displays the saved value and offers recalibration.

## 9. Web UI

The responsive local dashboard contains:

1. Safety status card
   - helmet: worn, not worn, or signal lost;
   - predicted passenger count or collecting state;
   - motor permission and a concise lock reason.
2. Load-distribution card
   - LC1 through LC4 percentages;
   - existing board visualization and center-of-gravity dot.
3. Motor-control card
   - ON/OFF button;
   - target RPM slider, disabled until armed;
   - target and actual RPM;
   - state label: locked, ready, armed, running, ramping down, fault, or emergency stop;
   - prominent emergency-stop button;
   - fault/reset action shown only when applicable.
4. Settings/editor area
   - encoder one-revolution calibration controls;
   - HTML upload form for replacing `/index.html`.

The page polls state at approximately 100 ms, matching the existing dashboard behavior. Control actions are explicit HTTP POST requests and the UI always renders the state returned by firmware rather than assuming a command succeeded.

## 10. HTTP Interface

- `GET /`: serve `/index.html` from LittleFS, with a minimal built-in recovery page if the file is absent.
- `GET /data`: return one JSON object containing load, center of gravity, people, helmet, prerequisites, motor state, target RPM, actual RPM, calibration, and fault information.
- `POST /motor/on`: arm only when firmware prerequisites are valid.
- `POST /motor/off`: initiate the two-second controlled ramp-down.
- `POST /motor/rpm`: accept a numeric target from 0 through 300 only while armed/running.
- `POST /motor/estop`: immediately disable the motor and latch emergency stop.
- `POST /motor/reset`: clear a resettable fault or emergency-stop latch only while stopped and under valid reset conditions.
- `POST /encoder/calibration/start`: begin stopped-state pulse counting.
- `POST /encoder/calibration/finish`: validate and persist the measured count.
- `POST /upload`: replace `/index.html` using a temporary file and rename-on-success so an interrupted upload does not destroy the working page.

Every state-changing endpoint returns JSON with success, current state, and an error reason when rejected.

## 11. LittleFS Update Behavior

- Firmware mounts LittleFS at boot without automatically formatting it on failure.
- The initial Arduino sketch upload includes `data/index.html` as the default page.
- Later visual changes use the browser upload endpoint and do not require Arduino IDE reflashing.
- Upload accepts HTML only, enforces a 256 KiB size limit, and writes to a temporary filename before replacing the active file.
- A built-in recovery page keeps `/upload` reachable if `index.html` is missing or invalid.

## 12. Timing and Concurrency

- Existing load-cell sampling remains approximately 20 Hz.
- Helmet packets arrive approximately every 100 ms.
- Browser state polling remains approximately every 100 ms.
- RPM is calculated every 100 ms, while the PID loop runs every 50 ms independently of browser traffic and without long blocking delays.
- The two-second ramp uses elapsed time, not blocking `delay`, so sensing, ESP-NOW, web requests, and emergency stop remain responsive.

## 13. Deliverables

- Main Arduino sketch with the existing load-cell/ML functions plus ESP-NOW, LittleFS, safety state machine, encoder measurement, PID, BTS7960 control, and web API.
- Existing `model.c`, unchanged unless compilation requires a compatibility adjustment.
- `data/index.html` dashboard.
- Updated helmet sketch with a fixed ESP-NOW channel and otherwise equivalent sensing behavior.
- Short setup and calibration instructions, including LittleFS upload workflow, wiring reminders, and hardware verification checklist.

## 14. Verification

Software-side checks:

- compile the sketch for the intended ESP32 Arduino core when the toolchain is available;
- confirm no GPIO conflicts with the existing HX711 pins;
- confirm all endpoints and JSON field names match the HTML;
- confirm every motor-start path rechecks firmware prerequisites;
- confirm ramp-down is nonblocking and emergency stop bypasses it;
- confirm invalid RPM and upload requests are rejected safely;
- confirm the page has a recovery path when LittleFS content is absent.

Hardware-side checks to be performed on the real system:

1. Raise the wheel so it can rotate without moving the platform.
2. Verify common ground, Hall 3.3 V supply, motor polarity, and BTS7960 power wiring.
3. Confirm outputs remain disabled at boot.
4. Calibrate one output-shaft revolution and verify displayed RPM with an independent measurement if available.
5. Start at a low target RPM and tune PID gains conservatively.
6. Test helmet loss and passenger mismatch; verify two-second ramp and locked controls.
7. Test emergency stop; verify immediate PWM disable.
8. Test encoder disconnection; verify immediate fault stop and no automatic restart.

Compilation cannot prove motor direction, encoder scaling, PID stability, or safe mechanical behavior. Those items remain explicitly hardware-unverified until the physical tests pass.

## 15. Scope Boundaries

- Local AP operation only; no cloud or internet remote control.
- One motor and one rotation direction.
- Maximum target 300 RPM.
- No automatic restart after any stop, ramp-down, fault, reset, or recovered prerequisite.
- HTML updates can change presentation and requests but cannot alter firmware safety logic.
