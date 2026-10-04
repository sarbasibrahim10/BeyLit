# BeyLit: complete handoff to Claude
Prepared 4 October 2026. Read this before the historical README, whose early sections are outdated. This is a continuation, not a new project.

## Start here
The user is a beginner and wants short, concrete steps explained simply. They can reposition hardware and report physical results. They often cannot type in the serial monitor and have authorized the agent to build, upload and send diagnostic commands. Do not confuse the VS Code read-only Output pane with the Terminal/serial monitor. Do not claim physical movement from software logs.

Immediate objective: make ONE SG90 on ESP32 GPIO18 respond opposite to MPU tilt, then verify its mechanical axis/sign for approximate leveling. User explicitly selected stabilization, not following tilt. Standalone servo motion worked; MPU-controlled motion has NOT been physically confirmed. User reported no motion in both roll and pitch tests. Their final technical question was whether all libraries were installed. Package inspection during this handoff confirms all eight configured libraries are installed. The current source builds successfully. Missing dependencies are not the demonstrated cause.

Handoff preparation sent COMP OFF and confirmed it in Serial. Latest runtime state: READY, axis=PITCH, sign=-1, compensation OFF. Last pitch was about45 degrees, so return to reference before enabling. Source boot default is ROLL: runtime axis changes do not persist across reset. No firmware edits or upload were made during handoff preparation.

## Goal and intended operation
BeyLit is a tabletop-first wheelchair-mounted obstacle-awareness hackathon prototype. It is advisory only: no motor driver, brake interface, automatic braking or autonomous steering. Never connect it to propulsion electronics. Tests should be tabletop or an unoccupied stationary wheelchair, not near stairs/traffic/drop-offs.

Original final arrangement:
1. VL53L0X distance sensor on a pan/tilt bracket.
2. Pan SG90 scans centre-left-centre-right.
3. A separate tilt SG90 compensates approximately for frame pitch using an IMU rigidly mounted on the fixed base.
4. OLED displays left/front/right obstacle distances and visual STOP advisories.
5. Measurements occur after movement settles, with valid/fresh data required; invalid results mean unknown, never clear.

Currently only ONE servo is being used, on GPIO18. Its current diagnostic role is tilt response although it was originally the pan servo. GPIO19 is unused. One servo cannot independently scan pan and compensate pitch. Two pan/tilt servos still do not provide independent roll stabilization. IMU alone does not provide drift-free absolute yaw. Simple pitch subtraction is approximate and geometry-dependent.

No runtime AI, cloud, Wi-Fi dashboard, Bluetooth, Raspberry Pi, mobile app, encoders, ultrasonic sensors or propulsion control. Claude/Codex helps develop deterministic firmware. Raspberry Pi boxes visible in photos are only supports. Website/demo page is a later deliverable after firmware works.

## Project and tools
Windows, PowerShell, VS Code + PlatformIO, Arduino C++.
Actual project: C:\Users\sarba\OneDrive\Documents\PlatformIO\Projects\BeyLit
The Codex workspace E:\BeyLim\Open-Ai is NOT the project.
Environment esp32dev; board ESP32-WROOM DevKit (flasher identifies ESP32-D0WD-V3).
ESP32 serial port observed repeatedly: COM5, Silicon Labs CP210x,115200 baud. Recheck after reconnection.
PlatformIO: C:\Users\sarba\.platformio\penv\Scripts\platformio.exe
Python with pyserial: C:\Users\sarba\.platformio\penv\Scripts\python.exe
Other seen COM ports were Bluetooth; don't select arbitrarily.
Uno used only for servo5V; it was not detected as a usable USB serial board. A D9 experiment was abandoned. Do not move control from GPIO18 to Uno D9.

Commands from project directory:
```powershell
& 'C:\Users\sarba\.platformio\penv\Scripts\platformio.exe' pkg list -e esp32dev
& 'C:\Users\sarba\.platformio\penv\Scripts\platformio.exe' run -e esp32dev
& 'C:\Users\sarba\.platformio\penv\Scripts\platformio.exe' run -e esp32dev -t upload --upload-port COM5
& 'C:\Users\sarba\.platformio\penv\Scripts\platformio.exe' device monitor -p COM5 -b 115200
```
Close competing serial monitors first. If Windows output encoding fails, set $env:PYTHONIOENCODING='utf-8'. Do not kill unrelated Python processes.

## Wiring and power
This is the established/user-reported pin plan, not an independent continuity audit. Exact breakout voltage specifications and every wire destination have not been independently measured.

