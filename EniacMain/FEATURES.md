# ENIAC Clock Firmware — Feature List

## Display Modes

- **Time**: HH:MM:SS in 12-hour (with optional leading zero suppression) or 24-hour format
- **Date**: Three layout options — YYMMDD, MMDDYY, or DDMMYY
- **Countdown timer**: Target date/time configurable; display units selectable (seconds, minutes, hours, or days)
- **Cryptocurrency ticker**: Bitcoin price as a 6-digit integer with per-digit trend indicators

## Display Effects & Animations

- **Fade transition**: Configurable 10–60 steps between digit changes
- **Scroll transition**: 1–8 step scroll or scroll-back between digit changes
- **Slots scramble**: Four modes — none, wipe, bang, or scramble — on display update
- **Anti-Cathode Poisoning (ACP)**: Cycles all digits to prevent cathode poisoning; three timing options (1 min, 10 min, 1 hour)
- **Startup digit test**: Cycles all digits on boot for visual diagnostics

## Separator Indicators

Eight modes for the neon separator indicators between digit pairs:

- Railroad blink / Railroad X blink
- Slow blink (1 s) / Fast blink (0.5 s) / Double blink
- Steady on / Steady off
- AM/PM indicator

## Temporary Overlay Displays

Button-triggered 5-second overlays on the main display:

- Current date
- LDR (ambient light) sensor reading
- Firmware version
- IP address (shown as two 3-digit groups)
- Display refresh rate (impressions per second)

## Time Sources & Synchronisation

Priority-based automatic failover across four sources:

| Priority | Source | Notes |
|---|---|---|
| 1 | GPS | UART NMEA GPZDA; 4-minute validity window |
| 2 | NTP | Configurable pool server; 60–86400 s update interval |
| 3 | RTC | DS3231 at I²C 0x68; battery-backed |
| 4 | Internal | ESP32 system clock |

## Timezone & DST

- POSIX timezone string format
- Automatic DST calculation, rechecked hourly
- Timezone database stored in SPIFFS (`zones.json`)
- RTC kept in sync with timezone offset applied

## LED Backlighting

- WS2812B or APA106 NeoPixels; 2 pixels per Nixie tube by default
- Optional separator tower LEDs and underlight extension LEDs
- **Color modes**: fixed single colour, cycling rainbow (1–10 speed), colour-by-digit-value, colour-by-day-of-week
- **Brightness**: LDR-adaptive with configurable min/max levels; non-linear PWM compensation for human perception
- **Hue offset**: 0–360° shift across the chain; per-tube gradient control
- **Pulse animation** mode
- **Crypto ticker integration**: per-digit LEDs show trend direction (green = up, red = down)

## Blanking & Motion Detection

**Nine time-based blanking schedules** controlling when tubes and LEDs switch off:

- Never / Always
- Weekdays only / Weekends only
- Between specific hours (daily, or split by weekday/weekend)

**Per-element blanking actions**: each output (tubes, backlights, separators, blinkenlight indicators, slave module) can be set to normal, dim, or full blank independently.

**PIR motion sensor**: three modes — override blanking (motion keeps display on), respect blanking, or disabled. Configurable timeout 60–3600 s.

## Networking

- **WiFi Station mode** with auto-reconnect
- **Access Point mode** for headless configuration
- **WPS** and **SmartConfig (ESP-Touch)** provisioning
- **Captive portal** for first-time WiFi setup
- **mDNS** hostname (`esp32-xxxxx.local`)
- **NTP client** (port 123)
- **UDP quote client** for crypto ticker (port 2222)

## Web Interface & REST API

- Async HTTP server on port 80
- **Over-the-air (OTA) firmware updates** via ElegantOTA (`/update`)
- Optional HTTP Basic Authentication
- 24+ REST endpoints covering:
  - Status & diagnostics
  - Time server configuration
  - Clock display configuration
  - Timezone list
  - WiFi credential management
  - I²C scan, SPIFFS scan
  - Force NTP update, factory reset, restart

## Cryptocurrency Ticker

- UDP protocol to remote quote server
- Bitcoin/USD price as a 6-digit integer
- Six per-digit trend indicators covering time periods: 1-minute, 15-minute, 1-hour, 4-hour, today (midnight UTC), yesterday
- LED colours reflect trend direction per indicator

## Secondary Display — Decatron Slave

- Wemos D1 Mini (ESP8266) connected via UART2 at 115200 baud
- Two Decatron cold-cathode decade counter tubes displaying seconds
- 30-step rotation per tube (10 cathodes × 3 guide phases)
- Automatic homing sequence on startup or re-enable
- Independent HV generator control
- Auto-blanks after 5 s without a valid packet

## Secondary Display — Nixie Slave (alternative to Decatron)

- I²C slave at address 0x69
- Modes: date, seconds, hundredths of seconds
- Per-packet dimming control

## Menu System (OLED + Rotary Encoder)

- SSD1306 (0.96" / 2.4") or SH1106 (1.3") OLED
- Rotary encoder with push button for navigation
- Menu categories: WiFi setup, display settings, blanking, LED settings, time adjustment, timezone, countdown, system utilities
- Configurable OLED on-time: always on, 60 s, or 3600 s
- Flash message overlay system

## Front Panel Switches

- Three configurable touch/push switches
- Per-switch function assignment: slave inhibit, force minimum dimming, LED blanking, countdown pause, or none

## Sensors

- **LDR** (GPIO 34): ambient light sensing for adaptive brightness
- **PIR** (GPIO 35): motion detection for blanking override

## Statistics & Diagnostics

- Total uptime tracking (minutes, persisted daily to SPIFFS)
- Tube on-time tracking (minutes)
- Real-time diagnostics mode accessible via web API
- Serial debug output (compile-time flag)
- I²C and SPIFFS scan utilities via REST API

## Configuration & Storage

- All settings persisted as JSON in SPIFFS
- 80+ parameters covering display, blanking, LED, networking, countdown, switch mapping, and authentication
- Factory reset available via web API or hardware trigger

## Reliability

- **Watchdog timer**: 5-second hardware watchdog, reset on hang
- **Time source failover**: automatic priority cascade
- **WiFi recovery**: auto-reconnect; AP-mode fallback; emergency AP via button
- **RTC validation**: tested on every boot and hourly

## Build Variants & Feature Flags

Key compile-time toggles:

| Flag | Purpose |
|---|---|
| `CONFIG_ENIAC` / `CONFIG_MINIAC` | Board variant selection |
| `FEATURE_BACKLIGHTS` | NeoPixel backlight support |
| `FEATURE_EXT_LEDS` | Underlight extension LEDs |
| `FEATURE_SEP_LED` | Separator tower LEDs |
| `FEATURE_BLINKENLIGHTS` | 6-LED status indicator array |
| `FEATURE_MENU` | Rotary encoder + OLED menu |
| `FEATURE_TICKER` | Cryptocurrency ticker |
| `COUNTDOWN` | Countdown timer |
| `DECATRON_SLAVE` / `NIXIE_SLAVE` | Secondary display type |
| `DEBUG` | Serial debug output |
| `DIGIT_DIAGNOSTICS` | Encoder burn-in diagnostic menu |

## Hardware Platform

- **MCU**: ESP32 (DOIT DevKit v1), 4/8/16 MB flash
- **Framework**: Arduino via PlatformIO
- **RTC**: DS3231 at I²C 0x68
- **OLED**: SSD1306 or SH1106 at I²C
- **GPS**: UART NMEA receiver
- **LEDs**: WS2812B or APA106 NeoPixels
- **Secondary display**: Decatron (ESP8266 UART) or Nixie slave (I²C)
