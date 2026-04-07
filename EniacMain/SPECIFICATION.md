# LTC-ESP32 ENIAC Firmware Specification

**Version:** 0.6.0.8
**Platform:** ESP32 (DOIT DevKit v1)
**Framework:** Arduino / PlatformIO

---

## 1. Overview

The LTC-ESP32 ENIAC firmware is embedded software for a WiFi-connected Nixie tube clock system. It provides time display with network synchronization, customizable display modes, motion detection-based blanking, LED backlighting, and comprehensive configuration through a web interface.

### 1.1 Key Features

- 6-digit Nixie tube display with anti-cathode poisoning (ACP) protection
- Multiple time sources: GPS, NTP, RTC with automatic fallback
- Addressable RGB LED backlighting (NeoPixel/WS2812B)
- Adaptive brightness via light sensor (LDR)
- Motion detection (PIR) with configurable blanking
- Web-based configuration interface
- OLED menu system with rotary encoder
- Over-the-air (OTA) firmware updates
- Secondary display support (Nixie slave, Decatron)
- Financial ticker display (cryptocurrency prices)

---

## 2. Hardware Requirements

### 2.1 Microcontroller

| Component | Specification |
|-----------|---------------|
| MCU | ESP32 (DOIT DevKit v1) |
| Flash | 4MB minimum (8MB/16MB supported) |
| Framework | Arduino via PlatformIO |

### 2.2 GPIO Assignments

#### Display Control (Shift Registers)
| Signal | GPIO | Description |
|--------|------|-------------|
| CLK | 19 | Shift register clock |
| BLANK | 18 | Display blanking |
| DATA1 | 23 | Channel 1 data |
| LATCH1 | 17 | Channel 1 latch |
| DATA2 | 26 | Channel 2 data |
| LATCH2 | 27 | Channel 2 latch |
| DATA3 | 32 | Channel 3 data |
| LATCH3 | 33 | Channel 3 latch |

#### User Input
| Signal | GPIO | Description |
|--------|------|-------------|
| ENC_A | 5 | Encoder channel A |
| ENC_B | 14 | Encoder channel B |
| ENC_SW | 16 | Encoder button |
| TOUCH1 | 15 | Touch switch 1 |
| TOUCH2 | 4 | Touch switch 2 |
| TOUCH3 | 12 | Touch switch 3 |

#### Sensors
| Signal | GPIO | Description |
|--------|------|-------------|
| LDR | 34 | Light sensor (analog) |
| PIR | 35 | Motion sensor (analog) |

#### Other
| Signal | GPIO | Description |
|--------|------|-------------|
| NEOPIXEL | 13 | LED data output |
| DECATRON_TX | 0 | UART2 TX — serial link to Decatron slave |
| I2C_SDA | 21 | I2C data |
| I2C_SCL | 22 | I2C clock |
| STATUS_LED | 2 | Onboard status LED |

### 2.3 Supported Peripherals