| Device | Pin | Connection |
|---|---|---|
| IMU | VCC | ESP32 3V3 |
| IMU | GND | common GND |
| IMU | SDA | GPIO21 |
| IMU | SCL | GPIO22 |
| VL53L0X | VIN | ESP32 3V3, per existing plan |
| VL53L0X | GND | common GND |
| VL53L0X | SDA | GPIO21 |
| VL53L0X | SCL | GPIO22 |
| OLED | VCC | ESP32 3V3 |
| OLED | GND | common GND |
| OLED | SDA | GPIO21 |
| OLED | SCL | GPIO22 |
| SG90 | signal, orange/yellow | GPIO18 |
| SG90 | red power | Uno5V, Uno powered by laptop USB |
| SG90 | brown/black GND | UnoGND and ESP32GND common |

ESP32 separately powered by laptop USB. Do not join Uno5V to ESP32 3V3 or GPIO. Avoid additional VIN power while ESP32 uses USB without verifying board arrangement. Servo current capacity through Uno/laptop USB remains unverified under load; user measured correct idle voltage and standalone motion happened. Final intended supply is an adequately rated regulated5V servo branch with common ground. Never power servo from3V3. I2C pull-ups must remain compatible with ESP32 3.3V logic.

IMU labels include VCC,GND,SCL,SDA,XDA,XCL,AD0,INT. Extra wires appear in photos; their destinations are NOT fully verified. Current firmware needs only power and SDA/SCL and polls without INT. Do not assume XDA/XCL are the normal SDA/SCL. Sensor currently responds at0x68. Check labels rather than wire colours.

## Hardware identities and discrepancies
### OLED
1.3-inch128x64 I2C, address0x3C observed. User initially said SSD1306. SSD1306 mode produced partial text and random pixels. Explicit SH1106 test produced fully legible text/border in user photo. Current firmware uses Adafruit_SH1106G. Keep this known-working choice unless evidence changes. ACK/initialization alone never proves visual output.

### Distance sensor
Small blue4-pin board, marking interpreted by user as UL53LDK/VL53LDK. At address0x29 checked registers C0=EE,C1=AA,C2=10 matched VL53L0X documented signature. Selected Adafruit_VL53L0X. Some photos show its BACK facing upward, optical window facing breadboard: this caused approximately30mm readings that did not track the intended target. Black optical window must face target, unobstructed.

### IMU
Board silkscreen says MPU6050 HW-123E. Repeated checked WHO_AM_I register0x75 reads at address0x68 return0x70; no ACK at0x69. MPU6050 expects identity0x68; MPU6500 documents0x70. Address0x68 is not the identity value! Adafruit MPU6050 initialization failed accordingly.
Current code uses a small direct Wire MPU6500-compatible register adapter, not an installed MPU6500 library and not a patched Adafruit identity check. Adafruit MPU6050 remains installed for older saved diagnostics. Actual chip authenticity/model cannot be guaranteed from this register alone; call it MPU6500-compatible. User questioned the change, so explain it with readback evidence and do not silently force6050.
Sources previously consulted:
- https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6500-Register-Map2.pdf
- https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
- https://www.st.com/resource/en/datasheet/vl53l0x.pdf
- https://www.st.com/resource/en/user_manual/um2039-world-smallest-timeofflight-ranging-and-gesture-detection-sensor-application-programming-interface-stmicroelectronics.pdf

## Installed versions: verified during handoff
All are pinned in platformio.ini and present according to pio pkg list:
- espressif32 platform7.1.3
- Arduino framework package4.20017.260907+sha.dcc1105b (Arduino2.x family)
- ESP32Servo0.13.0, chosen for this core; do not blindly update
- Adafruit SSD1306 2.5.13
- Adafruit SH110X 2.1.12
- Adafruit GFX Library1.12.6
- Adafruit BusIO1.17.4
- Adafruit_VL53L0X1.2.4
- Adafruit MPU6050 2.2.9
- Adafruit Unified Sensor1.1.15
Wire is supplied by Arduino. Current main uses Wire, SH110X and ESP32Servo; some installed libraries are only used by backups. Build during handoff: SUCCESS, RAM22420/327680bytes, flash324889/1310720bytes. Earlier upload of current firmware toCOM5 succeeded with flash hash verification.

## What actually worked
| Item | Evidence | Limits |
|---|---|---|
| ESP32 flashing/Serial | repeated successful upload and live logs | no endurance/power qualification |
| Shared I2C | ACK0x29,0x3C,0x68 | ACK alone isn't full functional proof |
| OLED | user photo legible in SH1106 mode | no long-run test |
| ToF | valid status0 readings;49 readings110..118mm in one capture | not calibrated accuracy |
| Distance to400mm | user says ruler and display matched | user-observed; no controlled range/material study |
| IMU calibration | several successful150-sample calibrations | requires stationary suitable pose |
| IMU tilt | user said pitch/roll changed and returned nearzero | physical axes/absolute accuracy unverified |
| Standalone SG90 | user: 'it move both direction a little bit' after repeated sweep | small pulse range only; load capability unverified |
| MPU-driven servo | commands enabled/logged | user repeatedly reports NO physical motion; unresolved |

