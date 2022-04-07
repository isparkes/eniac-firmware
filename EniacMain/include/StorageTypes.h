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
  int minDim;
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
  byte blankMode;
  bool useLDR;
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
  int  hueOffset;
  int  towerHueOffset;
  String WiFiSSID;
  String WiFiPassword;
  bool WifiOnAtStart;
  byte blinkenLightsMode;
  byte slaveMode;
  byte outputOnTime;

  // not saved
  int diagsMode;
  
  bool testMode;
  bool wasSetup;
} spiffs_config_t;

typedef struct {
  unsigned long uptimeMins = 0;
  unsigned long tubeOnTimeMins = 0;
} spiffs_stats_t;

