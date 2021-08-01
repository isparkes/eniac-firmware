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
  int pirTimeout;
  bool usePIRPullup;
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
  bool testMode;
  bool webAuthentication;
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

    bool testMountSpiffs();
    bool getSpiffsMounted();

    bool getConfigFromSpiffs(spiffs_config_t* spiffs_config);
    void    saveConfigToSpiffs(spiffs_config_t* spiffs_config);

    bool getStatsFromSpiffs(spiffs_stats_t* spiffs_stats);
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