Earlier ToF invalid results above about250mm had RangeStatus2 (signal failure), not a software250mm cutoff. Later400mm success means250mm was not established as a hard limit. Initial30mm measurements were likely nearby obstruction/orientation. Never convert invalid/stale to clear.

Calibration frequently failed REST BOARD FLAT or CALIBRATION MOTION while sensor was held, tilted, upside down or against a vertical panel. Once laid still chip-up on cardboard it succeeded. Latest successful saved calibration: reference pitch-1.35deg,roll2.09deg; gyro bias rad/s[-0.03026,0.07439,-0.00964]. Following short output around0.00deg. Those offsets reset on reboot; they are not permanent precision calibration.

## Current firmware: src/main.cpp
A one-axis IMU/OLED/servo diagnostic only. NO distance acquisition, scanning or obstacle warnings in this active version. Do not describe the final application as finished.

Boot: GPIO18/19LOW, I2C21/22 at100kHz,25ms Wire timeout, SH1106 init, probeIMU, require identity0x70, checked register writes/readback, auto calibration; servo remains detached until COMP ON.
Register/value pairs:6B=01,6C=00,1A=03,1B=00,1C=00,1D=03,19=09.
Checked14-byte read from0x3B. Accel scale16384LSB/g, gyro131LSB/(deg/s), gyro stored in rad/s then converted for integration.
Nominal20ms sampling; OLED writes at100kHz cause gaps, observed average closer40Hz. Reject dt>250ms. Complementary filter alpha=0.5/(0.5+dt); boardXgyro roll, boardYgyro pitch. Accelerometer correction only0.85..1.15g. Small-angle approximation, not robust full3D attitude estimation.
Calibration:1s settle,150samples,3attempts,15s deadline; reject accel outside0.85..1.15g,gyro norm>0.15rad/s, initial abs pitch/roll>60deg, or more than2deg deviation from firstsample. Failures disable servo. Raw diagnostic may still be available when calibration fails if imuReady remains true.

Serial commands, exact uppercase with newline:
- STATUS
- CALIBRATE (stops compensation)
- IMU CHECK
- RAW
- COMP ON
- COMP OFF or SERVO OFF
- AXIS PITCH / AXIS ROLL (both stop compensation)
- REVERSE (stops compensation and flips sign)
SERVO TEST is NOT a command in this current version; it belongs to backup M5.

Servo:50Hz GPIO18. Neutral1500us, range1450..1550us. Requested target=clamp(1500 + sign * relativeTiltDeg *10,1450,1550). Defaultsign=-1. Deadband0.5deg. Slew2us per eligible20ms update. Saturates at roughly5deg input; NOT calibrated degrees of servo travel.
COMP ON requiresREADY,valid,fresh IMU and acceleration gate. Automatically stops at60s,invalid/stale data,accel outsidegate,nonfinite tilt or selected tilt>30deg. startCompensation doesn't check30deg until subsequent update; this is a candidate small improvement. Stop detaches signal and drivesGPIOLOW, not power disconnect or guaranteed centering. pulseUs log retains lastcommand afterOFF. No physical position feedback.
Default useRoll=true in source. AXIS PITCH selected during latest session, but settings/sign/reference aren't persisted.
OLED: BeyLit TILT TEST, relative pitch/roll, COMP ON/OFF axis/pulse, advisory-only labels. 500ms display/status update.

## Why the no-movement issue is still open
Standalone sweep1500->1450->1550->1500 physically worked. In live tilt tests, commands were sometimes only1500->1507us, potentially too small to see. An earlier roll test logged1464 then1450us but the user did not confirm movement. In one follow-up COMP was alreadyOFF and input was mostlypitch (~36deg) while selectedaxisROLL (~6deg). Switched toPITCH, enablednearlevel; user still reported no movement. At handoff pitch~45deg, above30deg cutoff. There is no synchronized complete trace proving exactly what occurred while the user tilted.
Possible causes to investigate, NOT established diagnoses: timeout before user action, motion/acceleration cutoff, wrong physicalaxis, tiny/deadband pulse change, lost wiring/power/commonground, servo load, attach/PWM issue. Library installation was confirmed; successful build alone doesn't verify PWM or mechanics.

