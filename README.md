# The Autonomous Robo Cop

**An ESP32-Based Autonomous Security & Fire-Safety Robot with a Wi-Fi Remote-Controlled Companion Car**
---
![Autonomous Robo Cop](Screenshot%2026-08-17%145628.png)


## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Motivation & Objectives](#2-motivation--objectives)
3. [System Architecture](#3-system-architecture)
4. [Bill of Materials](#4-bill-of-materials)
5. [Repository Contents](#5-repository-contents)
6. [Firmware Walkthrough](#6-firmware-walkthrough)
   - [6.1 `robocop.ino`](#61-robocopino)
   - [6.2 `RemoteControlCar.ino`](#62-remotecontrolcarino)
   - [6.3 Differences Between the Two Sketches](#63-differences-between-the-two-sketches)
   - [6.4 Sensor Polarity — a Documentation Discrepancy Worth Knowing](#64-sensor-polarity--a-documentation-discrepancy-worth-knowing)
   - [6.5 How to Build & Flash](#65-how-to-build--flash)
7. [Subsystem 1 — Robo Cop (Sensing & Response Unit)](#7-subsystem-1--robo-cop-sensing--response-unit)
   - [7.1 Components & Roles](#71-components--roles)
   - [7.2 Wiring / Pin Connections](#72-wiring--pin-connections)
   - [7.3 Circuit Schematic](#73-circuit-schematic)
   - [7.4 Working Logic — Motion Detection](#74-working-logic--motion-detection)
   - [7.5 Working Logic — Fire Detection & Extinguishing](#75-working-logic--fire-detection--extinguishing)
8. [Subsystem 2 — Remote Control Car (Mobility & Vision Unit)](#8-subsystem-2--remote-control-car-mobility--vision-unit)
   - [8.1 Components & Roles](#81-components--roles)
   - [8.2 Wiring / Pin Connections](#82-wiring--pin-connections)
   - [8.3 Circuit Schematic](#83-circuit-schematic)
   - [8.4 ESP32-CAM Programming](#84-esp32-cam-programming)
   - [8.5 Remote-Control Interface & Operation](#85-remote-control-interface--operation)
9. [Physical Build](#9-physical-build)
10. [End-to-End Working Summary](#10-end-to-end-working-summary)
11. [Design Considerations & Safety](#11-design-considerations--safety)
12. [Issues Faced & How They Were Handled](#12-issues-faced--how-they-were-handled)
13. [Note on Project Documentation History](#13-note-on-project-documentation-history)
14. [Conclusion](#14-conclusion)
15. [Future Work](#15-future-work)

---

## 1. Project Overview

**The Autonomous Robo Cop** is an embedded systems and robotics project built around the **ESP32** and **ESP32-CAM** microcontrollers. It combines two cooperating physical subsystems mounted on a single four-wheeled chassis:

1. **The Robo Cop unit** — an autonomous sensing-and-response system that detects human intruders (via a PIR motion sensor) and detects fire (via IR flame sensors and an MQ2 smoke sensor), then automatically responds: sounding a buzzer alarm and triggering a servo-actuated water pump to extinguish the fire. **This is the subsystem whose firmware is present in this repository** (`robocop.ino` and `RemoteControlCar.ino` — see [Section 6](#6-firmware-walkthrough) for why both files implement this unit).
2. **The Remote Control Car unit** — a Wi‑Fi-controlled mobility platform built around an ESP32‑CAM, driven by four DC motors through an L298N motor driver, steerable from a web-based remote-control interface. This subsystem is documented here from the team's project report/paper, but **its firmware is not included in this repository** (see [Section 8](#8-subsystem-2--remote-control-car-mobility--vision-unit)).

Together, the two subsystems turn the platform into a low-cost, roaming **autonomous security guard and first-response fire extinguisher**, intended for use cases such as industrial warehouse security patrol and early-stage fire suppression.

The project explicitly targets **UN Sustainable Development Goal 9** (Industry, Innovation and Infrastructure) and **Goal 11** (Sustainable Cities and Communities), framing the robot as a low-cost automation solution that reduces reliance on human labor for repetitive, hazardous monitoring tasks.

---

## 2. Motivation & Objectives

**Motivation:** *"Make daily lives simpler."* The team's stated goal is to use embedded systems and IoT to reduce human intervention in industrial settings — replacing tasks that are simple enough to automate but currently rely heavily on human labor (security patrol, fire watch) — thereby cutting labor costs and improving response speed and safety.

**Objectives:**
- Design and implement an autonomous Robo Cop car using an **ESP32** microcontroller.
- Enable three core capabilities: **intruder detection**, **fire detection**, and **autonomous fire extinguishing**.
- Package the system as a mobile platform that can serve as a **security guard for industrial warehouses** and an **emergency fire extinguisher**.
- Provide **remote drivability** via Wi‑Fi so the platform can be manually navigated to a location of interest (using a second ESP32‑CAM-based control system).

---

## 3. System Architecture

The robot is built as **two logically separate but physically co-mounted subsystems**, each with its own microcontroller, sharing the same chassis, wheels, and power arrangement:

```
                         ┌─────────────────────────────────────────┐
                         │              4-Wheeled Chassis            │
                         └─────────────────────────────────────────┘
                                   │                        │
        ┌──────────────────────────┘                        └──────────────────────────┐
        ▼                                                                                ▼
┌───────────────────────────────┐                                    ┌───────────────────────────────┐
│   ROBO COP UNIT (ESP32)        │                                    │  REMOTE CONTROL CAR (ESP32-CAM) │
│                                 │                                    │                                 │
│ PIR Sensor ──► Intruder alert  │                                    │  Wi-Fi Web Interface            │
│ 3× IR Sensors ──► Fire detect  │                                    │        │                        │
│ MQ2 Smoke Sensor ──► Smoke     │                                    │        ▼                        │
│ (HW-038 Water Sensor, robocop  │                                    │  ESP32-CAM (control logic)      │
│  .ino only) ──► Tank level     │                                    │        │                        │
│        │                       │                                    │        ▼                        │
│        ▼                       │                                    │  L298N Motor Driver             │
│ Passive Buzzer (alarm)         │                                    │        │                        │
│ 5V Relay ──► Servo + Pump      │                                    │        ▼                        │
│        │                       │                                    │  4× DC Motors + Wheels          │
│        ▼                       │                                    │  = Car Movement (F/B/L/R)       │
│ Servo Motor + Water Pump/Tank  │                                    │                                 │
│  = Fire Extinguishing Action   │                                    │  (firmware not in this repo)    │
└───────────────────────────────┘                                    └───────────────────────────────┘
        ▲
        │
   Code in this repo:
   robocop.ino / RemoteControlCar.ino
   (both implement THIS unit — see §6)
```

The Robo Cop unit runs **autonomously and continuously**, independent of user input, monitoring its environment and reacting to detected threats. The Remote Control Car unit is **manually driven** by an operator through a web-based control page served over Wi‑Fi, allowing the whole platform to be positioned/patrolled as needed.

---

## 4. Bill of Materials

### Robo Cop unit
| Qty | Component |
|---|---|
| 1 | ESP32 (main microcontroller) |
| 1 | PIR Motion Sensor |
| 3 | IR Flame Sensors |
| 1 | Passive Buzzer |
| 1 | 5V Relay Module |
| 1 | Servo Motor |
| 1 | Water Pump + Water Tank |
| 1 | MQ2 Smoke Sensor |
| 1 | HW‑038 Analog Water-Level Sensor *(used in `robocop.ino` only — see §6.3)* |
| — | Connecting wires |

### Remote Control Car unit
| Qty | Component |
|---|---|
| 1 | ESP32‑CAM (Wi‑Fi-enabled microcontroller with camera) |
| 1 | Car chassis |
| 4 | DC motors + wheels |
| 1 | L298N dual/quad H‑bridge motor driver |
| 1 | 12–14V battery pack |
| — | Connecting wires |

---

## 5. Repository Contents

This is a small, code-plus-photos repository — it holds the two Arduino sketches for the Robo Cop unit and three reference images. It does **not** contain the team's original PDF/DOCX report files or any ESP32‑CAM / motor-driver firmware; those are referenced below purely as documentation sources.

| File | Description |
|---|---|
| [`robocop.ino`](robocop.ino) | Primary/most complete firmware for the Robo Cop unit — flame + smoke + water-level sensing, buzzer, relay-driven pump, servo actuation. See [§6.1](#61-robocopino). |
| [`RemoteControlCar.ino`](RemoteControlCar.ino) | Despite its filename, this is **also** Robo Cop fire/smoke-response firmware (an earlier/simpler variant without water-level sensing) — **not** the ESP32‑CAM motor-driving code described in [§8](#8-subsystem-2--remote-control-car-mobility--vision-unit). See [§6.2](#62-remotecontrolcarino). |
| `Screenshot 2026-08-17 145628.png` | Build photo — assembled robot on its 4-wheel chassis with water tank, pump, servo, and wiring visible. |
| `Screenshot 2026-08-17 145645.png` | Schematic — Robo Cop wiring (ESP32 + 3× IR sensors + MQ2 + PIR + relay + servo + buzzer + pump). |
| `Screenshot 2026-08-17 145739.png` | Schematic — Remote Control Car wiring (L298N motor driver + 2 DC motor pairs + ESP32‑CAM + 12V DC supply). |
| `README.md` | This file. |

> The team's original write-ups (`MMBD_Project_Report.docx`, an early draft, and `MMBD_Project.pdf`, the final IEEE-style paper) are referenced throughout this README as the source for hardware/architecture details that go beyond what the code alone shows (PIR wiring, the full Remote Control Car subsystem, BOM, issues faced, etc.). **Neither file is present in this repository** — if you have them, drop them in the repo root and the links above will resolve.

---

## 6. Firmware Walkthrough

Both `.ino` files in this repo target the **Robo Cop (fire/smoke/intruder-response) unit**, built with the [`ESP32Servo`](https://github.com/madhephaestus/ESP32Servo) library. There is currently no committed firmware for the ESP32‑CAM Remote Control Car described in the project report.

### 6.1 `robocop.ino`

The more complete and evidently later of the two sketches. Adds an analog **water-level sensor** so the pump won't run the tank dry.

**Pin map (as defined in code):**

| `#define` | GPIO | Mode | Purpose |
|---|---|---|---|
| `SERVO_PIN` | 5 | Servo (PWM) | Servo motor signal |
| `FLAME_SENSOR_PIN1` | 35 | `INPUT` | Flame sensor #1 digital output |
| `FLAME_SENSOR_PIN2` | 32 | `INPUT` | Flame sensor #2 digital output |
| `FLAME_SENSOR_PIN3` | 33 | `INPUT` | Flame sensor #3 digital output |
| `RELAY_PIN` | 26 | `OUTPUT` | Drives the relay that switches the pump |
| `SMOKE_SENSOR_PIN` | 18 | `INPUT` | MQ2 smoke sensor digital output |
| `BUZZER_PIN` | 19 | `OUTPUT` | Passive buzzer |
| `WATER_SENSOR_PIN` | 34 | `analogRead` | HW‑038 water-level sensor (analog) |

**Tunable constants:**
- `MAX_SENSOR_VALUE = 1880` — the raw ADC reading observed when the water sensor is fully submerged; used as the 100% reference for the water-level percentage calculation.
- `LOW_WATER_LEVEL_THRESHOLD = 500` — below this raw ADC value, the tank is considered too low to safely run the pump.

**`setup()`:**
1. Starts `Serial` at 115200 baud (for debug logging).
2. Configures the three flame-sensor pins, the smoke-sensor pin, and the water-sensor pin as `INPUT`; configures the relay and buzzer pins as `OUTPUT`.
3. Attaches the servo on `SERVO_PIN` with a 500–2400 µs pulse range and immediately parks it at 0°.
4. Drives `RELAY_PIN` **HIGH** (pump off — this relay module is active-LOW, see [§6.4](#64-sensor-polarity--a-documentation-discrepancy-worth-knowing)) and `BUZZER_PIN` **LOW** (buzzer off) so the system starts in a known-safe idle state.

**`loop()`** (runs every ~500 ms via a trailing `delay(500)`):
1. Reads all three flame sensors, the smoke sensor, and the water-level sensor; logs every raw value to Serial.
2. Computes `waterLevelPercentage = sensorValue / MAX_SENSOR_VALUE * 100` and logs it.
3. **Water-level gate:** if `sensorValue < LOW_WATER_LEVEL_THRESHOLD`, the tank is treated as empty — `deactivateActuators()` is called unconditionally (servo parked, relay/pump off), regardless of whether flame is present. This takes priority over fire response so the pump is never driven dry.
4. **Otherwise** (enough water): if *any* flame sensor reads `LOW` (i.e. is triggered — see §6.4), `activateActuators()` sweeps the servo and turns the pump on; if none are triggered, `deactivateActuators()` keeps things off.
5. **Smoke check runs independently of the water/flame branch above:** if the smoke sensor reads `LOW` (triggered), the buzzer is turned on; otherwise it's turned off. This means the buzzer can sound for smoke even while the pump is disabled for low water.

**Helper functions:**
- `activateActuators()` — sweeps the servo 0°→180° in steps of 10 (`rotateServo(0, 180, 10)`), then back 180°→0° (`rotateServo(180, 0, -10)`), then sets `RELAY_PIN` LOW to energize the pump. The sweep is what physically dispenses water via the pump's hand-actuated mechanism (see [§9](#9-physical-build)).
- `deactivateActuators()` — resets the servo to 0° and sets `RELAY_PIN` HIGH (pump off).
- `rotateServo(start, end, step)` — steps the servo from `start` to `end` in increments of `step` (positive or negative), writing each intermediate angle with a 25 ms delay between steps, producing a smooth sweep rather than an instant jump. Also logs each position to Serial.

### 6.2 `RemoteControlCar.ino`

Functionally a **subset/earlier draft** of `robocop.ino`, minus water-level sensing, and with the flame/smoke branches combined slightly differently. The filename is misleading — there is no Wi‑Fi, HTTP, or motor-driver code anywhere in this file; it is a second fire-response sketch, most likely an earlier iteration that predates the water-sensor feature and was never renamed.

**Pin map:** identical to `robocop.ino` for `SERVO_PIN` (5), `FLAME_SENSOR_PIN1/2/3` (35/32/33), `RELAY_PIN` (26), `SMOKE_SENSOR_PIN` (18), and `BUZZER_PIN` (19). No water-sensor pin is defined.

**Unused constants:** `pwmFreq = 50` and `pwmResolution = 8` are declared but never referenced anywhere in the file — leftover scaffolding, likely from an earlier attempt at manual `ledc`-based PWM before switching to the `ESP32Servo` library's built-in pulse generation.

**`setup()`:** same pin initialization pattern as `robocop.ino`, except the relay is initialized **LOW** (pump *energized*/on) rather than HIGH — an inconsistency with `robocop.ino`'s safer HIGH-at-boot default (see [§6.3](#63-differences-between-the-two-sketches)).

**`loop()`** (no trailing delay — runs as fast as the loop body allows, unlike `robocop.ino`'s throttled 500 ms cycle):
1. Reads the three flame sensors and the smoke sensor; logs raw values.
2. Four-way `if / else if` chain, evaluated in this order:
   - **Flame AND smoke both triggered** (`flameX == LOW` for any sensor, **and** `smokeSensorValue == LOW`) → logs a combined message and runs the same sweep-then-pump-on sequence as below (buzzer is *not* explicitly turned on in this branch — see the bug note below).
   - **Flame only** → 100 ms delay, sweep servo 0→180→0, 100 ms delay, pump on.
   - **Smoke only** → buzzer on (pump/servo untouched).
   - **Neither** → servo reset to 0°, pump off, buzzer off.
3. Because the buzzer is never explicitly turned on in the "flame AND smoke" branch, a fire that also triggers smoke will run the pump but **not** sound the buzzer unless a later loop iteration happens to land in the smoke-only branch — a latent bug relative to the clearer, independent smoke check in `robocop.ino`.

### 6.3 Differences Between the Two Sketches

| Aspect | `robocop.ino` | `RemoteControlCar.ino` |
|---|---|---|
| Water-level sensing | Yes (HW‑038 on GPIO34) | No |
| Pump inhibited on low water | Yes | N/A (no sensor) |
| Relay idle state at boot | HIGH (pump off — safe default) | LOW (pump on — unsafe default) |
| Smoke → buzzer logic | Independent `if/else`, always evaluated every loop | Nested inside the flame/smoke `if/else if` chain; buzzer not set when both flame and smoke trigger together |
| Loop pacing | `delay(500)` at end of `loop()` | No delay — loop runs unthrottled |
| Unused leftover constants | None | `pwmFreq`, `pwmResolution` (dead code) |
| Overall maturity | More defensive/complete | Earlier/simpler draft |

**Recommendation:** treat `robocop.ino` as the authoritative firmware to flash. `RemoteControlCar.ino` is best kept for reference/history unless you specifically need the simpler (no water-sensor) behavior.

### 6.4 Sensor Polarity — a Documentation Discrepancy Worth Knowing

The project report's wiring notes (reproduced in [§7.2](#72-wiring--pin-connections)) describe the flame and smoke sensor outputs as idling **LOW** and going **HIGH** when triggered. **The actual firmware in both sketches does the opposite** — it treats a `LOW` reading as "triggered":

```cpp
if ((flameSensorValue1 == LOW || flameSensorValue2 == LOW || flameSensorValue3 == LOW)) { ... }
if (smokeSensorValue == LOW) { ... }
```

This is consistent with common flame/gas sensor breakout boards (including many MQ2 and IR-flame modules), which use an onboard comparator that pulls the digital `OUT` pin **LOW** when the sensed condition exceeds the threshold, and idles HIGH otherwise — the opposite convention from the PIR sensor, whose `OUT` pin genuinely idles LOW and goes HIGH on motion (matching the code's `if (motion detected)` expectations described in the report, though the PIR read itself does not appear in either committed sketch).

**Practical takeaway:** if you're wiring this project from scratch, verify your specific sensor modules' datasheets for active-HIGH vs. active-LOW output before assuming either this README's prose or the code comments are authoritative — trust the `== LOW` / `== HIGH` comparisons in the actual sketch you flash.

### 6.5 How to Build & Flash

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x).
2. Add ESP32 board support: **File → Preferences → Additional Boards Manager URLs** → add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`, then install **esp32** via **Tools → Board → Boards Manager**.
3. Install the **ESP32Servo** library via **Sketch → Include Library → Manage Libraries…**, search for "ESP32Servo" by Kevin Harrington / John K. Bennett.
4. Open `robocop.ino` (recommended over `RemoteControlCar.ino` — see [§6.3](#63-differences-between-the-two-sketches)).
5. Select **Tools → Board → ESP32 Dev Module** (or your specific ESP32 board variant) and the correct **Port**.
6. Wire the hardware per [§7.2](#72-wiring--pin-connections), double-checking your sensors' active-HIGH/active-LOW polarity against [§6.4](#64-sensor-polarity--a-documentation-discrepancy-worth-knowing).
7. Click **Upload**. Open **Tools → Serial Monitor** at **115200 baud** to observe live sensor readings, water-level percentage, and state transitions.
8. Calibrate `MAX_SENSOR_VALUE` and `LOW_WATER_LEVEL_THRESHOLD` in `robocop.ino` against your own HW‑038 sensor and tank geometry — the shipped values were tuned for the team's specific tank/sensor pairing and will not generalize exactly.

---

## 7. Subsystem 1 — Robo Cop (Sensing & Response Unit)

### 7.1 Components & Roles

| Component | Function | How it works |
|---|---|---|
| **ESP32** | Main microcontroller | Runs all sensing and actuation logic for the Robo Cop unit |
| **PIR Sensor** | Intruder detection | Pyroelectric sensor that generates an electrical charge in response to changes in ambient infrared radiation (heat), typically emitted by moving humans/animals; flags movement in its field of view |
| **IR (flame) Sensors ×3** | Fire detection | Each comprises an IR LED emitter and a photodiode receiver; can also sense the infrared signature radiated directly by flames. Three units are used, oriented to cover more directions/angles around the robot |
| **MQ2 Smoke Sensor** | Gas/smoke detection | Uses a tin-dioxide (SnO₂)-based sensitive material whose resistance changes on exposure to smoke, LPG, methane, or propane; the resistance change alters output voltage in proportion to gas concentration |
| **Passive Buzzer** | Audible fire/smoke alert | Requires an external driving signal/waveform (unlike an active buzzer) — the ESP32 supplies the tone signal, and the buzzer vibrates at the input frequency to produce sound |
| **5V Relay Module** | High-power switch | Uses a small control current to energize an internal electromagnet that mechanically opens/closes contacts, letting the low-power ESP32 GPIO safely switch the higher-current servo/pump circuit |
| **Servo Motor** | Pump actuation | PWM-controlled motor providing precise angular position control; used to physically actuate/rotate the water pump mechanism |
| **Water Pump + Tank** | Fire extinguishing | The tank stores water; the pump uses motor-driven pressure differential to move water from the tank to the fire location |
| **HW‑038 Water-Level Sensor** *(code-only, `robocop.ino`)* | Tank level monitoring | Analog resistive sensor whose reading scales with water depth; used in firmware to inhibit the pump when the tank runs low |

### 7.2 Wiring / Pin Connections

| Component | Pin | ESP32 Connection |
|---|---|---|
| MQ2 Smoke Sensor | VCC | 3.3V |
| MQ2 Smoke Sensor | OUT | GPIO18 |
| MQ2 Smoke Sensor | GND | GND |
| IR Sensor #1 | OUT | GPIO35 |
| IR Sensor #2 | OUT | GPIO32 |
| IR Sensor #3 | OUT | GPIO33 |
| All IR Sensors | VCC / GND | 3.3V / GND |
| Passive Buzzer | + | GPIO19 |
| Passive Buzzer | − | GND |
| Relay Module | VCC | 3.3V |
| Relay Module | GND | GND |
| Relay Module | IN | GPIO26 |
| Relay Module | Common Contact | Pump VCC |
| Relay Module | Normally Open | Positive terminal of battery |
| Servo Motor | RED (V+) | 3.3V |
| Servo Motor | BROWN (GND) | GND |
| Servo Motor | ORANGE (signal) | GPIO5 |
| Water Pump | + | Relay Common Contact pin |
| Water Pump | − | GND |
| HW‑038 Water Sensor *(code-only)* | Signal (analog) | GPIO34 |

> **Polarity note:** the report describes all sensor OUT pins as idling LOW and going HIGH when triggered. The committed firmware treats a **LOW** reading on the flame/smoke sensors as "triggered" instead — see [§6.4](#64-sensor-polarity--a-documentation-discrepancy-worth-knowing) for the full explanation before wiring/trusting either source blindly.

### 7.3 Circuit Schematic

The wiring diagram below shows all Robo Cop components connected to the ESP32 — the three IR flame sensors and the MQ2 smoke sensor on the left feeding into the ESP32, with the relay module (driving the water pump), the servo motor, and the passive buzzer on the output side:

![Robo Cop wiring schematic](Screenshot%202026-08-17%20145645.png)

*Signal path: sensors (IR ×3, MQ2, PIR) → ESP32 GPIOs → decision logic → Relay + Servo (pump activation) and Buzzer (audible alert).*

### 7.4 Working Logic — Motion Detection

1. The PIR sensor's OUT pin is **LOW** by default (no motion).
2. When the PIR sensor detects motion (a change in sensed infrared/heat levels), it sets OUT to **HIGH**.
3. This HIGH signal is read by the ESP32 on **GPIO18** (per the report's pin map).
4. The ESP32 interprets this as a possible intruder and issues an **intruder alert**.

> **Note:** GPIO18 is also documented as the MQ2 smoke sensor's OUT pin in [§7.2](#72-wiring--pin-connections), and neither committed sketch reads a PIR sensor at all. The PIR wiring/logic above is preserved from the team's original report for completeness, but could not be cross-checked against the code in this repository — treat it as design intent rather than verified, shipped behavior.

### 7.5 Working Logic — Fire Detection & Extinguishing

1. Each IR flame sensor's OUT pin idles at a rest state (see the polarity note in §6.4/§7.2) and is read on **GPIO35**, **GPIO32**, or **GPIO33**.
2. When fire is present, its infrared radiation is picked up by one or more of the IR sensors, flipping the corresponding OUT pin.
3. The specific GPIO that changes state tells the ESP32 which of the three sensors — and therefore roughly which direction — detected the fire.
4. In parallel, if the **MQ2 smoke sensor** detects smoke, the ESP32 activates the **passive buzzer** to sound an audible fire alert.
5. In `robocop.ino`, before acting on a flame reading the firmware first checks the **water-level sensor**; if the tank is below `LOW_WATER_LEVEL_THRESHOLD`, actuators are held off regardless of flame state.
6. Otherwise, the ESP32 activates the **relay module**, closing the circuit for the **servo motor** and the **water pump**.
7. The **servo motor sweeps 0°→180°→0°**, actuating the water pump's hand-pump mechanism, driving water from the tank onto the fire — completing the autonomous fire-extinguishing action.

---

![Subsystem 1](Screenshot%2026-08-17%145645.png)


## 8. Subsystem 2 — Remote Control Car (Mobility & Vision Unit)

> **Reminder:** everything in this section describes the mobility subsystem **as documented in the team's project report**. No corresponding firmware (Wi‑Fi, HTTP server, or L298N motor-driver code) exists in this repository at this time — see [§5](#5-repository-contents) and [§6](#6-firmware-walkthrough).

### 8.1 Components & Roles

| Component | Function | How it works |
|---|---|---|
| **ESP32‑CAM** | Main microcontroller (Wi‑Fi + camera) | Hosts the remote-control web interface, receives movement commands over Wi‑Fi, and drives the motor control logic |
| **Car Chassis** | Structural platform | Lightweight plastic/aluminum/acrylic frame that mounts motors, battery, and electronics; pre-drilled for component mounting |
| **4× DC Motors + Wheels** | Propulsion | Convert electrical energy from the battery into rotational mechanical energy; wheels convert that rotation into linear motion of the car |
| **L298N Motor Driver** | Motor control interface | Converts the ESP32‑CAM's low-current digital control signals into the higher-current outputs needed to drive the DC motors; supports PWM-based speed control and includes a heat sink for thermal management |
| **Battery (12–14V)** | Power source | Supplies power to the ESP32‑CAM, the motors, and the motor driver — commonly Li‑ion/Li‑Po or AA packs, sized to the motor and ESP32‑CAM current draw |

### 8.2 Wiring / Pin Connections

**L298N Motor Driver → ESP32‑CAM**

| ESP32‑CAM GPIO | L298N Input | Function |
|---|---|---|
| GPIO 14 | IN1 | Left Motor Forward |
| GPIO 15 | IN2 | Left Motor Backward |
| GPIO 13 | IN3 | Right Motor Forward |
| GPIO 12 | IN4 | Right Motor Backward |
| ESP32‑CAM 5V | VCC | Motor driver logic supply |
| ESP32‑CAM GND | GND | Common ground |

**PIR Sensor (secondary, on this unit)**

| Pin | Connection |
|---|---|
| VCC | 5V on ESP32‑CAM |
| GND | GND on ESP32‑CAM |
| OUT | GPIO 2 on ESP32‑CAM |

**DC Motors (Left / Right)**

- Left motor's two terminals → L298N **OUT1** and **OUT2**.
- Right motor's two terminals → L298N **OUT3** and **OUT4**.

> **Note:** an earlier draft of the project report describes a 4-motor, 8-output L298N wiring scheme (`OUT1–OUT8`, `IN1–IN4` from generic GPIOs `D1–D4`). The **final published paper** (reflected in the schematic and pin table above) simplifies this to a differential 2-side drive — left-side motors ganged to OUT1/OUT2, right-side motors ganged to OUT3/OUT4 — controlled by four explicit GPIOs (14/15/13/12). This README documents the final, built configuration; see [Section 13](#13-note-on-project-documentation-history) for details on this discrepancy.

### 8.3 Circuit Schematic

The remote-control car's drivetrain schematic shows the L298N motor driver wired to two DC gear-motor pairs (front and rear, or left and right) and to the ESP32‑CAM, powered by a 12V DC source:

![Remote Control Car wiring schematic](Screenshot%202026-08-17%20145739.png)

### 8.4 ESP32‑CAM Programming

- The ESP32‑CAM is programmed using the **Arduino IDE** (or another suitable IDE/toolchain).
- Firmware logic maps incoming remote-control commands to motor driver GPIO states (forward/backward/left/right).
- A **Wi‑Fi connection** is established so the ESP32‑CAM can host or join a network for remote control.
- **This firmware is not present in the current repository** — the description above reflects the team's report, not a source file you can inspect here.

### 8.5 Remote-Control Interface & Operation

- A **web-based application** serves as the remote-control interface, with on-screen controls for **forward, backward, left, and right** movement.
- **Operation sequence:**
  1. **Power On** — connect the battery to power the car.
  2. **Connect to Wi‑Fi** — the ESP32‑CAM joins (or hosts) a Wi‑Fi network, enabling remote control.
  3. **Remote Control** — the operator sends movement commands via the web interface.
  4. **Command Processing** — the ESP32‑CAM parses the commands and drives the corresponding motor GPIOs.
  5. **Movement** — the L298N driver energizes the appropriate motors, and the car moves accordingly.

**Data/control flow:**

```
User (web UI) → Wi-Fi → ESP32-CAM (command parser) → GPIO signals
    → L298N Motor Driver (adjusts speed & direction) → 4× DC Motors → Wheels → Car moves
```

---
![Subsystem 1](Screenshot%2026-08-17%145739.png)


## 9. Physical Build

The completed robot (below) is a 4-wheeled chassis (yellow wheels) carrying:
- A clear plastic water-tank reservoir (red lid) with a hand-pump-style dispenser mechanism mounted on top, used for fire suppression — this is the mechanism the servo physically actuates each time `activateActuators()` runs (see §6.1).
- A servo motor and mounting bracket for actuating the pump.
- Exposed breadboard-style wiring connecting the PIR sensor, IR sensors, buzzer, relay, and battery pack across the chassis.
- A battery pack strapped to one side for the sensing/response electronics.

![Assembled Robo Cop robot](Screenshot%202026-08-17%20145628.png)

---

## 10. End-to-End Working Summary

| Trigger | Detection | ESP32 Action | Physical Result |
|---|---|---|---|
| Human/animal movement nearby | PIR sensor OUT → HIGH (per report; not present in committed code) | Flags intruder alert | Alert signaled (per system design) |
| Flame detected (any of 3 directions) | IR sensor OUT triggers (GPIO35 / 32 / 33) | Identify fire + activate buzzer + close relay (subject to water-level gate in `robocop.ino`) | Servo sweeps → pump actuates → water dispensed onto fire |
| Smoke/gas detected | MQ2 sensor output crosses threshold | Activate passive buzzer | Audible smoke/fire alert |
| Water tank runs low *(`robocop.ino` only)* | Analog reading on GPIO34 drops below `LOW_WATER_LEVEL_THRESHOLD` | Deactivate servo/pump regardless of flame state | Pump inhibited to avoid dry-running |
| Operator command via web UI | Wi‑Fi packet received by ESP32‑CAM | Set L298N IN1–IN4 GPIO states | Car moves forward / backward / left / right |

---

## 11. Design Considerations & Safety

- **Power budgeting:** sufficient, separate/appropriately-rated power must be supplied to both the motor/driver circuit and the ESP32‑CAM, since camera modules are current-sensitive and prone to brownout under shared/high loads.
- **Error handling & safety mechanisms:** the design calls for safeguards against accidental motor activation or pump misfires to prevent damage or injury; `robocop.ino`'s low-water-level pump inhibition is one concrete example of this principle implemented in code.
- **Idle-state safety:** `robocop.ino` boots with the relay HIGH (pump off), a safer default than `RemoteControlCar.ino`, which boots with the relay LOW (pump on) — worth fixing if you build from the latter file.
- **Staged testing:** the car should be tested in a controlled environment before deployment in more complex, real-world scenarios (e.g., an actual warehouse floor).

---

## 12. Issues Faced & How They Were Handled

The team documented the following practical challenges during development:

- **PIR sensor sensitivity tuning** — calibrating the PIR sensor's sensitivity/detection range required iterative adjustment to avoid false positives/negatives.
- **Servo calibration for water extraction** — tuning the servo motor's rotation/position to extract a precise, controlled amount of water from the pump was non-trivial.
- **Unstable PWM signals** — running more than one PWM signal simultaneously (e.g., multiple motors/servo together) caused signal instability.
- **Brownout under high current draw** — the ESP32‑CAM experienced brownout resets due to high current consumption when motors and camera drew power simultaneously.
- **IR flame sensors affected by external infrared** — the IR sensors proved sensitive to ambient/external infrared sources beyond actual flames, causing potential false triggers.

*(The earlier project-report draft records a subset of these — PIR sensitivity tuning and water-pump/servo calibration — as the two headline issues; the final paper expands this list with the PWM instability, brownout, and external-IR interference findings.)*

---

## 13. Note on Project Documentation History

Two source documents describe this project, representing two stages of the same effort:

1. **`MMBD_Project_Report.docx`** — an earlier, more verbose project report (EC310, Syndicate‑B, submitted 30/12/2024, supervised by Asst. Prof. Dr. Sagheer Khan and LE Ayesha Khanam). It documents a slightly different Remote Control Car wiring scheme (4 independently-wired motors across 8 L298N outputs, generic GPIO labels `D1–D4`) and lists two issues faced.
2. **`MMBD_Project.pdf`** — the final, polished, IEEE-conference-style paper (course instructor LE Umair Khalil) used as the primary source for this README. It documents the refined 2-side differential drive wiring (explicit GPIOs 14/15/13/12 → IN1–IN4), adds the MQ2 smoke sensor into the fire-response logic, and reports five issues faced.

**Neither document is present in this repository.** This README follows the **final PDF's** described design as the authoritative source for hardware specifications not derivable from code (pin maps for the PIR/Remote-Control-Car subsystem, BOM, issues faced), since it represents the completed, most-refined version of the build. Where the *actual committed code* differs from the report — most notably sensor polarity ([§6.4](#64-sensor-polarity--a-documentation-discrepancy-worth-knowing)) and the absence of car/PIR firmware ([§6](#6-firmware-walkthrough), [§8](#8-subsystem-2--remote-control-car-mobility--vision-unit)) — this README flags the code as the more reliable, currently-verifiable source.

---

## 14. Conclusion

The ESP32 Robo Cop car project successfully demonstrates the integration of embedded systems and robotics to create a low-cost, efficient, and autonomous security and fire-safety solution. The system reliably detects intruders using a PIR sensor and alerts users via a buzzer. The addition of IR flame sensors enables fire detection, with the system autonomously activating a water pump through a relay to mitigate fire hazards. The project demonstrates how embedded systems and robotics can address real-world challenges in industrial security and fire safety at low cost, laying groundwork for more sophisticated autonomous solutions.

---

## 15. Future Work

Building on the issues and limitations identified during development, natural next steps include:

- Improving PWM signal stability when driving multiple actuators (motors + servo) concurrently, e.g. via dedicated PWM driver ICs or hardware timers.
- Adding dedicated power regulation/decoupling for the ESP32‑CAM to eliminate brownout under combined motor + camera load.
- Shielding or filtering the IR flame sensors to reduce false triggers from ambient/external infrared sources.
- Integrating the ESP32‑CAM's live video feed into the remote-control web interface for visual situational awareness during patrol.
- Closed-loop water-level monitoring in the tank — largely delivered by `robocop.ino`'s HW‑038 integration; remaining work is calibrating thresholds per physical build rather than adding the capability from scratch.
- Expanding intruder/fire alerting to networked notifications (e.g., push alerts, SMS, or a central monitoring dashboard) rather than only a local buzzer.
- Committing the actual ESP32‑CAM Wi‑Fi/motor-driver firmware and a PIR-integrated build of the Robo Cop sketch to this repository, so the documented architecture and the shipped code are fully in sync.
- Reconciling the sensor active-HIGH/active-LOW polarity between the report's wiring narrative and the firmware's `== LOW` checks (§6.4), ideally with inline code comments citing each sensor's datasheet.

---

