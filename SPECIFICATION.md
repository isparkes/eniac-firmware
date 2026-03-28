# ENIAC Firmware Specification

## Overview

ENIAC is firmware for a 6-digit Nixie tube clock, split across two microcontrollers:

- **EniacMain** — ESP32 main controller. Manages time sources, display output, Wi-Fi, web API, NeoPixel LEDs, and a rotary encoder menu.
- **EniacDecatron** — Wemos D1 Mini (ESP8266) slave controller. Drives two Decatron cold-cathode decade counter tubes as a seconds display.

The two boards communicate over I2C: EniacMain is master, EniacDecatron is slave at address 106.

Software version string: `LTC-ESP32 0.6.0.8`

---

## Hardware — EniacMain (ESP32)

### Digit Display

| Signal   | Pin |
|----------|-----|
| CLK      | 19  |
| BLANK    | 18  |
| DATA1    | 23  |
| LATCH1   | 17  |
| DATA2    | 26  |
| LATCH2   | 27  |
| DATA3    | 32  |
| LATCH3   | 33  |

Six digits are driven via three sets of serial shift registers (DATA + LATCH pairs sharing a common CLK). BLANK is an active-high enable pulled low to blank all tubes simultaneously.

Digit positions: `H10 H1 M10 M1 S10 S1` (indices 0–5).

### NeoPixel LEDs

| Signal  | Pin |
|---------|-----|
| LED_DOUT | 13 |

A single NeoPixel chain carries:
- `DIGIT_COUNT * PIXELS_PER_TUBE` backlight pixels (default: 6 tubes × 2 pixels = 12)
- Optionally `DIGIT_COUNT` underlight pixels (`FEATURE_EXT_LEDS`)
- 2 separator tower pixels (`FEATURE_SEP_LED`), interleaved at positions 4 and 9 in the chain

Supported pixel types: WS2812B, APA106.

### User Input

| Signal    | Pin | Description                        |
|-----------|-----|------------------------------------|
| Switch1   | 15  | Touch-capable front panel switch 1 |
| Switch2   | 4   | Touch-capable front panel switch 2 |
| BTN3      | 12  | Auxiliary button                   |
| ENC_A     | 5   | Rotary encoder channel A           |
| ENC_B     | 14  | Rotary encoder channel B           |
| ENC_BTN   | 16  | Rotary encoder push button         |

Proto 2 hardware swaps ENC_B and BTN3 (`PROTO2` define).

### Sensors

| Signal | Pin | Description                          |
|--------|-----|--------------------------------------|
| LDRPin | 34  | Light-dependent resistor (analogue)  |
| PIRPin | 35  | PIR motion detector (analogue input) |

Both pins are analogue-only (no internal pull-up).

### Auxiliary

| Signal | Pin | Description                              |
|--------|-----|------------------------------------------|
| PPSPin | 0   | Pulse-per-second / cog crank GPIO output |
| LED_PIN | 2  | Onboard status LED                       |
| SDA    | 21  | I2C data                                 |
| SCL    | 22  | I2C clock (400 kHz)                      |
| RX0    | 3   | UART0 — GPS NMEA input / debug           |
| TX0    | 1   | UART0 transmit                           |

### Storage

4 MB flash, SPIFFS partition for persistent config and statistics. Supports 8 MB and 16 MB flash variants via alternate partition tables.

---

## Hardware — EniacDecatron (Wemos D1 Mini / ESP8266)

Two Decatron tubes, each with:

| Signal   | Pin | Description                        |
|----------|-----|------------------------------------|
| Guide1_1 | D4  | Tube 1 guide cathode G1            |
| Guide2_1 | D3  | Tube 1 guide cathode G2            |
| Index1   | D0  | Tube 1 index mark (K0 glow detect) |
| Guide1_2 | D6  | Tube 2 guide cathode G1            |
| Guide2_2 | D5  | Tube 2 guide cathode G2            |
| Index2   | D7  | Tube 2 index mark                  |
| HVEnable | D8  | High-voltage generator enable      |

The CPU runs at 160 MHz. Each Decatron has 10 cathodes × 3 guide phases = 30 steps per revolution.

---

## I2C Protocol (EniacMain → EniacDecatron)

