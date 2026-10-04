# BeyLit OLED and single-servo bench test

This firmware initializes a user-identified SSD1306 128x64 I2C OLED and provides a manually triggered SG90 test. It is not obstacle-awareness firmware yet. No automatic braking. ToF model remains pending. MPU6050 is only address-scanned, not initialized for measurements.

## Connections

ESP32 GPIO21 to OLED SDA, GPIO22 to OLED SCL, GND to OLED GND. User reports OLED VCC on 3V3; the exact breakout electrical specification and physical pin order have not been independently verified. Do not infer supply ratings from the controller name.

SG90 signal to GPIO18. User reports servo red lead supplied by USB-powered Uno 5V. Servo ground, Uno GND and ESP32 GND must share ground. Do not connect Uno 5V to ESP32 VIN/3V3 while ESP32 uses USB. Uno/USB servo supply capability remains unverified; a suitably rated regulated 5V servo branch is the intended final supply. GPIO19 is unused. No bracket load during servo test.

## Build, upload, monitor

Run from this project in a PlatformIO terminal:

```powershell
pio run -e esp32dev
pio run -e esp32dev -t upload --upload-port COM5
pio device monitor -p COM5 -b 115200
```

COM5 was previously observed; recheck with `pio device list` if it changes. Close other serial monitors before uploading. If pio is not on PATH, substitute `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe"` for pio.

## OLED test first

Disconnect servo power first. Upload and reset. Firmware probes 0x3C and 0x3D and initializes only if exactly one responds. Expected display: `BeyLit - BENCH TEST`, `SG90 OFF / SERIAL`, advisory-only wording and advancing uptime. Pass: stable legible text and uptime for at least one minute. This requires visual confirmation: initialization return value and address ACK do not prove the screen works.

If no ACK: power off and check the actual connector labels, SDA/SCL, ground and verified supply. If ACK but blank or distorted: verify the stated controller/resolution, supply and connections. Do not randomly raise supply voltage. `OLED RETRY` retries initialization. Both OLED addresses responding is treated as ambiguous.

## Servo test separately

Remove bracket load and keep the horn clear. Initial centering can jump because physical position is unknown. Power off before changing wiring. Verify common ground and servo supply before powering.

Type uppercase `SERVO TEST` and press Enter in Serial Monitor. Servo is attached at 50Hz, commanded to 1500us, then slowly traverses 1450 -> 1550 -> 1500us. This is a small pulse-width test, not calibrated degrees. Signal detaches at completion or after ten seconds. `SERVO OFF` stops pulses immediately when processed; it does not disconnect servo power. No automatic test at boot.

Pass: visible small movement in both directions and return, with no ESP32 resets, display corruption or mechanical contact. If no movement: check GPIO18, common ground, lead identification and supply voltage under load. Software cannot detect servo motion, stall or supply failure. Stop and disconnect power if it buzzes against a stop or resets occur.

Commands: HELP, STATUS, SCAN, OLED RETRY, SERVO TEST, SERVO OFF. SCAN and OLED RETRY are rejected during servo motion. OLED writes are bounded by Wire transaction timeouts but the display library does not report each write's result.

## Versions and validation

Pinned platform: espressif32 7.1.3; installed Arduino framework package 4.20017.260907+sha.dcc1105b. SSD1306 2.5.13, GFX 1.12.6, BusIO 1.17.4, ESP32Servo 0.13.0. This older ESP32Servo is used with the installed Arduino 2.x framework. Both drivers are compiled into this firmware; their hardware behavior remains untested.

Original scanner is retained in src/i2c_scanner.cpp.txt (not compiled).

Hardware uploads, OLED appearance, servo motion and power adequacy have not been tested by the agent. Record actual results before continuing to distance sensing or combined scanning.

## OLED corruption investigation

User photo showed a partial line and extensive corrupted pixels with SSD1306 mode. Controller identity is now under investigation; SH1106 is a diagnostic candidate, not a confirmed identification. Adafruit SH110X 2.1.12 is installed alongside SSD1306. Commands `OLED SH1106` and `OLED SSD1306` select an explicit driver for visual comparison, with no automatic model detection. User reported the display worked in SH1106 mode. Firmware now defaults to SH1106 on every boot. Serial mode changes are temporary.