- **OLED Display:** SSD1306 (0.96", 2.4") or SH1106 (1.3") via I2C
- **RTC:** DS3231 or compatible at I2C address 0x68
- **GPS:** UART-based GPS module (NMEA GPZDA sentences)
- **NeoPixel:** WS2812B addressable LEDs (2 per tube + separators)
- **Blinkenlights:** 6-LED status indicator array

---

## 3. Software Architecture

### 3.1 Design Pattern

The firmware uses a **manager-based singleton pattern**:

```
Main Loop (setup/loop)
    │
    ├── Peripheral Managers (Timers, Input handlers)
    │
    ├── Functional Managers (Time, Display, LED, Blanking)
    │
    ├── Hardware Abstraction (GPIO, I2C, SPI, Serial)
    │
    └── Physical Hardware
```

### 3.2 Core Managers

| Manager | Responsibility |
|---------|----------------|
| OutputManager | Display control, digit output, transitions, stunts |
| LEDManager | NeoPixel backlighting, color strategies |
| BlankingManager | Display blanking modes, PIR override |
| WiFiManager | Network connectivity, AP mode, WPS |
| NTPManager | Network time synchronization |
| GPSManager | GPS time parsing (NMEA) |
| RTCManager | Battery-backed RTC interface |
| TZManager | Timezone and DST handling |
| LDRManager | Adaptive brightness control |
| BlinkenlightsManager | Status LED indicators |
| MenuManager | OLED menu navigation |
| CountdownManager | Countdown timer |
| QuoteManager | Financial ticker via UDP with 6 trend indicators |
| TransitionManager | Display transition effects |
| SpiffsStorage | Configuration persistence |
| WebManager | REST API and web server |
| TimerManager | Hardware timer interrupts |
| SlaveManagerNixie | I2C slave display control |
| SlaveManagerDecatron | UART serial control of Decatron slave (Serial2 on GPIO0) |

### 3.3 Initialization Sequence

1. Serial console initialization (115200 baud)
2. GPIO configuration
3. SPIFFS initialization and config loading
4. Timer setup for display refresh
5. LDR manager initialization
6. I2C bus initialization
7. OLED initialization
8. LED (NeoPixel) initialization
9. WiFi setup
10. Timezone manager initialization
11. RTC testing and time sync
12. NTP manager initialization
13. GPS manager initialization
14. Blanking manager initialization
15. Menu manager setup
16. Watchdog timer enablement
17. Slave device initialization

### 3.4 Main Loop

The main loop processes:
1. Millisecond counter updates
2. Diagnostic mode operations
3. LDR adaptive dimming
4. Menu processing
5. Encoder handling
6. Switch event handling
7. Display refresh
8. Once-per-second tasks
9. Once-per-hour tasks
10. WiFi maintenance
11. Watchdog feeding

---

## 4. Display System

### 4.1 Display Modes

| Mode | Description |
|------|-------------|
| Primary | Current time with configured format |
| Secondary | Alternate display (date, value, countdown) |
| Value | Arbitrary 6-digit value |
| ACP | Anti-cathode poisoning animation |
| Slots | Random digit animation |
| Diagnostics | Self-test mode |
| Ticker | Financial data display with per-digit trend indicators |

### 4.2 Transition Effects

| Effect | Description |
|--------|-------------|
| Wipe | Scroll digits in/out |
| Bang | Instant change |
| Scramble | Random digit display before settling |

### 4.3 Anti-Cathode Poisoning (ACP)

Rotates all digits through 0-9 at configurable intervals:
- 1 minute
- 10 minutes
- 1 hour

### 4.4 Separator Modes

The colon separator between digit pairs supports multiple blink modes:
- Off
- On (steady)
- Blink (1 second)
- Blink (0.5 second)
- PWM fade

### 4.5 Financial Ticker Display

When configured in Ticker mode, the display shows cryptocurrency prices (BTC/USD) fetched from a UDP quote server. Each digit's LED backlight indicates the price trend for a specific time period.

#### 4.5.1 UDP Protocol

| Command | Response Format | Description |
|---------|-----------------|-------------|
| IBTCUSDT | `dddddd;iiiiii` | Integer price with 6 trend indicators |

- `dddddd`: 6-digit price value (zero-padded)
- `iiiiii`: 6 trend indicator characters

#### 4.5.2 Trend Indicators

Each digit position corresponds to a different time period:

| Digit | Position | Period | Description |
|-------|----------|--------|-------------|
| H10 | 0 | Yesterday | Price at previous midnight UTC |
| H1 | 1 | Today | Price at current midnight UTC |
| M10 | 2 | 4 hours | Price at 4-hour block boundary |
| M1 | 3 | 1 hour | Price at hour boundary |
| S10 | 4 | 15 minutes | Price at 15-minute block boundary |
| S1 | 5 | 1 minute | Price at minute boundary |

#### 4.5.3 Indicator Values and LED Colors

| Indicator | Meaning | LED Color |
|-----------|---------|-----------|
| U | Price is above snapshot | Green |
| D | Price is below snapshot | Red |
| - | Price equals snapshot or no data | Off/dim |

#### 4.5.4 Example

Response `077485;UU-UDD` displays:
- Price: 77485
- H10 (Yesterday): Green (up)
- H1 (Today): Green (up)
- M10 (4h): Off (unchanged)
- M1 (1h): Green (up)
- S10 (15m): Red (down)
- S1 (1m): Red (down)

---

## 5. Time Management

### 5.1 Time Sources (Priority Order)

1. **GPS** - Most accurate, GPZDA NMEA parsing, 4-minute validity window
2. **NTP** - Network-based, configurable pool (default: pool.ntp.org)
3. **RTC** - Local battery-backed DS3231 at I2C 0x68
4. **Internal** - ESP32 system clock (fallback)

### 5.2 NTP Configuration

| Parameter | Default | Range |
|-----------|---------|-------|
| Server Pool | pool.ntp.org | Any valid hostname |
| Update Interval | 3600s | 60s - 86400s |
| Retry Delay | 60s | Configurable |

### 5.3 Timezone Support

- POSIX timezone string format
- Automatic DST calculation
- Zone database stored in SPIFFS (`/config/zones.json`)

---

## 6. LED Backlighting

### 6.1 Configuration

| Parameter | Description |
|-----------|-------------|
| Pixels per tube | 2 (configurable) |
| Separator LEDs | Enabled by default |
| LED Type | WS2812B |
| Data Pin | GPIO 13 |

### 6.2 Color Strategies

| Strategy | Description |
|----------|-------------|
| Fixed | Single static color |
| Cycling | Rainbow color rotation |
| Color by Time | Hue based on time of day |
| Day of Week | Different color per day |

### 6.3 Brightness Control

- LDR-based adaptive dimming
- Separate curves for tubes and LEDs
- Non-linear compensation for human perception
- Manual min/max override available

---

## 7. Blanking System

### 7.1 Blanking Modes

| Mode | Description |
|------|-------------|
| Never | Display always on |
| Weekday | Blank during weekday hours |
| Weekend | Blank during weekend hours |
| Always | Blank during configured hours |
| Hours | Custom hourly schedule |

### 7.2 Blanking Targets

- Tubes only
- LEDs only
- Both tubes and LEDs
- Tower separators (independent control)

### 7.3 PIR Motion Detection

- Analog input on GPIO 35
- Configurable activation threshold
- Timeout-based blanking override
- Separate configuration for tubes/LEDs

---

## 8. Network Communication

### 8.1 WiFi Modes

| Mode | Description |
|------|-------------|
| Station (STA) | Connect to existing AP |
| Access Point (AP) | Configuration portal |
| WPS | WiFi Protected Setup |
| SmartConfig | ESP-Touch configuration |

### 8.2 mDNS

Device accessible via `esp32-xxxxx.local` where `xxxxx` is derived from MAC address.

### 8.3 Protocols

| Protocol | Port | Purpose |
|----------|------|---------|
| HTTP | 80 | Web interface, REST API |
| NTP | 123 | Time synchronization |
| UDP | 2222 | Quote server (tickering) |

---

## 9. REST API

### 9.1 Endpoints

#### Status & Diagnostics
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /api/getSummary | Current status and time |
| GET | /api/getDiags | Detailed diagnostics |
| POST | /api/postDiags | Update diagnostic settings |

#### Time Configuration
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /api/getTimeserver | NTP configuration |
| POST | /api/postTimeserver | Update NTP settings |
| GET | /api/getZonesList | Available timezones |

#### Clock Configuration
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /api/getConfig | Full configuration |
| POST | /api/postConfig | Update configuration |

#### WiFi
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /api/credentials | WiFi status |
| POST | /api/postWiFiCredentials | Set WiFi credentials |

#### Utilities
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /utils/scanI2C | Discover I2C devices |
| GET | /utils/scanSPIFFS | List SPIFFS files |
| GET | /utils/saveStats | Save statistics |
| GET | /utils/ntpupdate | Force NTP sync |
| GET | /utils/resetoptions | Factory reset config |
| GET | /utils/resetall | Full factory reset |
| GET | /utils/restart | Restart device |

#### OTA Updates
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /update | ElegantOTA interface |

---

## 10. Configuration

### 10.1 Compile-Time Options (Configuration.h)

| Option | Description |
|--------|-------------|
| CONFIG_ENIAC | Main board configuration |
| CONFIG_MINIAC | Alternate board configuration |
| DEBUG | Enable/disable debug output |
| OLED_TYPE | SSD1306 or SH1106 |
| NEOPIXEL_BACKLIGHTS | Enable LED backlighting |
| PIXELS_PER_TUBE | LEDs per Nixie tube |
| BLINKENLIGHTS | Enable status indicators |
| MENU | Enable OLED menu system |
| SLAVE_DECATRON | Enable Decatron slave (UART serial) |
| TICKER | Enable financial ticker |

### 10.2 Runtime Configuration (SPIFFS)

Configuration stored as JSON in SPIFFS filesystem:

| Category | Parameters |
|----------|------------|
| Network | NTP pool, update interval, WiFi credentials |
| Display | Hour mode, dimming levels, fade/scroll settings |
| Blanking | Mode, hours, PIR timeout |
| LED | Mode, color, cycle speed, hue offset |
| ACP/Slots | Animation modes and timing |
| Countdown | Target date/time |
| Authentication | Web interface credentials |
| Slave | Secondary display settings |
| Switches | Button function mapping |

---

## 11. Secondary Displays

### 11.1 Nixie Slave (I2C)

- I2C Address: 0x69
- Packet Format: `Mode | Second | DoM | Month | Dimming%`
- Modes: 100ths, date, seconds, off

### 11.2 Decatron Slave (UART)

- UART serial via ESP32 Serial2, TX on GPIO0 at 115200 baud 8N1
- 5-byte packet sent once per second: `0xAA | Hour | Minute | Second | Control`
- Control byte: bit 0 = blanked; bits 1–4 = primary display mode
- Slave auto-blanks and disables HV if no packet received for >5 seconds

---

## 12. Build System

### 12.1 Requirements

- PlatformIO Core or IDE
- ESP32 Arduino framework (espressif32 platform)

### 12.2 Build Targets

| Target | Flash Size | Description |
|--------|------------|-------------|
| esp32-rz568 | 4MB | Default build |
| esp32-rz568-4MB-Eric | 4MB | Eric variant |
| esp32-rz568-8MB | 8MB | Extended flash |
| esp32-rz568-16MB | 16MB | Large flash |
| esp32-rz568-16MB-Eger | 16MB | Eger variant |

### 12.3 Dependencies

```ini
paulstoffregen/Time@^1.6.1
bblanchon/ArduinoJson@5.13.4
adafruit/Adafruit GFX Library@^1.11.3
adafruit/Adafruit SSD1306@^2.5.7
adafruit/Adafruit BusIO@^1.13.2
makuna/NeoPixelBus@^2.7.0
adafruit/Adafruit SH110X@^2.1.8
ayushsharma82/ElegantOTA@^3.1.6
esp32async/ESPAsyncWebServer@^3.6.1
```

### 12.4 Build Commands

```bash
# Build
pio run -e esp32-rz568

# Upload via serial
pio run -e esp32-rz568 -t upload

# Upload filesystem
pio run -e esp32-rz568 -t uploadfs

# Monitor serial output
pio device monitor -b 115200
```

---

## 13. File Structure

```
EniacMain/
├── src/
│   ├── EniacMain.ino          # Main entry point
│   ├── *Manager.cpp           # Manager implementations
│   └── *.cpp                  # Other source files
├── include/
│   ├── Configuration.h        # Compile-time options
│   ├── Defs.h                 # GPIO and constants
│   ├── StorageTypes.h         # Configuration structures
│   └── *Manager.h             # Manager headers
├── data/
│   ├── config/
│   │   └── zones.json         # Timezone database
│   └── web/
│       ├── clockconfig.html   # Configuration page
│       └── *.html/css/js      # Web assets
├── platformio.ini             # Build configuration
└── partitions/
    └── *.csv                  # Flash partition schemes
```

---

## 14. Statistics Tracking

The firmware tracks operational statistics:

| Statistic | Description |
|-----------|-------------|
| Uptime | Total operating time (minutes) |
| Tube On-Time | Display active time (minutes) |

Statistics are persisted to SPIFFS and can be viewed/saved via the web interface.

---

## 15. Safety Features

### 15.1 Watchdog Timer

- 5-second timeout
- Automatic reset on hang
- Fed in main loop

### 15.2 Time Source Failover

Automatic fallback through GPS -> NTP -> RTC -> Internal clock.

### 15.3 WiFi Recovery

- Auto-reconnect with stored credentials
- AP mode fallback for reconfiguration
- Button-triggered emergency AP mode

---

## 16. Version History

| Version | Changes |
|---------|---------|
| 0.6.0.8 | New UART serial protocol for Decatron slave; more Decatron display options; stop blue LED flashing when blanked; remove cog crank output (GPIO0 repurposed as UART TX) |
| 0.6.0.7 | Enhanced ticker with 6 per-digit trend indicators |
| 0.6.0.6 | Fix tower blanking, fix quote server |
| 0.6.0.5 | Add UDP server, update blanking mode manager |
| 0.6.0.4 | Reverse Digit (RD) tag in features |
| 0.6.0.3 | Backlight corrections |
| 0.6.0.2 | Tower dimming, event-driven switches |
| 0.6.0.1 | Split dimming for tubes and LEDs |
| 0.6.0.0 | Decatron slave support, ticker functionality |

---

## 17. License and Attribution

This firmware is developed for the LTC-ESP32 Nixie clock platform.

**Quote Server:** tzs.nixieclock.biz:2222 (UDP protocol for financial ticker data with trend indicators)