Transmitted once per second. 4 bytes:

| Byte | Content         | Range  |
|------|-----------------|--------|
| 1    | Hours           | 0–23   |
| 2    | Minutes         | 0–59   |
| 3    | Seconds         | 0–59   |
| 4    | Control         | —      |

Control byte bit fields:

| Bits | Mask  | Description                        |
|------|-------|------------------------------------|
| 0    | 0x01  | Blanked (1 = display is blanked)   |
| 1–4  | 0x1E  | Primary display mode (`cc->pMode`) |

If no I2C packet is received for >5 seconds, the Decatron slave self-blanks and disables the HV generator.

---

## Software Architecture — EniacMain

All subsystems are implemented as singletons. The main loop runs at approximately 100 Hz (10 ms delay per iteration).

### Main Loop Cadence

| Frequency      | Actions                                                              |
|----------------|----------------------------------------------------------------------|
| Every loop     | LDR update, LED update, display output, menu loop, DNS, switch scan |
| Once per second | NTP check, countdown calc, display update, GPS parse, slave update, blanking, watchdog feed |
| Once per minute | RTC sync, time source arbitration, slave minute update, quote fetch, uptime stats |
| Once per hour  | TZ/DST recalculate, RTC test, cog crank output                      |
| Once per day   | Save stats to SPIFFS                                                 |

### Subsystem Summary

| Manager              | Responsibility                                                                 |
|----------------------|--------------------------------------------------------------------------------|
| OutputManager        | Digit display rendering, display mode arbitration, transitions, stunts         |
| BlankingManager      | Time-based and PIR-based blanking with per-element BlankingAction control      |
| LEDManager           | NeoPixel backlight and separator LED control                                   |
| TransitionManager    | Fade and scroll transition effects between digit values                        |
| NTPManager           | NTP time fetch, configurable pool and update interval                          |
| RTCManager           | DS3231 (or compatible) RTC read/write over I2C                                 |
| GPSManager           | NMEA sentence parser, extracts UTC time from GPS serial data                   |
| TZManager            | POSIX timezone string handling, DST calculation, primary time source arbitration |
| WiFiManager          | STA connection, captive portal (open AP) for initial Wi-Fi setup               |
| WebManager           | Async HTTP server (ESPAsyncWebServer), REST config API, ElegantOTA             |
| MenuManager          | Rotary encoder input and SSD1306/SH1106 OLED menu navigation                  |
| CountdownManager     | Configurable countdown to a target datetime                                    |
| TimerManager         | ESP32 hardware timer — drives the ~10 ms display/LED refresh interrupt         |
| LDRManager           | ADC sampling, smoothing, PWM dimming of tubes and LEDs                         |
| SpiffsStorage        | SPIFFS read/write of config (`spiffs_config_t`) and stats (`spiffs_stats_t`)   |
| BlinkenlightsManager | Neon blinkenlight control                                                      |
| QuoteManager         | UDP crypto/FX ticker fetch from `tzs.nixieclock.biz:2222`                      |
| SlaveManagerDecatron | I2C master — sends time + control byte to Decatron slave once per second       |
| SlaveManagerNixie    | I2C master — alternative Nixie slave (mutually exclusive with Decatron slave)  |
| DebugManager         | Conditional serial debug output with auto-off timer                            |

---

## Display Modes

Modes are prioritised: lower enum value = higher priority.

| Priority | Mode          | Description                                           | Fade/Scroll | Blanking |
|----------|---------------|-------------------------------------------------------|-------------|----------|
| 1        | `acpMode`     | Anti-cathode poisoning cycle                          | No          | No       |
| 2        | `diagsMode`   | Startup digit test / encoder diagnostics              | No          | No       |
| 3        | `secondaryMode` | Alternate display (slots animation trigger)         | Yes         | Yes      |
| 4        | `valueMode`   | Arbitrary value (REST push or encoder)                | Yes         | Yes      |
| 5        | `primaryMode` | Normal time display                                   | Yes         | Yes      |

### Primary / Secondary Display Content

