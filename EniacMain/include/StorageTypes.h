// ************************************************************
// Global types used to hold the config and statistics
// ************************************************************

#pragma once

#include <Arduino.h>

// ------------------------ Types ------------------------

// Used for holding the config set
typedef struct {
  String ntpPool;           // NTP server or pool host name
  int ntpUpdateInterval;    // Seconds between NTP time updates (NTP_UPDATE_INTERVAL_MIN..MAX)
  String tzs;               // POSIX time zone string, e.g. "CET-1CEST,M3.5.0,M10.5.0/3"
  bool hourMode;            // true = 12 hour display, false = 24 hour
  int minTubeDim;           // Tube brightness % in the dark when using the LDR (DIM_MIN..DIM_MAX)
  int maxTubeDim;           // Tube maximum brightness % (saved, not currently used)
  int setTubeDim;           // Fixed tube brightness % when not using the LDR
  int minBLDim;             // Backlight brightness % in the dark when using the LDR
  int maxBLDim;             // Backlight maximum brightness % (saved, not currently used)
  int setBLDim;             // Fixed backlight brightness % when not using the LDR
  byte dayBlanking;         // DayBlankingMode: which days/hours the blanking period covers
  bool scrollback;          // Scroll digits back through the numbers when they roll over to 0
  bool fade;                // Cross fade between old and new digits
  byte fadeSteps;           // Number of steps in a digit cross fade (FADE_STEPS_MIN..MAX)
  byte scrollSteps;         // Number of steps per digit when scrolling back (SCROLL_STEPS_MIN..MAX)
  bool suppressACP;         // Skip anti cathode poisoning while the tubes are at minimum dim
  int thresholdBright;      // LDR reading offset before dimming starts (SENSOR_THRSH_MIN..MAX)
  int sensitivityLDR;       // LDR sensitivity, 200 = normal (SENSOR_SENSIT_MIN..MAX)
  int sensorSmoothCountLDR; // LDR smoothing: higher = slower response to light changes
  byte blankHourStart;      // Hour the blanking period starts (0..23)
  byte blankHourEnd;        // Hour the blanking period ends (0..23)
  byte blankModeNeon;       // BlankingAction (Normal/Dim) shared brightness for all neon outputs
  bool blankTubes;          // Blank nixie tubes during blanking period
  bool blankSepNeon;        // Blank separator neons during blanking period
  bool blankBlinkenLights;  // Blank neon indicators during blanking period
  byte blankModeLEDs;       // BlankingAction for NeoPixel backlights
  byte blankModeSlave;      // BlankingAction for slave module
  byte blankModeSepTower;   // BlankingAction for separator tower NeoPixels
  byte blankFadeSpeed;      // BLANK_FADE_SPEED_*: how fast the neons and LEDs fade when blanking starts or ends
  bool useLDRTube;          // Dim the tubes using the LDR (else use setTubeDim)
  bool useLDRBL;            // Dim the backlights using the LDR (else use setBLDim)
  bool useLDRSep;           // Dim the separator tower LEDs using the LDR
  int mdTimeout;            // Seconds without motion before PIR blanking starts (MD_TIMEOUT_MIN..MAX)
  byte ledMode;             // LED_MODE_* (saved, not currently used)
  byte backlightMode;       // BACKLIGHT_*: fixed colour, colour cycle etc.
  bool useBLPulse;          // Pulse the backlight brightness once per second
  bool useBLDim;            // Dim the backlights with ambient light
  byte redCnl;              // Backlight red channel for fixed colour (COLOUR_CNL_MIN..MAX)
  byte grnCnl;              // Backlight green channel for fixed colour (COLOUR_CNL_MIN..MAX)
  byte bluCnl;              // Backlight blue channel for fixed colour (COLOUR_CNL_MIN..MAX)
  byte cycleSpeed;          // Backlight colour cycle speed (CYCLE_SPEED_MIN..MAX)
  byte slotsMode;           // SLOTS_MODE_*: effect when switching to the secondary display
  bool blankLeading;        // Blank a leading zero on the hours
  byte dateFormat;          // DATE_FORMAT_*: YYMMDD, MMDDYY or DDMMYY
  bool webAuthentication;   // Require a user name and password for the web pages
  String webUsername;       // Web page user name (when webAuthentication is set)
  String webPassword;       // Web page password (when webAuthentication is set)
  byte acpMode;             // ACP_MODE_*: how often anti cathode poisoning runs
  byte mdBlankMode;         // MotionDetectionMode: how the PIR interacts with the blanking period
  byte alarmMode;           // Alarm mode (saved, not currently used)
  byte alarmHour;           // Alarm hour, 0..23 (saved, not currently used)
  byte alarmMinute;         // Alarm minute, 0..59 (saved, not currently used)
  byte sepMode;             // SEP_*: separator neon/LED pattern
  byte backlightDimFactor;  // Backlight brightness scale % (BACKLIGHT_DIM_FACTOR_MIN..MAX)
  byte extDimFactor;        // External (underlight) LED brightness scale % (EXT_DIM_FACTOR_MIN..MAX)
  int  hueOffset;           // Backlight hue offset in degrees (0..359)
  int  towerHueOffset;      // Separator tower LED hue offset in degrees (0..359)
  String WiFiSSID;          // Saved WiFi network name
  String WiFiPassword;      // Saved WiFi password
  bool WifiOnAtStart;       // Connect to the saved WiFi network at start up
  byte blinkenLightsMode;   // BLNKN_MODE_*: status indicators or chase pattern
  byte slaveMode;           // SLAVE_NIX_MODE_* or SLAVE_DECA_MODE_*, depending on the slave fitted
  int  backlightGradient;   // Hue change in degrees across the backlights (0..360)
  byte sw1Mode;             // SW_*: what switch 1 does
  byte sw2Mode;             // SW_*: what switch 2 does
  byte pMode;               // DISPLAY_*: primary display (time, date, value, countdown, ticker)
  byte sMode;               // DISPLAY_*: secondary display, shown by slots mode
  byte oledOnTime;          // OLED_ON_*: always on, off after 1 minute or after 1 hour
  #ifdef COUNTDOWN
  String countdownTarget;   // Countdown target date, "YYYY-MM-DD"
  #endif

  // not saved
  int diagsMode;            // DIGIT_DIAGS_MODE_*: digit diagnostics mode

  bool testMode;            // Set on factory reset (not currently used)
  bool wasSetup;            // Set on factory reset (not currently used)
} spiffs_config_t;

typedef struct {
  unsigned long uptimeMins = 0;
  unsigned long tubeOnTimeMins = 0;
} spiffs_stats_t;