The diagnostic draws a 128x64 border, mode label, advisory text and uptime. Pass requires all lines and the border to be legible without corruption for at least a minute. ACK and begin() results alone do not verify display operation. Neither display() API reports framebuffer transfer success.

Executed: PlatformIO build and upload to ESP32 COM5 both succeeded. Sent `SERVO OFF` followed by `OLED SH1106`; Serial reported SH1106 diagnostic initialization at 0x3C and SG90 OFF. User subsequently reported the display worked in this mode. Previous SG90 command test produced physical movement per user report. Do not claim OLED corruption resolved until visually confirmed.

Driver API reference: https://adafruit.github.io/Adafruit_SH110x/html/class_adafruit___s_h1106_g.html

SH1106 boot default was built and uploaded successfully to ESP32 COM5 after reconnection. Servo remains detached at startup. Current physical display appearance still requires user confirmation.

## Distance sensor identification result

Executed I2C scan: ACK at 0x29, 0x3C and 0x68. Added `TOF ID` command with checked I2C transfers and bounded Wire timeout. Build/upload succeeded, then actual hardware returned C0=EE, C1=AA, C2=10 at 0x29. These match the ST VL53L0X documented fresh-reset signature: https://www.st.com/resource/en/datasheet/vl53l0x.pdf . Combined with the VL53-looking board marking, this supports selecting the VL53L0X driver for the next stationary measurement milestone. This does not verify the breakout supply specification, distance accuracy or ranging status. No range measurements have yet been taken. Servo remained OFF throughout identification. User supplied a subsequent photo showing all OLED test lines and border legible in SH1106 mode.

## M3 stationary VL53L0X measurement

Installed pinned Adafruit_VL53L0X 1.2.4. Firmware starts continuous ranging at 100ms with a 33000us timing budget. It polls completion cooperatively, accepts only RangeStatus 0 plus API success and non-sentinel positive distance, and marks failed/invalid/timed-out data unknown. No median filter or obstacle thresholds are used in this measurement milestone. A 500ms acquisition deadline disables ranging after timeout; reset to retry. Wire transactions have a 25ms timeout; library initialization can still take time through its internal bounded polling. SERVO TEST is disabled for stationary measurement. Earlier servo test code is retained for a later milestone.

Executed build/upload: `platformio run -e esp32dev -t upload --upload-port COM5`, success. Captured actual serial readings in logs/m3_stationary_initial.txt: valid distance values 28–35mm, RangeStatus=0 and API status=0. This is approximately 3cm, not the proposed 30cm target distance. User target placement/direction and ruler agreement remain unverified; do not call accuracy test passed.

User steps: keep sensor still with black optical window facing a large matte flat target, not facing the table or a nearby wire/bracket. Measure from the window to the target at 100, 300 and 600mm using a ruler. Hold each position and record displayed minimum/maximum and ruler distance. Pass for this initial demonstration: stable valid readings that track moving target distances; use a provisional ±20mm at these distances for the bench comparison, not as a manufacturer accuracy guarantee. If readings stay around 30mm, check physical direction, nearby obstruction and any protective film without touching/scratching the optical window. If invalid: check target size/material/direction and bus connections with power removed. Dark/reflective/angled target tests and disconnect/stale/fifteen-minute tests remain pending. OLED now shows raw distance or DISTANCE UNKNOWN. No left/right obstacle claims yet.

Driver reference: https://github.com/adafruit/Adafruit_VL53L0X

## Current milestone M4: IMU identification blocked

Current src/main.cpp is an IMU-only bench test; previous M3 distance firmware is preserved in src/m3_distance_test.cpp.txt. No wire changes needed. Servo pins are held LOW and no PWM is attached. Distance ranging is paused in this firmware.

Installed Adafruit MPU6050 2.2.9 and Unified Sensor 1.1.15. Build and upload to COM5 succeeded. Hardware initialization failed. Checked read of WHO_AM_I register 0x75 at 0x68 returned 0x70, not expected MPU6050 0x68. At 0x69 no ACK. Evidence: logs/m4_imu_identification.txt. Do not bypass the identity check or claim working MPU6050 calibration. Exact chip must be verified; board silkscreen alone was insufficient. Pitch/roll remain null and OLED shows IMU ERROR. Calibration, complementary filter, movement rejection and tilt response are implemented but unexecuted due to initialization failure.