| Code                | Content                        |
|---------------------|--------------------------------|
| `DISPLAY_TIME`      | HH MM SS                       |
| `DISPLAY_DATE`      | Formatted date (see formats)   |
| `DISPLAY_VALUE`     | Arbitrary integer value        |
| `DISPLAY_COUNTDOWN` | Countdown timer                |
| `DISPLAY_TICKER`    | Crypto/FX ticker price         |

### Date Formats

| Code             | Format    |
|------------------|-----------|
| `DATE_FORMAT_YYMMDD` | YY MM DD |
| `DATE_FORMAT_MMDDYY` | MM DD YY |
| `DATE_FORMAT_DDMMYY` | DD MM YY (default) |

### Transition Effects

**Scroll** — digits scroll through values, speed configurable (`SCROLL_STEPS_MIN`=1 … `SCROLL_STEPS_MAX`=8, default 4).

**Fade** — digit brightness fades between values over N display impressions (`FADE_STEPS_MIN`=10 … `FADE_STEPS_MAX`=60, default 25; ~100 impressions/second).

### Stunts

**ACP (Anti-Cathode Poisoning)**

Cycles all cathodes to prevent cathode poisoning. Triggered at second 15 of each interval.

| Mode          | Frequency  |
|---------------|------------|
| `ACP_MODE_NONE` | Disabled |
| `ACP_MODE_1M`   | Every minute |
| `ACP_MODE_10M`  | Every 10 minutes |
| `ACP_MODE_1H`   | Every hour (default) |

**Slots**

Scrambles digits like a slot machine at second 50 before the new value settles.

| Mode               | Behaviour               |
|--------------------|-------------------------|
| `SLOTS_MODE_NONE`  | Disabled                |
| `SLOTS_MODE_WIPE`  | Digit wipe              |
| `SLOTS_MODE_BANG`  | Bang (default)          |
| `SLOTS_MODE_SCRAMBLE` | Scramble all digits  |

### Separator Modes

Separator LEDs/neons between digit pairs:

`SEP_RAILROAD`, `SEP_RAILROAD_X`, `SEP_BLINK_SLOW`, `SEP_BLINK_FAST`, `SEP_BLINK_DBL`, `SEP_ON`, `SEP_OFF`, `SEP_AM_PM`

---

## Blanking

### Day/Time Blanking Modes

| Mode                          | Behaviour                                                   |
|-------------------------------|-------------------------------------------------------------|
| `DAY_BLANKING_NEVER`          | Never blank (default)                                       |
| `DAY_BLANKING_WEEKEND`        | All day Saturday/Sunday                                     |
| `DAY_BLANKING_WEEKDAY`        | All day Monday–Friday                                       |
| `DAY_BLANKING_ALWAYS`         | Always blanked                                              |
| `DAY_BLANKING_HOURS`          | Between `blankHourStart` and `blankHourEnd` every day       |
| `DAY_BLANKING_WEEKEND_OR_HOURS` | Hours on weekdays, all day on weekends                    |
| `DAY_BLANKING_WEEKDAY_OR_HOURS` | Hours on weekends, all day on weekdays                    |
| `DAY_BLANKING_WEEKEND_AND_HOURS` | Hours during weekends only                               |
| `DAY_BLANKING_WEEKDAY_AND_HOURS` | Hours during weekdays only                               |

### Blanking Action

Each clock element has an independently configurable `BlankingAction` that controls its behaviour during a blanking period:

| Value                    | Behaviour                              |
|--------------------------|----------------------------------------|
| `BLANKING_ACTION_NORMAL` | Output unaffected during blanking      |
| `BLANKING_ACTION_DIM`    | Output dimmed during blanking          |
| `BLANKING_ACTION_BLANK`  | Output fully blanked during blanking   |

Per-element controls:

| Config field          | Element                                          | Notes                        |
|-----------------------|--------------------------------------------------|------------------------------|
| `blankModeNeon`       | All neon outputs (shared dim brightness)         | Normal or Dim only           |
| `blankTubes`          | Nixie tube digits                                | Bool — blank or not          |
| `blankSepNeon`        | Separator neon indicators                        | Bool — blank or not          |
| `blankBlinkenLights`  | Neon blinkenlight indicators                     | Bool — blank or not          |
| `blankModeLEDs`       | NeoPixel backlights                              | Full BlankingAction           |
| `blankModeSlave`      | Slave module (Decatron / Nixie slave)            | Full BlankingAction           |
| `blankModeSepTower`   | Separator tower NeoPixels                        | Full BlankingAction           |

