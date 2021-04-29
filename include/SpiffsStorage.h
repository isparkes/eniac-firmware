#pragma once

#include <FS.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

// ------------------------ Types ------------------------

typedef void (*DebugCallback) (String);

// Used for holding the config set
typedef struct {
  String ntpPool;
  int ntpUpdateInterval;
  String tzs;
  boolean hourMode;
  int minDim;
  byte dayBlanking;
  boolean scrollback;
  boolean fade;
  byte fadeSteps;
  byte scrollSteps;
  boolean suppressACP;
  int thresholdBright;
  int sensitivityLDR;
  int sensorSmoothCountLDR;
  byte blankHourStart;
  byte blankHourEnd;
  byte blankMode;
  boolean useLDR;
  int pirTimeout;
  boolean usePIRPullup;
  byte backlightMode;
  boolean useBLPulse;
  boolean useBLDim;
  byte redCnl;
  byte grnCnl;
  byte bluCnl;
  byte cycleSpeed;
  byte slotsMode;
  boolean blankLeading;
  byte dateFormat;
  boolean testMode;
  boolean webAuthentication;
  String webUsername;
  String webPassword;
  byte antiGhost;
  byte dpEnable;
  byte acpMode;
  byte pirBlankMode;
  byte alarmMode;
  byte alarmHour;
  byte alarmMinute;
  byte sepMode;  
} spiffs_config_t;

typedef struct {
  unsigned long uptimeMins = 0;
  unsigned long tubeOnTimeMins = 0;
} spiffs_stats_t;

// ----------------------------------------------------------------------------------------------------
// ------------------------------------- SPIFFS Clock Component ---------------------------------------
// ----------------------------------------------------------------------------------------------------

class SPIFFS_CLOCK
{
  public:
    void setDebugOutput(bool newDebug);

    boolean testMountSpiffs();
    boolean getSpiffsMounted();

    boolean getConfigFromSpiffs(spiffs_config_t* spiffs_config);
    void    saveConfigToSpiffs(spiffs_config_t* spiffs_config);

    boolean getStatsFromSpiffs(spiffs_stats_t* spiffs_stats);
    void    saveStatsToSpiffs(spiffs_stats_t* spiffs_stats);

    JsonObject& getConfigAsJsonObject(spiffs_config_t* spiffs_config);

    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  private:
    DebugCallback _dbcb;
    bool _debug = false;
    bool _spiffsMounted = false;

    void debugMsg(String message);                        // print a debug message to the callback
};

// ----------------- Exported Variables ------------------

// Config from SPIFFS
static spiffs_config_t current_config;

// Stats from SPIFFS
static spiffs_stats_t current_stats;

// SPIFFS component
static SPIFFS_CLOCK spiffsStorage;
