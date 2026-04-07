# ENIAC Nixie Clock — User Manual

**Firmware version:** LTC-ESP32 0.6.0.8

---

## Contents

1. [Overview](#1-overview)
2. [First-time setup](#2-first-time-setup)
3. [Physical controls](#3-physical-controls)
4. [Checking the IP address and firmware version](#4-checking-the-ip-address-and-firmware-version)
5. [Web interface](#5-web-interface)
6. [Time sources](#6-time-sources)
7. [Display modes](#7-display-modes)
8. [LED backlighting](#8-led-backlighting)
9. [Adaptive dimming](#9-adaptive-dimming)
10. [Blanking and sleep schedule](#10-blanking-and-sleep-schedule)
11. [Motion detection](#11-motion-detection)
12. [Anti-cathode poisoning (ACP)](#12-anti-cathode-poisoning-acp)
13. [Slots animation](#13-slots-animation)
14. [Cryptocurrency ticker](#14-cryptocurrency-ticker)
15. [Countdown timer](#15-countdown-timer)
16. [Decatron secondary display](#16-decatron-secondary-display)
17. [Over-the-air firmware updates](#17-over-the-air-firmware-updates)
18. [Factory reset](#18-factory-reset)
19. [Troubleshooting](#19-troubleshooting)

---

## 1. Overview

The ENIAC clock displays time on six Nixie tubes using an ESP32 microcontroller. It synchronises time from GPS, NTP (internet), or a battery-backed hardware RTC and falls back through these sources automatically. An addressable RGB LED chain behind the tubes provides configurable backlighting. A web interface running on the clock itself gives access to all settings.

An optional Decatron secondary display (Wemos D1 Mini) shows seconds as a rotating glow on two cold-cathode decade counter tubes.

---

## 2. First-time setup

### 2.1 Powering on

Connect power. The tubes will light in sequence as a startup test. The status LED on the ESP32 board flashes during boot.

### 2.2 Connecting to WiFi

On first boot, or whenever the clock cannot connect to a saved network, it creates an open WiFi access point named **ENIAC** (or similar). Connect your phone or computer to this network. A captive portal page opens automatically (if it does not, open a browser and navigate to `192.168.4.1`).

Enter your WiFi network name and password and tap **Save**. The clock restarts and connects to your network.

**Alternative — WPS:** Hold the encoder button during boot to trigger WPS. Press the WPS button on your router within 2 minutes.

### 2.3 Setting the timezone

Once connected to WiFi, open the clock's web interface (see §4 for how to find the address). Go to **Time Settings** and select your timezone from the list. The clock automatically handles daylight saving time changes.

---

## 3. Physical controls

### 3.1 Front panel switches (SW1, SW2)

Two touch-capable switches on the front panel. Their function is configurable in the web interface:

| Function | Behaviour while held |
|----------|----------------------|
| None | No action |
| Min dim | Forces tubes to minimum brightness |
| Blank LEDs | Turns off LED backlights |
| Slave inhibit | Disables the secondary Decatron display |
| Countdown inhibit | Pauses the countdown timer |

**Default:** SW1 = Min dim, SW2 = Slave inhibit.

A **short press** of SW1 while showing the time cycles through temporary info screens (see §4).

### 3.2 Rotary encoder

The encoder knob and its push button are used to navigate the OLED menu (if fitted):

| Action | Result |
|--------|--------|
| Turn | Scroll through menu items |
| Press | Select / confirm |
| Hold during boot | Trigger WPS or enter AP config mode |

---

## 4. Checking the IP address and firmware version

Press SW1 briefly while the clock shows the time. It cycles through these 5-second info screens:

| Screen | Content |
|--------|---------|
| Date | Current date |
| LDR | Light sensor reading (100–999; higher = brighter room) |
| Version | Firmware version |
| IP 1–2 | First two octets of the clock's IP address |
| IP 3–4 | Last two octets of the clock's IP address |
| Impr | Display refresh rate (impressions per second) |

Once you have the IP address, open `http://<ip-address>/` in a browser. The clock is also reachable as `http://esp32-xxxxxx.local/` where `xxxxxx` is derived from the MAC address.

---

## 5. Web interface

All settings are available at the clock's IP address on port 80. If web authentication is enabled, you will be prompted for a username and password (configurable; default is no authentication).

### 5.1 Main sections

| Section | Description |
|---------|-------------|
| Status | Current time, time source, WiFi signal, uptime |
| Display | Hour mode, date format, transition effects |
| Backlights | LED colour mode, fixed colour, cycle speed |
| Dimming | Tube and LED brightness curves, LDR settings |
| Blanking | Sleep schedule, per-element blanking actions |
| Motion | PIR sensor mode and timeout |
| Stunts | ACP interval, slots animation mode |
| Time | NTP server, timezone, sync interval |
| WiFi | Network credentials |
| Ticker | Cryptocurrency display settings |
| Countdown | Target date/time for countdown |
| Secondary | Decatron slave display settings |
| Utilities | Force NTP sync, restart, factory reset |
| Update | Firmware OTA update |

### 5.2 Saving settings

Click **Save** on each section after making changes. Settings are stored in flash memory and survive power cycles.

---

## 6. Time sources

The clock uses the best available time source, falling back automatically:

| Priority | Source | Notes |
|----------|--------|-------|
| 1 | GPS | Most accurate; requires GPS module connected to UART0 |
| 2 | NTP | Internet time; requires WiFi |
| 3 | RTC | Battery-backed DS3231; keeps time through power cuts |
| 4 | Internal | ESP32 system clock; drifts without external sync |

The active time source is shown on the web interface Status page.

### 6.1 NTP settings

Go to **Time Settings** in the web interface:

- **NTP pool:** defaults to `pool.ntp.org`. Change to a regional pool (e.g. `uk.pool.ntp.org`) for potentially faster sync.
- **Sync interval:** how often to re-sync (60–86400 seconds; default 3600).

To force an immediate NTP sync go to **Utilities → Force NTP sync**.

### 6.2 Timezone and DST

Select your timezone from the **Time Settings** dropdown. DST offsets are calculated automatically and updated every hour.

---

## 7. Display modes

### 7.1 Primary display

The main display content shown on the six Nixie tubes:

| Mode | Content |
|------|---------|
| Time | Hours, minutes, seconds (default) |
| Date | Current date |
| Countdown | Time remaining to a target date/time |
| Ticker | Cryptocurrency price |

Set in **Display → Primary mode**.

**Hour mode:** choose 12-hour or 24-hour. In 12-hour mode the leading zero on hours can be blanked.

**Date format:** DD/MM/YY (default), MM/DD/YY, or YY/MM/DD.

### 7.2 Secondary display

A second content item can be shown on a timer or in rotation:

| Mode | Content |
|------|---------|
| None | Only primary mode shown |
| Date | Alternates with time |
| Value | An arbitrary 6-digit number (set via REST API) |
| Countdown | Countdown timer |
| Ticker | Cryptocurrency price |

### 7.3 Transition effects

When digit values change, transitions smooth the update:

**Scroll** — each digit scrolls through intermediate values before landing on the new one. Speed: 1–8 steps (default 4).

**Fade** — digits cross-fade between old and new values. Steps: 10–60 (default 25, at ~100 impressions/second).

Both effects can be enabled or disabled independently in **Display → Transitions**.

### 7.4 Separators

The dot/colon indicators between digit pairs have several modes:

| Mode | Behaviour |
|------|-----------|
| Off | Indicators always off |
| On | Indicators always on |
| Railroad | Alternating left/right tick each second |
| Railroad X | As Railroad but crossways |
| Blink slow | 1-second blink |
| Blink fast | 0.5-second blink |
| Double blink | Two quick blinks per second |
| AM/PM | Indicates morning (AM) or afternoon/evening (PM) |

---

## 8. LED backlighting

The NeoPixel LEDs behind the tubes are configured in **Backlights**.

### 8.1 Colour modes

| Mode | Behaviour |
|------|-----------|
| Fixed | All tubes show one static colour |
| Cycle | Colours rotate through the spectrum continuously |
| Colour by time | Hue is derived from the digit value |
| Day of week | Different colour for each day of the week |

### 8.2 Fixed colour

Set red, green, and blue channels (0–15 each) to choose any colour.

### 8.3 Cycle settings

- **Speed:** 1 (slowest) to 10 (fastest); default 5.
- **Hue offset:** angular offset between adjacent tubes (0–360°); default 30°. A larger offset creates a rainbow spread across the display.
- **Gradient:** enables a gradual hue progression across the LED chain.

### 8.4 Brightness

- **Backlight brightness:** 10–100% of full LED output; default 100%.
- **Dim factor:** applied when the display enters a dimmed blanking state.

---

## 9. Adaptive dimming

The light-dependent resistor (LDR) measures ambient brightness and adjusts display and LED intensity automatically.

### 9.1 Enable/disable

In **Dimming**, LDR-based dimming can be independently enabled for:
- Tubes
- LED backlights
- Separator LEDs

### 9.2 Brightness limits

| Setting | Description |
|---------|-------------|
| Min tube dim | Lowest tube brightness (dark room) |
| Max tube dim | Highest tube brightness (bright room) |
| Min LED dim | Lowest LED brightness |
| Max LED dim | Highest LED brightness |

If LDR dimming is disabled, the **Set dim** value is used as a fixed brightness.

### 9.3 LDR calibration

- **Bright threshold:** LDR reading above which the display goes to maximum brightness.
- **Sensitivity:** how sharply the dimming curve responds to light changes.
- **Smoothing count:** number of LDR samples averaged (reduces flicker in changing light).

The current LDR reading is shown in the temporary info screens (see §4) for calibration reference.

---

## 10. Blanking and sleep schedule

Blanking switches parts of the display off during unwanted hours. Configure in **Blanking**.

### 10.1 Day/time modes

| Mode | Blanks when |
|------|-------------|
| Never | Never (always on; default) |
| Always | During the hours set below, every day |
| Weekday | All day Monday–Friday |
| Weekend | All day Saturday–Sunday |
| Weekday + hours | All day on weekdays; hours only at weekends |
| Weekend + hours | All day on weekends; hours only on weekdays |
| Weekday and hours | Hours during weekdays only |
| Weekend and hours | Hours during weekends only |

**Blank start / Blank end:** hour (0–23) defining the daily blanking window when an hours-based mode is selected.

### 10.2 Per-element blanking actions

Each part of the clock can respond differently to a blanking period:

| Action | Behaviour |
|--------|-----------|
| Normal | Unaffected (stays on) |
| Dim | Reduced to a low brightness |
| Blank | Fully off |

Elements that can be independently configured:

| Element | Options |
|---------|---------|
| Nixie tubes | Normal / Blank |
| LED backlights | Normal / Dim / Blank |
| Separator neons | Normal / Blank |
| Blinkenlight indicators | Normal / Blank |
| Separator tower LEDs | Normal / Dim / Blank |
| Secondary display (Decatron) | Normal / Dim / Blank |

---

## 11. Motion detection

The PIR sensor detects movement in front of the clock and can override the blanking schedule.

Configure in **Motion**:

| Mode | Behaviour |
|------|-----------|
| Override blanking | Motion turns the display on even during a blanking period |
| Respect blanking | Motion is ignored during blanking periods |
| Disabled | PIR sensor not used |

**Timeout:** how long to keep the display on after the last detected motion (60–3600 seconds; default 300 s / 5 minutes).

---

## 12. Anti-cathode poisoning (ACP)

Cathode poisoning occurs when digits that are rarely shown (typically 0 and 1 in the hours position) develop faint shadows from the other cathodes. The ACP routine exercises every cathode regularly to prevent this.

Configure in **Stunts → ACP mode**:

| Mode | Frequency |
|------|-----------|
| Disabled | ACP off (not recommended for long-term use) |
| Every minute | All cathodes cycled at second :15 each minute |
| Every 10 minutes | Cycled at second :15 every 10 minutes |
| Every hour | Cycled at second :15 each hour (default) |

ACP overrides all other display modes for its brief duration and does not trigger transitions or blanking.

---

## 13. Slots animation

The slots animation scrambles the digits like a slot machine before they settle on the new value, triggered at second :50 of each minute.

Configure in **Stunts → Slots mode**:

| Mode | Behaviour |
|------|-----------|
| Off | No animation |
| Wipe | Digits wipe across the display |
| Bang | Digits snap instantly (default) |
| Scramble | All digits spin randomly before landing |

---

## 14. Cryptocurrency ticker

When enabled, the clock can display a live Bitcoin/USD price fetched from an external UDP quote server.

### 14.1 Enable

In **Ticker**, enable the feature and set the server address and port (default: `tzs.nixieclock.biz:2222`).

Set **Primary mode** or **Secondary mode** to **Ticker** in Display settings.

### 14.2 What is shown

The six tubes display the BTC/USD price zero-padded to 6 digits (e.g. `077485` for $77,485).

Each digit's LED backlight independently shows the price trend for a different time period:

| Digit | Period | Green = | Red = |
|-------|--------|---------|-------|
| H10 (leftmost) | Yesterday | Price above midnight-yesterday | Below |
| H1 | Today | Price above midnight-today | Below |
| M10 | 4 hours | Price above 4-hour boundary | Below |
| M1 | 1 hour | Price above hour boundary | Below |
| S10 | 15 minutes | Price above 15-min boundary | Below |
| S1 (rightmost) | 1 minute | Price above minute boundary | Below |

LED off means the price is unchanged from that period's reference snapshot, or data is not yet available.

---

## 15. Countdown timer

The clock can count down to a future target date and time.

### 15.1 Set the target

In **Countdown**, enter the target date and time. Enable countdown display by setting **Primary mode** or **Secondary mode** to **Countdown**.

### 15.2 Pause/resume

If SW2 is assigned the **Countdown inhibit** function (see §3.1), hold it to pause the countdown. The display freezes on the remaining time until you release the switch.

---

## 16. Decatron secondary display

If a Decatron slave module (Wemos D1 Mini with two Decatron tubes) is connected, it shows seconds as a rotating glow on the left tube.

### 16.1 Connection

Connect the Decatron slave UART RX pin (D9) to the EniacMain GPIO0 pin. Both boards must share a common ground.

### 16.2 How it works

The main clock sends a time packet over serial once per second. The Decatron slave steps the glow to the position corresponding to the current second (30 positions per revolution, mapping 0–59 seconds to 0–29 steps).

On power-up the slave homes both tubes by finding the K0 index mark, then positions itself at Top Dead Centre before accepting time packets. If the connection is lost for more than 5 seconds, the slave disables its high-voltage generator and blanks.

### 16.3 Blanking behaviour

The Decatron display follows the blanking action configured for the secondary display in **Blanking**. Because the Decatron has no partial brightness, both Dim and Blank map to full blanking (HV off).

### 16.4 Inhibiting from the front panel

If SW2 is assigned **Slave inhibit**, hold it to disable the Decatron display temporarily (e.g. to reduce electrical noise during photography).

---

## 17. Over-the-air firmware updates

New firmware can be uploaded without removing the clock from its enclosure.

1. Download the new `.bin` firmware file.
2. Open `http://<clock-ip>/update` in a browser.
3. Click **Choose File**, select the `.bin` file, and click **Update**.
4. Wait for the upload progress bar to complete. The clock restarts automatically.

The filesystem (SPIFFS) can also be updated separately by uploading a `.bin` filesystem image to the same page.

---

## 18. Factory reset

### 18.1 Reset configuration only

Restores all settings to factory defaults but keeps WiFi credentials. Open **Utilities → Reset options** in the web interface.

### 18.2 Full reset

Clears all settings including WiFi credentials. Open **Utilities → Reset all**. The clock will reboot into AP mode for first-time WiFi setup.

---

## 19. Troubleshooting

### The clock shows dashes or blanks on startup

The tubes light in sequence during the startup test. If a tube appears dark it may be that:
- The high-voltage supply needs a few seconds to stabilise — wait a moment.
- The tube's shift register chain has a fault — check wiring to that tube's DATA/LATCH pair.

### The clock is not connecting to WiFi

- Check that SSID and password are correct (case-sensitive).
- The ESP32 supports 2.4 GHz networks only — 5 GHz networks will not appear.
- Hold the encoder button at boot to re-enter AP mode and reconfigure credentials.

### The time is wrong after power-on

If no WiFi is available and no GPS is connected, the clock uses the battery-backed RTC. If the RTC battery is flat, time will be incorrect until an NTP sync succeeds. Replace the CR2032 coin cell on the DS3231 module.

### The web interface is not reachable

- Confirm the clock's IP address using the temporary info screens (§4).
- mDNS (`esp32-xxxxxx.local`) may not work on all networks; use the IP address directly.
- If the clock shows an IP of `0.0.0.0` it has not connected — check WiFi setup.

### The Decatron display is blank

- Check the serial wire between EniacMain GPIO0 and the Decatron slave RX pin.
- Verify both boards share a common ground.
- Check that the high-voltage supply on the Decatron board is operational (small indicator LED on the HV module).
- The slave auto-blanks if no time packet is received for 5 seconds — a wiring fault or loose connector will cause this.

### LEDs are the wrong colour or off

- Confirm NeoPixel LED type matches the `WS2812B` or `APA106` compile-time flag.
- Check the LED data wire to GPIO13.
- LED brightness may be at minimum due to LDR dimming — check the LDR reading in the info screens and adjust thresholds.

### How do I access the clock if I forget the web password?

Perform a full factory reset (§18.2) by connecting to the clock's AP and navigating to `http://192.168.4.1/utils/resetall`. This clears the web password along with all other settings.

---

*For firmware source code, build instructions, and REST API reference see the SPECIFICATION.md files in the repository.*