### PIR / Motion Detection

| Mode                | Behaviour                                              |
|---------------------|--------------------------------------------------------|
| `MD_OVERRIDE_BLANK` | Motion overrides time-based blanking period            |
| `MD_RESPECT_BLANK`  | Motion will not unblank during a blanking period       |
| `MD_DISABLE`        | Motion detection disabled                              |

Timeout: 60–3600 seconds (default 300 s / 5 minutes).

---

## NeoPixel Backlight Modes

| Mode                    | Description                              |
|-------------------------|------------------------------------------|
| `BACKLIGHT_FIXED`       | Single fixed colour                      |
| `BACKLIGHT_CYCLE`       | Cycle through colours (default)          |
| `BACKLIGHT_COLOUR_TIME` | Digit value maps to a distinct colour    |
| `BACKLIGHT_DAY_OF_WEEK` | Day of week maps to a distinct colour    |

When `FEATURE_TICKER` is enabled, three additional override modes apply based on the price trend direction: `BACKLIGHT_UP`, `BACKLIGHT_DOWN`, `BACKLIGHT_UNCHANGED`.

Cycle speed: 1–10 (default 5). Hue offset: 0–360° (default 30°). Per-tube hue gradient configurable.

---

## Time Sources

Time is arbitrated by TZManager. Sources in preference order:

| Code               | Source                           |
|--------------------|----------------------------------|
| `TIME_SOURCE_GPS`  | GPS NMEA (highest accuracy)      |
| `TIME_SOURCE_NTP`  | NTP (network)                    |
| `TIME_SOURCE_RTC`  | Hardware RTC                     |
| `TIME_SOURCE_INT`  | Internal (fallback, no sync)     |

Timezone is stored as a POSIX TZ string (`cc->tzs`). DST offset is recalculated each hour and on boot.

---

## Persistent Configuration (`spiffs_config_t`)

Stored in SPIFFS. Factory-reset restores all defaults.

