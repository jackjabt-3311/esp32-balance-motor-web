# ESP32 Balance Controller

This project combines four HX711 load cells, the supplied passenger classifier,
a helmet interlock over ESP-NOW, a local ESP32 access-point dashboard, and a
one-direction BTS7960 motor controller. The firmware is the safety authority:
the HTML dashboard cannot bypass a helmet, passenger, calibration, or motor
fault lockout.

> **Safety notice:** This is control firmware for a moving motor. Test with the
> driven wheel raised, power available to an immediately accessible disconnect,
> and no person in the motion path. Compilation and software tests do not prove
> motor direction, sensor polarity, encoder scaling, PID stability, or physical
> emergency-stop behavior.

## Required hardware and wiring

Use an ESP32 with the standard `esp32:esp32:esp32` board definition, four HX711
modules, a BTS7960 driver, a 12 V JGA25-371-style one-direction motor, and a
Hall encoder. Power the ESP32 and all logic from an appropriate regulated logic
supply. Use a **separate 12 V motor supply** for the BTS7960/motor power path.

All logic devices must share one common ground: ESP32 GND, every HX711 GND,
encoder GND, BTS7960 logic GND, and the 12 V supply negative must be connected
at the intended common-ground point. Do not feed 12 V into an ESP32 GPIO, HX711,
or Hall sensor. Supply the Hall encoder at **3.3 V**, not 5 V, so its outputs
are safe for ESP32 GPIO.

| Function | ESP32 pin | Connect to |
| --- | ---: | --- |
| Load cell 1 | DOUT 34, SCK 16 | HX711 #1 DT/SCK |
| Load cell 2 | DOUT 36, SCK 18 | HX711 #2 DT/SCK |
| Load cell 3 | DOUT 35, SCK 17 | HX711 #3 DT/SCK |
| Load cell 4 | DOUT 39, SCK 19 | HX711 #4 DT/SCK |
| Motor forward PWM | 25 | BTS7960 RPWM |
| Motor reverse PWM | 26 | BTS7960 LPWM (kept at zero) |
| Motor right enable | 27 | BTS7960 R_EN |
| Motor left enable | 14 | BTS7960 L_EN |
| Encoder Hall A | 32 | Encoder A |
| Encoder Hall B | 33 | Encoder B (direction diagnostic) |
| Helmet sensor 1 | 21 | Helmet transmitter input 1 |
| Helmet sensor 2 | 35 | Helmet transmitter input 2 |
| Helmet sensor 3 | 33 | Helmet transmitter input 3 |

The helmet transmitter uses a second ESP32. Its input pins belong to that
second board; they are not additional connections to the main controller.

## Before the first upload

1. Install Arduino IDE/CLI with the ESP32 core and the HX711 library used by the
   sketch. Select the actual board and port; `esp32:esp32:esp32` is the verified
   generic-board build target, not a substitute for confirming your hardware.
2. In `BalanceController`, copy `LocalConfig.example.h` to `LocalConfig.h` and
   replace `AP_PASSWORD` with a private password of at least eight characters.
   `LocalConfig.h` is ignored by Git and must not be committed or included in a
   shared package. Then open `BalanceController.ino` and upload it to the main
   ESP32. On boot the firmware explicitly holds the BTS7960 PWM and enable pins
   LOW before it starts sensors or networking.
3. In `HelmetTransmitter`, make a local copy of `LocalConfig.example.h` named
   `LocalConfig.h`, then replace its example `receiverMac` with the main ESP32's
   STA MAC. `LocalConfig.h` is ignored by Git and must never be committed or
   included in a shared package. Upload `HelmetTransmitter.ino` to the helmet
   ESP32.
4. Keep both boards on Wi-Fi **channel 1**. The main firmware starts the local
   AP using the SSID and private password from `BalanceController/LocalConfig.h`.
   Never publish or reuse that password on another service.

## Dashboard and LittleFS page

After the main board starts, join the `ESP32_Balance` Wi-Fi network from a phone
or computer and open the AP address shown in the serial monitor (normally
`http://192.168.4.1/`). The page polls `/data` about every 100 ms; it only shows
firmware state and sends explicit POST commands.