If exact supported MPU6050 is connected, firmware configures +/-2g, +/-250deg/s and 21Hz filter, then one-second settling plus 150 samples at nominal 20ms intervals. It retries detected motion up to 3 attempts within 15sec, records gyro bias and reference pitch/roll. Readings use a checked burst because library 2.2.9 getEvent ignores burst-read failure. Acceleration is converted with 16384 LSB/g and gyro with 131 LSB/(deg/s). Simple small-angle filter uses measured dt, alpha=0.5/(0.5+dt), rejects gaps above 250ms, and skips accelerometer correction outside 0.85–1.15g. OLED transfer creates sample gaps at 100kHz; actual sample rate is lower than 50Hz. Board X gyro -> roll and Y -> pitch; physical chair axes remain unverified. This is not drift-free yaw or precise terrain classification.

Commands: HELP, STATUS, CALIBRATE, IMU CHECK. On successful calibration, display/serial should show reference-relative pitch/roll near zero at rest. Gentle tilt testing, direction verification, failed calibration, unplug/recovery and long-run tests remain pending. Do not tug mounted boards or wires; wait for mechanical assistance before moving them. Build/upload commands remain those documented above.

## M4 MPU6500 diagnostic update

User authorized proceeding after steady power LED confirmation. Current source uses a small checked Wire register adapter for the repeatedly observed WHO_AM_I=0x70, not a modified MPU6050 library. It rejects other IDs. This is an MPU6500-compatible diagnostic; faint chip markings still prevent physical part authentication. Configuration checked by write/readback: PWR_MGMT_1=01, PWR_MGMT_2=00, CONFIG=03, GYRO_CONFIG=00 (+/-250deg/s), ACCEL_CONFIG=00 (+/-2g), ACCEL_CONFIG2=03, SMPLRT_DIV=09 (100Hz sensor output). Gyro and accelerometer DLPF settings are the MPU6500-specific 03 values. Reference register definitions: https://raw.githubusercontent.com/bolderflight/invensense-imu/main/src/mpu6500.h and https://raw.githubusercontent.com/bolderflight/invensense-imu/main/src/mpu6500.cpp ; manufacturer RM-MPU-6500A-00.

Build/upload succeeded to COM5. Actual checked burst returned accel_mps2=7.462,-6.431,0.426 and gyro_radps=0.0077,-0.9944,-0.4469 during user handling. Calibration rejected pose with accelerometer pitch=-47.9deg and roll=-97.1deg. Final message REST BOARD FLAT distinguishes excessive initial tilt from movement. These are uncalibrated diagnostic values, not accurate reference-relative tilt results. Servo remains OFF and compensation OFF. Logs: logs/m4_mpu6500_calibration.txt and logs/m4_mpu6500_pose.txt. Calibration/filter output/tilt response still pending.

Next user action: rest board flat on a dry nonconductive surface, components up, jumper leads slack. Keep still and request CALIBRATE through Serial (agent can send it). No need to unplug or tug jumper connections simply to rest a board if exposed contacts are kept away from metal. If wire tension prevents resting it, wait for help. Commands now also include RAW for checked uncalibrated acceleration/gyro diagnostics. MPU6050 library stays pinned for the saved earlier diagnostic; current source does not use it. Do not characterize failed calibration as success.
M4 actual calibration succeeded after user placed chip side up on cardboard box: 150 samples, reference pitch 32.92deg and roll -2.39deg; gyro biases rad/s -0.03280,0.07685,-0.01127. Relative READY output stayed approximately -0.05 to -0.01deg pitch and -0.07 to +0.01deg roll in the short captured interval. These are relative-to-reference values; absolute tilt accuracy and physical axis response are not verified. The 32.92deg reference indicates this position is not an established level reference. Evidence: logs/m4_chip_up_calibration.txt. Servo and compensation remained OFF.
User reports a ruler comparison matching approximately 400mm; this is user-observed, not an agent-verified accuracy measurement. Re-enabled manually triggered SERVO TEST for the requested mounted servo check. Distance readings are marked unknown during motion and for 500ms afterward; measurements continue to drain. Build/upload succeeded. Test commands and outputs are saved in logs/m5_mounted_servo_test.txt; physical motion must be confirmed by user. Servo remains OFF on startup.