| Field               | Description                              |
|---------------------|------------------------------------------|
| `ntpPool`           | NTP server hostname                      |
| `ntpUpdateInterval` | NTP resync interval (seconds)            |
| `tzs`               | POSIX timezone string                    |
| `hourMode`          | 12/24-hour mode                          |
| `blankLeading`      | Blank leading zero on hours              |
| `scrollback`        | Enable scrollback transition             |
| `fade`              | Enable fade transition                   |
| `fadeSteps`         | Fade transition steps (10–60)            |
| `scrollSteps`       | Scroll transition steps (1–8)            |
| `minTubeDim`        | Minimum tube PWM dim level               |
| `maxTubeDim`        | Maximum tube PWM dim level               |
| `setTubeDim`        | Manual tube dim override                 |
| `minBLDim`          | Minimum backlight dim level              |
| `maxBLDim`          | Maximum backlight dim level              |
| `setBLDim`          | Manual backlight dim override            |
| `useLDRTube`        | LDR auto-dim tubes                       |
| `useLDRBL`          | LDR auto-dim backlights                  |
| `useLDRSep`         | LDR auto-dim separator LEDs              |
| `thresholdBright`   | LDR bright threshold                     |
| `sensitivityLDR`    | LDR sensitivity                          |
| `sensorSmoothCountLDR` | LDR smoothing sample count            |
| `dayBlanking`       | Day blanking mode enum                   |
| `blankHourStart`    | Hour-based blank start                   |
| `blankHourEnd`      | Hour-based blank end                     |
| `blankModeNeon`     | BlankingAction for all neon outputs (Normal/Dim shared brightness) |
| `blankTubes`        | Blank nixie tubes during blanking period |
| `blankSepNeon`      | Blank separator neons during blanking period |
| `blankBlinkenLights`| Blank neon blinkenlight indicators during blanking period |
| `blankModeLEDs`     | BlankingAction for NeoPixel backlights   |
| `blankModeSlave`    | BlankingAction for slave module          |
| `blankModeSepTower` | BlankingAction for separator tower NeoPixels |
| `mdTimeout`         | PIR timeout (seconds)                    |
| `mdBlankMode`       | PIR motion detection mode enum           |
| `ledMode`           | Separator LED mode                       |
| `backlightMode`     | Backlight colour mode                    |
| `useBLPulse`        | Enable backlight pulse animation         |
| `useBLDim`          | Enable backlight dimming                 |
| `redCnl`            | Fixed colour red channel (0–15)          |
| `grnCnl`            | Fixed colour green channel (0–15)        |
| `bluCnl`            | Fixed colour blue channel (0–15)         |
| `cycleSpeed`        | Colour cycle speed (1–10)                |
| `backlightDimFactor`| Backlight dim factor (10–100%)           |
| `extDimFactor`      | Underlight dim factor (10–100%)          |
| `hueOffset`         | Per-tube hue offset (0–360°)             |
| `towerHueOffset`    | Tower LED hue offset                     |
| `backlightGradient` | Hue gradient across the LED chain        |
| `slotsMode`         | Slots stunt mode                         |
| `acpMode`           | ACP stunt mode                           |
| `suppressACP`       | Disable ACP                              |
| `sepMode`           | Separator blink mode                     |
| `dateFormat`        | Date display format                      |
| `pMode`             | Primary display content mode             |
| `sMode`             | Secondary display content mode           |
| `sw1Mode`           | Switch 1 function assignment             |
| `sw2Mode`           | Switch 2 function assignment             |
| `alarmMode`         | Alarm mode                               |
| `alarmHour`         | Alarm hour                               |
| `alarmMinute`       | Alarm minute                             |
| `blinkenLightsMode` | Neon blinkenlight mode                   |
| `slaveMode`         | Slave display mode                       |
| `outputOnTime`      | Cog crank output on-time (seconds/hour)  |
| `oledOnTime`        | OLED display on-time (seconds)           |
| `WiFiSSID`          | Wi-Fi network name                       |
| `WiFiPassword`      | Wi-Fi password                           |
| `WifiOnAtStart`     | Connect to Wi-Fi on boot                 |
| `webAuthentication` | Enable web UI basic auth                 |
| `webUsername`       | Web UI username                          |
| `webPassword`       | Web UI password                          |
| `countdownTarget`   | Countdown target datetime string         |

### Statistics (`spiffs_stats_t`)

| Field           | Description                  |
|-----------------|------------------------------|
| `uptimeMins`    | Total uptime in minutes       |
| `tubeOnTimeMins`| Total tube-on time in minutes |

Stats are saved to SPIFFS once per day.

---

## Switch Functions

Each front panel switch can be independently assigned:

| Mode                  | Description                        |
|-----------------------|------------------------------------|
| `SW_NONE`             | No action                          |
| `SW_SLAVE_INHIBIT`    | Disable slave display while held   |
| `SW_MIN_DIM`          | Force minimum dimming while held   |
| `SW_BLANK_LEDS`       | Blank LEDs while held              |
| `SW_COUNTDOWN_INHIBIT`| Pause countdown while held         |

---

## Temporary Display Modes

A short button press while showing time cycles through 5-second info screens:

| Mode               | Display content                       |
|--------------------|---------------------------------------|
| `TEMP_MODE_DATE`   | Current date                          |
| `TEMP_MODE_LDR`    | Normalised LDR value (100–999)        |
| `TEMP_MODE_VERSION`| Firmware version number               |
| `TEMP_IP_ADDR12`   | IP address octets 1 and 2            |
| `TEMP_IP_ADDR34`   | IP address octets 3 and 4            |
| `TEMP_IMPR`        | Display impressions per second        |

---

## Quote / Ticker Service

`QuoteManager` fetches a crypto or FX price via UDP from `tzs.nixieclock.biz` port 2222.

Request: 8-byte packet. Response: 16-byte packet containing the price value and 6 trend direction indicators.

Trend indicators (positions 0–5): Yesterday, Today, 4 h, 1 h, 15 m, 1 m. Values: `U` (up), `D` (down), `-` (unchanged).

