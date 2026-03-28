# EniacDecatron Firmware Specification

## Overview

EniacDecatron is the firmware for a dual Decatron cold-cathode decade counter tube slave display. It runs on a Wemos D1 Mini (ESP8266) and acts as an I2C slave, receiving time and status from the EniacMain ESP32 master. The two Decatron tubes are driven to display seconds as a rotating glow.

---

## Hardware

### Platform

| Property       | Value                        |
|----------------|------------------------------|
| Board          | Wemos D1 Mini (ESP8266)      |
| Framework      | Arduino                      |
| CPU Frequency  | 160 MHz                      |
| Flash Size     | 4 MB (FS: 2 MB, OTA: ~1 MB) |
| Serial Baud    | 115200                       |

### Pin Allocations

| Signal    | Pin | Direction | Description                               |
|-----------|-----|-----------|-------------------------------------------|
| Guide1_1  | D4  | Output    | Tube 1 guide cathode G1                   |
| Guide2_1  | D3  | Output    | Tube 1 guide cathode G2                   |
| Index1    | D0  | Input     | Tube 1 index mark (LOW when at K0)        |
| Guide1_2  | D6  | Output    | Tube 2 guide cathode G1                   |
| Guide2_2  | D5  | Output    | Tube 2 guide cathode G2                   |
| Index2    | D7  | Input     | Tube 2 index mark (LOW when at K0)        |
| HVEnable  | D8  | Output    | High-voltage generator enable (HIGH = on) |

I2C uses the default D1 Mini pins (SDA = D2, SCL = D1) in slave mode.

| Signal    | Pin | Direction     | Description                           |
|-----------|-----|---------------|---------------------------------------|
| I2C SCL   | D1  | Bidirectional | I2C Clock                             |
| I2C SDA   | D2  | Bidirectional | I2C Data                              |

---

## I2C Interface

| Property        | Value |
|-----------------|-------|
| Role            | Slave |
| Address         | 106   |
| Packet size     | 4 bytes, received once per second from master |

### Packet Format

| Byte | Content  | Range |
|------|----------|-------|
| 1    | Hours    | 0–23  |
| 2    | Minutes  | 0–59  |
| 3    | Seconds  | 0–59  |
| 4    | Control  | —     |

### Control Byte

| Bits | Mask  | Description                              |
|------|-------|------------------------------------------|
| 0    | 0x01  | Blanked (1 = display should be blanked)  |
| 1–4  | 0x1E  | Primary display mode from master         |

Extra bytes beyond 4 are drained and discarded.

---

## Decatron Tube Mechanics

Each Decatron is a cold-cathode decade counter tube with 10 main cathodes (K0–K9) and 2 guide cathodes (G1, G2). Stepping through the 3-phase guide sequence advances the glow by one cathode position.

### Step Encoding

Each cathode position consists of 3 guide phases:

| Phase | G1   | G2   |
|-------|------|------|
| 0     | LOW  | LOW  |
| 1     | HIGH | LOW  |
| 2     | LOW  | HIGH |

### Position Arithmetic

```
position = (phaseStep) + (digitStep × 3)
```

- `digitStep`: current cathode (0–9)
- `phaseStep`: current guide phase within that cathode (0–2)
- `position`: absolute step around the tube (0–29, 30 steps total per revolution)

**Step forward** (glow moves forward): decrement phase; on underflow, decrement cathode.
**Step backward** (glow moves backward): increment phase; on overflow, increment cathode.

---

## Homing Sequence

On startup and whenever the HV generator is re-enabled after blanking, both tubes are homed to find the index mark (K0):

1. Step each tube **forward** until its Index input reads LOW. This sweeps past K0.
2. Step each tube **backward** until its Index input reads LOW again. This lands precisely on K0; reset `digitStep` and `phaseStep` to 0.
3. Take 3 more **forward** steps on each tube. This moves off the index mark to the true Top Dead Centre (TDC) reference position.
4. Record `currentPos` as `tdc` for each tube. Set `expPos = tdc`.

Both tubes are homed in parallel within the same loop to minimise startup time.

---