## 2026-10-04 Claude session: MPU-controlled servo movement confirmed

Diagnosis summary (logs/m6_* .. m11_*):
- Libraries were not the cause. Earlier "no movement" tests were explained by: tilt saturating the tiny +/-50 us command at ~5 deg, tests started near the 30 deg cutoff, and the 0.85..1.15 g acceleration gate stopping COMP on hand jolts (previously one vague stop message).
- Added: SERVO TEST in current firmware, 100 ms TRACE lines, specific stop reasons, COMP ON refusal above 30 deg.
- Faster response: slew limited by elapsed time at 0.4 us/ms (full 100 us span ~0.25 s) instead of 2 us per loop update.
- Acceleration gate: brief excursions now hold the last command (tilt continues from gyro); COMP stops only if the excursion lasts 3 s. 30 deg limit, 60 s timeout, stale-data checks unchanged.
- Servo signal moved to GPIO27 (default; `SERVO PIN 18` / `SERVO PIN 27` switch at runtime, not persisted). After a swap test with a second SG90 (which never moved on either pin; possibly reversed plug) the original servo stopped responding on the GPIO18 path. GPIO18 itself is unverified, not proven damaged. Second servo set aside.
- Boot default axis is now PITCH.
- Servo supply measured by user: 4.9 V at the servo leads, idle.

Result: logs/m11_comp_pitch_hold3s.txt, a full 45 s COMP run, tilt -9.4..+4.8 deg, command 1461..1550 us following opposite to tilt. User physically confirmed the horn turned one way for one tilt direction and the other way for the opposite direction. Correct leveling sign, mechanical axis alignment, gain and range remain unverified.

Test method: tilt the cardboard base ~1 cm at one end. Do not lift the 2 cm MPU board itself by 1 cm (about 30 deg).
Rejected: pasted code using Adafruit_MPU6050 begin(); the chip reports WHO_AM_I=0x70, so that library's identity check fails (see m4 logs).

## Verdict modes (current firmware, src/main.cpp)

BeyLit is an ADVISORY display. A mode tells the person what the sensors see; it never controls a motor, brake or steering. "STOP MOTOR" is on-screen text only. Every limit below is an UNVALIDATED PROTOTYPE LIMIT for a tabletop demo, not a braking distance or certified threshold.

Inputs: one fixed forward VL53L0X distance reading (no scanning, so only straight ahead) and MPU6500-compatible pitch/roll relative to the last calibration pose. The MPU may be mounted flat or upright: calibration takes the measured gravity direction as level (mount-frame rotation), so keep the base level and still while it calibrates.

Owner decision (2026-10-04): the top level is labelled **SAFE**, replacing CLEAR and the original requirement to never show "safe". SAFE means only "a fresh valid reading puts the nearest object straight ahead beyond 40 cm, the board is level, and every input is valid". It says nothing about sides, behind, drop-offs, or objects the ToF cannot return from. A missing ToF return is never shown as SAFE.

| Mode (big OLED text) | Meaning | Enters when | Reason line | Screen | Onboard LED (GPIO2) |
|---|---|---|---|---|---|
| **STOP MOTOR** | Object very close straight ahead; person should stop | fresh valid distance < 300 mm (stays until >= 320 mm) | `OBJECT < 30 CM` | inverted, blinks ~2 Hz | 150 ms period (fastest) |
| **ASSIST** | Steep tilt; person may need help | pitch >= 10 deg or roll >= 8 deg (stays until < 9 / < 7 deg) | `SLOPE` or `SIDE TILT` | inverted | 400 ms |
| **?UNKNOWN** | System cannot see: input missing, invalid, stale or uncalibrated | ToF or IMU fault, servo moving, calibrating, other invalid range statuses | see table below | normal, "?" prefix | 700 ms |
| **CAUTION** | Something worth attention | distance 300-400 mm (stays until >= 420 mm), or pitch >= 5 deg / roll >= 4 deg (stays until < 4 / < 3 deg) | `OBJECT NEAR`, `SLOPE`, `SIDE TILT` | normal | 1 s |
| **NO TARGET** | ToF working but no return: nothing within its range, OR a dark/glossy/angled object it cannot see | ToF healthy, latest result fresh, RangeStatus 2 (signal fail) or 4 (phase fail) | `BEYOND RANGE` | normal | 2 s |
| **SAFE** | No object within 40 cm straight ahead, level, all readings valid | valid distance >= 400 mm (>= 420 mm when leaving CAUTION) AND pitch < 5 AND roll < 4 deg AND IMU calibrated | `NO OBJECT < 40 CM` | normal | off |