When enabled (`FEATURE_TICKER`), the ticker price can be shown as the primary or secondary display content, and the 6 trend indicators drive the backlight colours.

---

## Web Interface

Served by `ESPAsyncWebServer`. Features:

- REST API for reading and writing all configuration fields
- OTA firmware update via ElegantOTA
- Optional HTTP Basic Authentication
- Captive portal mode (open AP) for initial Wi-Fi setup, triggered automatically if Wi-Fi is not connected and the encoder button is held at boot

---

## Build Configuration

Set in `EniacMain/include/Configuration.h`.

### Config Sets

| Define          | Target                      |
|-----------------|-----------------------------|
| `CONFIG_ENIAC`  | Full ENIAC clock (default)  |
| `CONFIG_MINIAC` | Miniac variant              |

### Feature Flags

| Flag                      | Description                                         |
|---------------------------|-----------------------------------------------------|
| `DEBUG` / `DEBUG_OFF`     | Include serial debug output                         |
| `DIGIT_DIAGNOSTICS`       | Encoder-driven digit burn / diagnostic menu entries |
| `COUNTDOWN`               | Countdown timer feature                             |
| `OLED_SSD1306`            | 0.96" or 2.4" SSD1306 OLED                         |
| `OLED_SH1106`             | 1.3" SH1106 OLED                                   |
| `FEATURE_BACKLIGHTS`      | NeoPixel backlight support                          |
| `FEATURE_EXT_LEDS`        | Additional underlight NeoPixels                     |
| `FEATURE_SEP_LED`         | Separator tower NeoPixels                           |
| `PIXELS_PER_TUBE`         | 1 or 2 NeoPixels per tube                           |
| `WS2812B` / `APA106`      | NeoPixel type selection                             |
| `NORMAL_BL_OUTPUT` / `REVERSE_BL_OUTPUT` | Backlight chain direction        |
| `NORMAL_DIGIT_OUTPUT` / `REVERSE_DIGIT_OUTPUT` | Digit output order         |
| `FEATURE_BLINKENLIGHTS`   | Neon blinkenlight support                           |
| `FEATURE_MENU`            | Rotary encoder + OLED menu                         |
| `DECATRON_SLAVE`          | I2C Decatron slave                                  |
| `NIXIE_SLAVE`             | I2C Nixie slave (alternative)                       |
| `COG_CRANK_OUTPUT`        | Hourly pulse on PPSPin for cog/crank mechanism      |
| `FEATURE_TICKER`          | Crypto/FX ticker UDP service                        |
| `NORMAL_SWITCHES` / `INVERT_SWITCHES` | Front panel switch polarity            |
| `PROTO2`                  | Proto 2 hardware pin swap (ENC_B / BTN3)            |

### PlatformIO Targets

| Project       | Environment    | Board                  | Framework |
|---------------|----------------|------------------------|-----------|
| EniacMain     | `esp32-rz568`  | ESP32 DOIT DevKit v1   | Arduino   |
| EniacDecatron | `d1_mini`      | Wemos D1 Mini (ESP8266)| Arduino   |

### Key Libraries (EniacMain)

| Library                  | Purpose                         |
|--------------------------|---------------------------------|
| paulstoffregen/Time      | Arduino time functions          |
| bblanchon/ArduinoJson 5  | JSON config serialisation       |
| Adafruit GFX / SSD1306 / SH110X | OLED display            |
| makuna/NeoPixelBus       | NeoPixel LED output             |
| ayushsharma82/ElegantOTA | Over-the-air firmware update    |
| esp32async/ESPAsyncWebServer | Async HTTP server           |

---

## Decatron Slave Operation (EniacDecatron)

On startup:
1. HV generator enabled.
2. Both Decatrons homed by stepping until the index mark is detected.
3. Top Dead Centre (TDC) recorded for each tube.
4. I2C slave registered at address 106.

Each loop iteration (~3 ms):
- Blanking state evaluated from control byte or I2C timeout.
- If unblanked: step each tube one position toward its expected position.
- Tube 1 tracks seconds: `expPos = (tdc + (seconds * 30 / 60)) % 30`.
- Tube 2 currently holds TDC (available for future use).

On re-enable after blanking: tubes are re-homed before resuming normal operation.
