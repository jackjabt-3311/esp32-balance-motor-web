# Verification Record — ESP32 Balance Controller

Date: 2026-09-01

## Scope and toolchain

- Target FQBN: `esp32:esp32:esp32`.
- Installed Arduino-ESP32 core: 3.3.10.
- Arduino CLI: 1.5.1.

| Sketch | Exit | Flash | Globals | Remaining global RAM | Warnings |
| --- | ---: | --- | --- | --- | --- |
| `BalanceController` | 0 | 1,030,731 / 1,310,720 bytes (78%) | 48,956 / 327,680 bytes (14%) | 278,724 bytes | None reported |
| `HelmetTransmitter` | 0 | 898,652 / 1,310,720 bytes (68%) | 45,392 / 327,680 bytes (13%) | 282,288 bytes | None reported |

The helmet build used a temporary local `LocalConfig.h` copied from the tracked
non-device example solely to supply the compile-time receiver address. It was
deleted after the build and is not in the repository or package.

## Software checks

The package must be verified from its own root with:

```text
python -m unittest discover -s tests -p "test_*.py" -v
node --test tests/test_dashboard_js.js
```

The test suite includes source contracts for safety timing, motor command
states, ESTOP/fault behavior, encoder calibration, ESP-NOW channel/payload,
HTTP methods and JSON, atomic HTML upload, dashboard DOM/API behavior, no
blocking `delay()` in the main loop, the 16 classifier feature assignments, and
the classifier hash.

Fix round 1 added contracts that (1) preserve an ESTOP/fault latch unless the
fault-excluded `prerequisitesReady` conditions are met while stopped and not
ramping, (2) reject empty or unrecognizable HTML before promotion and fall back
to recovery for a missing, empty, or invalid active page, and (3) include one
captured `state` object in every command JSON response, including rejection.
The dashboard race test supplies such command responses and confirms it still
uses forced `/data` reconciliation rather than trusting a delayed response.

## Classifier integrity

`BalanceController/model.c` SHA-256:

```text
f6e136bd80e5003a7465616237539f3f37d3affbc3b7192fce4df2dd0182dd54
```

This equals the supplied-classifier hash asserted by `tests/test_contracts.py`.
No compatibility edit is documented for `model.c`.

## Spec audit (design sections 1–15)

| Design section | Evidence |
| --- | --- |
| 1 Goal | `BalanceController/BalanceController.ino`, dashboard, README |
| 2 Inputs/pins | main sketch, `MotorController.h`, `EncoderService.h`, README wiring table |
| 3 Architecture | main sketch services/adapters; design document |
| 4 Safety state machine | `SafetyGate.*`, `MotorController.*`, state-model/contract tests |
| 5 Helmet reception | `HelmetReceiver.*`, `HelmetTransmitter.ino`, channel/payload tests |
| 6 Passenger prerequisite | `SafetyGate.*`, main classifier update, timing tests |
| 7 Motor/RPM control | `MotorController.*`, `EncoderService.*`, motor contracts |
| 8 Encoder calibration | `EncoderService.*`, web endpoints, calibration contracts |
| 9 Web UI | `data/index.html`, Node dashboard test |
| 10 HTTP interface | `WebInterface.*`, route/schema/upload contracts |
| 11 LittleFS updates | `WebInterface.cpp`, `data/index.html`, recovery/upload contracts |
| 12 Timing/concurrency | main loop/mailbox, encoder/motor constants, integration contracts |
| 13 Deliverables | package manifest below |
| 14 Verification limits | this record and README physical checklist |
| 15 Scope boundaries | firmware constants/state rules; README |

## Package manifest and hygiene

The deliverable contains `BalanceController/`, `HelmetTransmitter/`, `tests/`,
`README.md`, `.gitignore`, and the three `docs/` records (design, plan,
verification). It must exclude `.git`, `.build`, `__pycache__`, generated
firmware binaries, LittleFS temporary/backup pages, and either sketch's
`LocalConfig.h`.

The package tests were run inside a temporary Git context because the existing
source-contract test intentionally checks `git ls-files`; that temporary `.git`
directory was deleted before package delivery. The retained `.gitignore` keeps
the real receiver configuration ignored for users who develop from the package.

The package audit also searches tracked/package text for local absolute paths
and device credentials. The two `LocalConfig.example.h` files contain only a
clearly marked non-device MAC and a change-me AP placeholder. Real receiver and
AP credentials remain only in ignored local configuration files.

## Fix-round host-test evidence

On 2026-09-01, the source and packaged copies each passed the non-compiler
Python contracts and the Node dashboard race test. The one separate Python test
that invokes the installed Xtensa compiler for a header-only syntax check was
deliberately not run in the fix rounds because compilation was reserved for the
parent task. Root then ran the final sequential Arduino CLI builds after the
firmware fixes: both sketches exited 0 with the flash/RAM values recorded
above and no warnings.

## Hardware still requiring physical verification

Compilation and host tests do **not** validate: wiring/common ground, 12 V motor
power isolation, boot output levels on the real board, motor polarity, Hall
signal levels and one-turn scaling, HX711 calibration/signs, classifier quality
on the installed platform, PID stability, the 2-second physical ramp profile,
or immediate physical ESTOP/fault output disable. Follow the raised-wheel,
low-speed checklist in README before normal use.