### Priority and timing

- Ranking, worst first: STOP MOTOR > ASSIST > ?UNKNOWN > CAUTION > NO TARGET > SAFE. The worst of the distance and tilt results is shown. A known STOP MOTOR or ASSIST outranks an unknown input; an unknown input outranks CAUTION, NO TARGET and SAFE.
- Worse modes appear immediately. Better modes must hold for 300 ms before the display downgrades.
- Hysteresis: 20 mm for distance, 1 deg for tilt.
- Dropout hold: after a valid reading, no-return results keep that measured level (STOP MOTOR, CAUTION or SAFE) for up to 2 s, then NO TARGET. SAFE always comes from a real reading no older than 2 s.
- ToF runs the library LONG_RANGE preset (signal-rate limit 0.1 MCPS, sigma 60 mm) with a 100 ms timing budget so valid readings beyond 40 cm are possible; readings are slightly noisier.
- A distance reading counts only if RangeStatus 0, API success, and younger than 300 ms. Readings taken within 150 ms of servo movement are rejected (the sensor rides on the servo horn).

### ?UNKNOWN reasons

| Reason line | Cause | What to do |
|---|---|---|
| `NO DIST READING` | invalid range status other than 2/4 (e.g. sigma or min-range fail) | reposition target; never treat as safe |
| `SERVO MOVING` | horn moving, readings rejected | wait for leveling to settle |
| `CHECK SENSOR` | ToF or IMU not responding (unplugged, bus error, timeout) | check wiring; firmware retries every 2 s |
| `CALIBRATING` | IMU collecting its level reference | keep the board still |
| `RECALIBRATE IMU` | calibration failed (board moved during calibration) | lay flat and keep still; firmware retries calibration every 5 s |
| `STARTING` / `WAITING` | just booted | wait |

### Other screen lines

Line 2 `Pitch +x.x Roll +x.x` (or `--`), line 3 `Dist NN cm` (or `Dist --`) plus leveling state `LVL ON` / `LVL WAIT` (auto mode paused by a safety condition) / `LVL OFF` / `SRV TST`, line 4 reason, bottom `ADVISORY-DEMO LIMITS`.

### Leveling (servo)

Auto mode by default: starts 1 s after calibration, no time limit, range 1350-1650 us (about +/-15 deg horn travel, 10 us per degree of pitch). Pauses on tilt > 30 deg, IMU fault, or acceleration outside 0.85-1.15 g for 3 s; resumes after 1 s of normal conditions. `COMP OFF` / `LEVEL OFF` disable, `LEVEL AUTO` re-enables. The servo follows ONE axis: ROLL by default (the MPU axis that matches this mount's tilt direction, user-confirmed 2026-10-04: horn followed both ways); `AXIS PITCH` switches at runtime. One servo cannot level both axes.

## Local web mirror (owner decision 2026-10-04)

Changes the original "no Wi-Fi dashboard" scope at the owner's request. The ESP32 runs its own Wi-Fi access point; no router or internet is involved.

- Join Wi-Fi `BeyLit-Display`, password `beylit123`, then open `http://192.168.4.1` (phones may warn "no internet"; stay connected).
- The page (src/web_page.h) mirrors the OLED: verdict word, pitch/roll, distance, leveling state, reason. ASSIST inverts; STOP MOTOR inverts and blinks.
- It polls `/status` (JSON) every 250 ms. After 1.5 s without an update it shows `?UNKNOWN / NO LINK TO ESP32`, never the last verdict.
- Read-only: no control endpoints. Serial commands remain the only way to change behaviour.
- Wi-Fi adds current peaks on the ESP32 USB supply; servo power is separate (Uno 5V). Watch for resets.