`BalanceController/data/index.html` is the initial offline page. Upload it to
LittleFS with the data-upload method that matches your Arduino ESP32 setup when
you need the initial page. If the page is absent, the firmware serves a minimal
recovery page at `/` with an HTML upload form. Later visual-only changes do not
require reflashing: upload one `.html` file from the dashboard. The firmware
limits uploads to 256 KiB, writes `/index.tmp`, and promotes it only after a
completed upload. Before promotion it rejects empty files and checks only the
first 256 bytes for leading whitespace followed by `<!doctype html` or `<html`;
invalid uploads leave the working page in place. The root route applies the
same bounded check and serves recovery instead of a missing, empty, or invalid
active page.

Every POST result, including a rejected command, includes `{ ok, reason,
state }`. `state` is one captured firmware snapshot containing motor state,
target/actual RPM, ready/motor-allowed status, and fault. The dashboard still forces
a fresh `/data` read after every command, so a delayed response cannot
re-enable normal controls after ESTOP.

## Encoder calibration and safe first run

The firmware intentionally has no assumed pulses-per-output-revolution value.
With the motor fully stopped and the wheel raised:

1. On the dashboard, start encoder calibration.
2. Rotate the output wheel **exactly one full revolution by hand**.
3. Finish calibration and confirm a nonzero saved pulses-per-revolution value.
4. Confirm helmet status is worn, its ESP-NOW signal is fresh, and the classifier
   has reported exactly one passenger continuously for one second. Otherwise the
   motor remains locked.
5. Press **ON**. This only arms the motor at 0 RPM.
6. Move the RPM slider from a very low value upward while the wheel is still
   raised. The allowed range is 0–300 RPM. Compare measured RPM with an
   independent measurement if available.

The initial PID gains (`Kp=0.8`, `Ki=0.15`, `Kd=0.0`) are conservative starting
values only. Tune them slowly on the real mechanism after verifying motor
direction, load, encoder edges, and mechanical guarding; they are not
hardware-validated settings.

## Stops and lockouts

- **ON** arms at zero RPM; the slider starts or changes motion only while the
  state is armed/running. Setting 0 returns to armed without disarming.
- **OFF** and loss of a normal safety prerequisite perform a nonblocking,
  linear **2-second ramp-down**. A recovered prerequisite does not restart the
  motor; press ON again after the ramp completes.
- **ESTOP**, boot/reset, encoder-stall fault, and invalid control state stop
  PWM immediately, disable the BTS7960 enables, latch the state, and never
  auto-restart. Reset is accepted only while output is stopped, no ramp is in
  progress, and helmet-worn/fresh, exactly-one-passenger, calibrated
  non-calibrating encoder prerequisites are all valid. A motor fault never
  qualifies its own reset.
- Helmet packets time out after 500 ms. Passenger count must remain exactly one
  for 1000 ms. Calibration is required before arming. Any of these conditions
  locks or ramps down the motor in firmware regardless of what the browser UI
  displays.

## Software verification and scope

Run the repository checks from this directory:

```powershell
python -m unittest discover -s tests -p "test_*.py" -v
node --test tests/test_dashboard_js.js
```

With Arduino CLI 1.5.1, Arduino-ESP32 3.3.10, and FQBN
`esp32:esp32:esp32`, the final compile checks exited 0 for both sketches:
`BalanceController` used 1,030,731 / 1,310,720 flash bytes (78%) and 48,956 /
327,680 global bytes (14%); `HelmetTransmitter` used 898,652 / 1,310,720 flash
bytes (68%) and 45,392 / 327,680 global bytes (13%). No warnings were reported.

The checked classifier copy is SHA-256
`f6e136bd80e5003a7465616237539f3f37d3affbc3b7192fce4df2dd0182dd54`.
See `docs/verification.md` for the final spec audit, package contents, build
evidence, and the separate list of still-required physical checks.

## Repository layout

- `BalanceController/` — main firmware and replaceable LittleFS page.
- `HelmetTransmitter/` — helmet sensor sender; only the non-device
  `LocalConfig.example.h` is versioned.
- `tests/` — Python firmware contracts/reference models and Node dashboard test.
- `docs/` — approved design, implementation plan, and verification record.