## Suggested next work for Claude
1. Read this handoff, current main and config. Explain the6050/6500 mismatch without changing driver blindly.
2. Keep servoOFF initially. Check actual wiring and power underload if possible.
3. Re-run bounded standalone sweep on the SAME current wiring to separate hardware from IMU logic, preserving current source first. Use M5 backup or add explicit test safely. No0..180 sweep.
4. Calibrate still; choosePITCH explicitly after each reboot. Give 'hold still' then 'tilt now' as separate instructions.
5. Capture a continuous timestamped trace through enable, smalltilt,return,oppositetilt andstop; include selectedangle,target,currentpulse,attached status and specificstop reason. Coordinate while user watches. Stop and timeout conditions currently share a vague message; distinguish them.
6. If commands change widely but horn doesn't, diagnose power/PWM/connection rather than changing sensor library. If commands barelychange, diagnoseaxis/deadband/timing. Do not remove safeguards just to force motion.
7. Once movement confirmed, fix mechanical mount and verify compensation sign/gain. Current picturedservo shaft isvertical (pan), while leveling needs axis aligned to intendedpitch. Keep IMU onfixedbase, notmovinghorn. A flatmountedToF may point down; proposed Lcardboard mount is not yet validated. Keep windowclear andwires slack.
8. Decide one-servo demonstration scope: pan scan OR pitchcompensation. Add secondservo later for combined behavior after supply/mechanics checked.
9. Implement sectors/settling/warnings incrementally; retain basic pan+ToF+OLED fallback if stabilization blocks progress.

## Remaining original acceptance requirements
Not yet implemented/tested as complete system:
- centre-left-centre-right cooperative state machine; modest configurableangles; waitforsettle thenacquire withboundedtimeout
- timestamped per-sector mm,status,validity,commandedangle; freshness compatible with revisitperiod
- reject invalid before median/filter; immediate freshclosewarning, clearinghysteresis/persistence; preserveknownwarning whenanothersectorunknown
- never PATH SAFE; use NO NEAR OBSTACLE onlyallrequiredsectorsfreshvalid outsidewarningrange
- originaldemo STOP600mm/CAUTION1200mm are NOT validated or suitable to assert with currentrange evidence; adjust onlywith explicitbenchvalidation, never claimbrakingdistance
- disconnect/fault/stale/scanoff tests, timestamp rollover unit tests, thresholds/hysteresis/mixedunknown tests
- axislabels verifiedphysically, current/powertransients andresets checked, fullscanlatency measured,15min endurance andpowercycle tests
- finalwiring/powerdiagram,knownlimitations,testchecklist,2minute demoscript, staticprojectpage afterfirmwareworks
Original requirements included separately verbatim; do not interpret its 'first response' instructions as a need to restart completedmilestones.

## Files and recovery map
- platformio.ini: verified dependencies
- src/main.cpp: CURRENT compensationdiagnostic, builds/uploads
- src/m5_distance_servo_backup.cpp.txt: ToF+SH1106+manualSERVO TEST, known standalone movement. Read before restoring; .txt intentionally notcompiled.
- src/m3_distance_test.cpp.txt: stationaryToF; servo testdisabled
- src/m4_mpu6500_test.cpp.txt: IMU-only diagnostic beforecompensation
- src/m4_mpu6050_diagnostic.cpp.txt: older6050 initfailure version
- src/i2c_scanner.cpp.txt: firstscanner
- src/pre_distance_test.cpp.txt,ssd1306_bench_backup.cpp.txt: olderdisplay/servoversions
- diagnostics/uno-servo: abandoned UnoD9 experiment, built butnotuploaded
- README.md: historicalappend-onlynotes; earlyclaims superseded
- logs/: actualcaptures, some failedtests. Reviewdates/content, don'tassume all arepasses.
Usefullogs: m4_chip_up_calibration.txt; m3_forward_target.txt; m3_300mm_recheck.txt; m5_mounted_servo_test.txt; m5_servo_repeat.txt; tilt_calibration_*.txt; mpu_compensation_enabled.txt.

## Serial access example
Python pyserial is already installed in PlatformIO's Python. Use a short boundedreader; no concurrent monitor. SettingDTR/RTSfalse beforeopen avoids an intentionalreset but stillverifyboot/state.
```python
import serial, time
p=serial.Serial()
p.port='COM5'; p.baudrate=115200; p.timeout=0.2
p.dtr=False; p.rts=False
try:
    p.open(); p.reset_input_buffer()
    p.write(b'STATUS\n'); p.flush()
    end=time.monotonic()+2
    while time.monotonic()<end:
        print(p.read(4096).decode('utf-8',errors='replace'),end='')
finally:
    if p.is_open: p.close()
```
Never claim a command was sent unless it actuallywas. Without computeraccess, provideuser instructions andrequest output. Explainonephysicalstep atatime; do not repeatedlyrequest arbitrary boardrepositioning ifconfirmedflatdata remainswrong.
