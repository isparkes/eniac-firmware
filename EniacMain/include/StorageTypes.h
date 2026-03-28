// ************************************************************
// Global types used to hold the config and statistics
// ************************************************************

#pragma once

#include <Arduino.h>

// ------------------------ Types ------------------------

// Used for holding the config set
typedef struct {
  String ntpPool;
  int ntpUpdateInterval;
  String tzs;
  bool hourMode;
  int minTubeDim;
  int maxTubeDim;
  int setTubeDim;
  int minBLDim;
  int maxBLDim;
  int setBLDim;
  byte dayBlanking;
  bool scrollback;
  bool fade;
  byte fadeSteps;
  byte scrollSteps;
  bool suppressACP;
  int thresholdBright;
  int sensitivityLDR;
  int sensorSmoothCountLDR;
  byte blankHourStart;
  byte blankHourEnd;
  byte blankModeNeon;       // BlankingAction (Normal/Dim) shared brightness for all neon outputs
  bool blankTubes;          // Blank nixie tubes during blanking period
  bool blankSepNeon;        // Blank separator neons during blanking period
  bool blankBlinkenLights;  // Blank neon indicators during blanking period
  byte blankModeLEDs;       // BlankingAction for NeoPixel backlights
  byte blankModeSlave;      // BlankingAction for slave module
  byte blankModeSepTower;   // BlankingAction for separator tower NeoPixels
  bool useLDRTube;
  bool useLDRBL;
  bool useLDRSep;
  int mdTimeout;
  byte ledMode;
  byte backlightMode;
  bool useBLPulse;
  bool useBLDim;
  byte redCnl;
  byte grnCnl;
  byte bluCnl;
  byte cycleSpeed;
  byte slotsMode;
  bool blankLeading;
  byte dateFormat;
  bool webAuthentication;
  String webUsername;
  String webPassword;
  byte acpMode;
  byte mdBlankMode;
  byte alarmMode;
  byte alarmHour;
  byte alarmMinute;
  byte sepMode;
  byte backlightDimFactor;
  byte extDimFactor;
  int  hueOffset;
  int  towerHueOffset;
  String WiFiSSID;
  String WiFiPassword;
  bool WifiOnAtStart;
  byte blinkenLightsMode;
  byte slaveMode;
  int  backlightGradient;
  byte sw1Mode;
  byte sw2Mode;
  byte pMode;
  byte sMode;
  byte oledOnTime;
  #ifdef COUNTDOWN
  String countdownTarget;
  #endif

  // not saved
  int diagsMode;
  
  bool testMode;
  bool wasSetup;
} spiffs_config_t;

typedef struct {
  unsigned long uptimeMins = 0;
  unsigned long tubeOnTimeMins = 0;
} spiffs_stats_t;

