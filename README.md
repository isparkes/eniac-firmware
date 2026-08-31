# ENIAC Nixie Clock Firmware

Firmware and tooling for the **ENIAC** 6-digit Nixie tube clock, with an
optional dual-Decatron cold-cathode "seconds" display.

Current software version: `LTC-ESP32 0.6.0.8`

See [`SPECIFICATION.md`](SPECIFICATION.md) for full hardware/firmware specs and
[`USER_MANUAL.md`](USER_MANUAL.md) for end-user documentation of the clock's features and
web interface.

## Repository layout

| Directory | Platform | Description |
|---|---|---|
| [`EniacMain`](EniacMain) | ESP32 (Arduino, PlatformIO) | Main clock firmware. Manages time sources (GPS/NTP/RTC), Nixie digit output, NeoPixel LED backlighting, rotary encoder menu, Wi-Fi, web UI/API, and communication with the slave boards. See [`EniacMain/SPECIFICATION.md`](EniacMain/SPECIFICATION.md) and [`EniacMain/FEATURES.md`](EniacMain/FEATURES.md). |
| [`EniacDecatron`](EniacDecatron) | ESP8266 / Wemos D1 Mini (Arduino, PlatformIO) | Slave firmware driving two Decatron cold-cathode decade counter tubes as a rotating seconds display. Receives time/status from `EniacMain` over a unidirectional UART link. See [`EniacDecatron/SPECIFICATION.md`](EniacDecatron/SPECIFICATION.md). |
| [`EniacSlave`](EniacSlave) | Arduino Uno (AVR, PlatformIO) | Slave firmware for an additional Nixie display driven from `EniacMain` over UART. |
| [`DecatronTest`](DecatronTest) | ESP32 (Arduino, PlatformIO) | Standalone serial tester that emulates `EniacMain`'s UART protocol, for bench-testing `EniacDecatron` without the full clock. |
| [`UDPServer`](UDPServer) | Node.js (Docker) | Small UDP "quote of the day" / price ticker server used by the clock's quote and cryptocurrency ticker features. See [`UDPServer/README.md`](UDPServer/README.md). |

## Hardware overview

- **EniacMain** (ESP32) is the master controller. It drives six Nixie tubes via serial shift
  registers, a NeoPixel (WS2812B/APA106) backlight chain, a touch/rotary-encoder front panel,
  and optional LDR/PIR sensors. Time is synchronised with automatic failover across GPS → NTP →
  battery-backed RTC → internal clock.
- **EniacDecatron** (ESP8266) and **EniacSlave** (AVR) are optional secondary displays, each
  driven from `EniacMain` over a simple framed UART protocol (`0xAA` header + time/control
  bytes).
- The web UI served by `EniacMain` is used to provision Wi-Fi and configure the clock's many
  display/behaviour options.

## Building

Each firmware directory is an independent [PlatformIO](https://platformio.org/) project.

```bash
cd EniacMain          # or EniacDecatron / EniacSlave / DecatronTest
pio run               # build
pio run -t upload     # build and flash
pio device monitor     # serial monitor
```

`UDPServer` is a Node.js project, also packaged as a Docker image — see
[`UDPServer/README.md`](UDPServer/README.md) for build/run instructions.

## Documentation

- [`SPECIFICATION.md`](SPECIFICATION.md) — overall hardware/firmware specification
- [`USER_MANUAL.md`](USER_MANUAL.md) — end-user manual (setup, web interface, all display modes)
- [`EniacMain/FEATURES.md`](EniacMain/FEATURES.md) — detailed feature list for the main firmware
- [`EniacMain/README.md`](EniacMain/README.md) — web API examples and release notes
- Per-subproject `SPECIFICATION.md` files for `EniacMain` and `EniacDecatron`