## Blanking

Blanking is asserted if either condition is true:

- Control byte bit 0 is set (master requests blanking).
- No I2C packet received for more than **5000 ms** (master timeout / disconnected).

When blanked: HV generator is disabled (`HVEnable` LOW).
When unblanking: HV generator is re-enabled, then the full homing sequence runs before normal stepping resumes.

---

## Normal Operation Loop

Each loop iteration runs with a **3 ms delay**, giving a maximum step rate of ~333 steps/second per tube.

### Seconds Display (Tube 1)

On each received I2C packet, the target position for Tube 1 is calculated:

```
targetPos = (seconds × 30) / 60     // maps 0–59 s → 0–29 steps
expPos1   = (tdc1 + targetPos) % 30
```

Each loop, one step toward `expPos1` is taken if the tube is not already there.

### Tube 2

Tube 2 currently tracks TDC (parked at 12 o'clock). The expected position `expPos2` is set to `tdc2` and the tube is aligned to it each loop. Reserved for future use (e.g. minutes display).

### Per-Second Debug

Once per second, the current and TDC positions of both tubes are logged to serial if debug is enabled:

```
pos: <currentPos1>/<currentPos2> tdc: <tdc1>/<tdc2>
```

---

## Debug

`DebugManager` wraps serial output behind a compile-time flag:

```cpp
#define DEBUG     true
#define DEBUG_OFF false

bool debugVal = DEBUG;   // set to DEBUG_OFF to disable
```

Serial is initialised at 115200 baud when debug is enabled. All debug output goes via `debugManager.debugMsg(String)`.

---

## Software Structure

| File                      | Description                              |
|---------------------------|------------------------------------------|
| `src/main.cpp`            | All application logic                    |
| `include/defs.h`          | Pin definitions, software version        |
| `include/DebugManager.h`  | Debug manager class declaration          |
| `src/DebugManager.cpp`    | Debug manager implementation             |
| `platformio.ini`          | PlatformIO build configuration           |

### Key Global Variables

| Variable       | Type              | Description                                      |
|----------------|-------------------|--------------------------------------------------|
| `digitStep1/2` | `int`             | Current cathode index for each tube (0–9)        |
| `phaseStep1/2` | `int`             | Current guide phase for each tube (0–2)          |
| `currentPos1/2`| `int`             | Absolute step position for each tube (0–29)      |
| `tdc1/2`       | `int`             | Top Dead Centre reference position for each tube |
| `expPos1/2`    | `int`             | Target position each tube is stepping toward     |
| `indexMark1/2` | `int`             | Index mark detection state during homing (-1 = not found) |
| `blanked`      | `boolean`         | Current blanking state                           |
| `rxHour`       | `volatile uint8_t`| Last received hour from master                   |
| `rxMinute`     | `volatile uint8_t`| Last received minute from master                 |
| `rxSecond`     | `volatile uint8_t`| Last received second from master                 |
| `rxControl`    | `volatile uint8_t`| Last received control byte from master           |
| `rxMode`       | `volatile uint8_t`| Primary mode extracted from control byte bits 1–4|
| `i2cDataReceived` | `volatile bool`| Flag set by I2C ISR when new packet arrives      |
| `lastI2CMillis`| `volatile unsigned long` | Timestamp of last received I2C packet   |

---

## Build Configuration

`platformio.ini`:

```ini
[env:d1_mini]
platform  = espressif8266
board     = d1_mini
framework = arduino
board_build.f_cpu = 160000000L
monitor_speed = 115200
```

Upload and monitor ports are configured for Linux (`/dev/ttyUSB*`); change to `/dev/cu.*` for macOS.

---

## Startup Sequence Summary

```
1. Configure GPIO pins (Guide1, Guide2 as outputs; Index as inputs; HVEnable as output)
2. Enable HV generator
3. Home both Decatrons (find index marks, step to TDC)
4. Wait 1 second (TDC visible for inspection)
5. Register I2C slave at address 106, install onReceive callback
6. Record lastI2CMillis = millis() (prevents immediate timeout blank on startup)
7. Enter main loop
```
