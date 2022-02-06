#include "SpiffsStorage.h"

//**********************************************************************************
//**********************************************************************************
//*                               SPIFFS functions                                 *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Test SPIFFS
// ************************************************************
bool SpiffsStorage_::testMountSpiffs()
{
  bool mounted = false;
  if (SPIFFS.begin())
  {
    mounted = true;
  }

  return mounted;
}

// ************************************************************
// Retrieve the config from the SPIFFS
// ************************************************************
bool SpiffsStorage_::getConfigFromSpiffs(spiffs_config_t *spiffs_config)
{
  bool loaded = false;

  #ifdef DEBUG_ON
  debugMsg("mounted file system config read");
  #endif
  if (SPIFFS.exists("/config.json"))
  {
    //file exists, reading and loading
    #ifdef DEBUG_ON
    debugMsg("reading config file");
    #endif
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile)
    {
      #ifdef DEBUG_ON
      debugMsg("opened config file");
      #endif
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);

      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());

      // Dump the raw JSON
      //if (_debug) json.printTo(Serial);
      //  debugMsg("\n");

      if (json.success())
      {
        #ifdef DEBUG_ON
        debugMsg("parsed json");
        #endif

        spiffs_config->ntpPool = json["ntp_pool"].as<String>();
        #ifdef DEBUG_ON
        debugMsg("Loaded NTP pool: " + spiffs_config->ntpPool);
        #endif

        spiffs_config->ntpUpdateInterval = json["ntp_update_interval"].as<int>();
        #ifdef DEBUG_ON
        debugMsg("Loaded NTP update interval: " + String(spiffs_config->ntpUpdateInterval));
        #endif

        spiffs_config->tzs = json["time_zone_string"].as<String>();
        #ifdef DEBUG_ON
        debugMsg("Loaded time zone string: " + spiffs_config->tzs);
        #endif

        spiffs_config->hourMode = json["hourMode"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded 12/24H mode: " + String(spiffs_config->hourMode));
        #endif

        spiffs_config->blankLeading = json["blankLeading"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded lead zero blanking: " + String(spiffs_config->blankLeading));
        #endif

        spiffs_config->dateFormat = json["dateFormat"];
        #ifdef DEBUG_ON
        debugMsg("Loaded date format: " + String(spiffs_config->dateFormat));
        #endif

        spiffs_config->dayBlanking = json["dayBlanking"];
        #ifdef DEBUG_ON
        debugMsg("Loaded dayBlanking: " + String(spiffs_config->dayBlanking));
        #endif

        spiffs_config->fade = json["fade"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded fade: " + String(spiffs_config->fade));
        #endif

        spiffs_config->fadeSteps = json["fadeSteps"];
        #ifdef DEBUG_ON
        debugMsg("Loaded fadeSteps: " + String(spiffs_config->fadeSteps));
        #endif

        spiffs_config->scrollback = json["scrollback"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded scrollback: " + String(spiffs_config->scrollback));
        #endif

        spiffs_config->scrollSteps = json["scrollSteps"];
        #ifdef DEBUG_ON
        debugMsg("Loaded scrollSteps: " + String(spiffs_config->scrollSteps));
        #endif

        spiffs_config->thresholdBright = json["thresholdBright"];
        #ifdef DEBUG_ON
        debugMsg("Loaded thresholdBright: " + String(spiffs_config->thresholdBright));
        #endif

        spiffs_config->sensitivityLDR = json["sensitivityLDR"];
        #ifdef DEBUG_ON
        debugMsg("Loaded sensitivityLDR: " + String(spiffs_config->sensitivityLDR));
        #endif

        spiffs_config->minDim = json["minDim"];
        #ifdef DEBUG_ON
        debugMsg("Loaded minDim: " + String(spiffs_config->minDim));
        #endif

        spiffs_config->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        #ifdef DEBUG_ON
        debugMsg("Loaded sensorSmoothCountLDR: " + String(spiffs_config->sensorSmoothCountLDR));
        #endif

        spiffs_config->backlightMode = json["backlightMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded backlight mode: " + String(spiffs_config->backlightMode));
        #endif

        spiffs_config->useBLPulse = json["useBLPulse"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded backlight pulse: " + String(spiffs_config->useBLPulse));
        #endif

        spiffs_config->useBLDim = json["useBLDim"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded backlight dim: " + String(spiffs_config->useBLDim));
        #endif

        spiffs_config->redCnl = json["redCnl"];
        #ifdef DEBUG_ON
        debugMsg("Loaded redCnl: " + String(spiffs_config->redCnl));
        #endif

        spiffs_config->grnCnl = json["grnCnl"];
        #ifdef DEBUG_ON
        debugMsg("Loaded grnCnl: " + String(spiffs_config->grnCnl));
        #endif

        spiffs_config->bluCnl = json["bluCnl"];
        #ifdef DEBUG_ON
        debugMsg("Loaded bluCnl: " + String(spiffs_config->bluCnl));
        #endif

        spiffs_config->blankMode = json["blankMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded blankMode: " + String(spiffs_config->blankMode));
        #endif

        spiffs_config->blankHourStart = json["blankHourStart"];
        #ifdef DEBUG_ON
        debugMsg("Loaded blankHourStart: " + String(spiffs_config->blankHourStart));
        #endif

        spiffs_config->blankHourEnd = json["blankHourEnd"];
        #ifdef DEBUG_ON
        debugMsg("Loaded blankHourEnd: " + String(spiffs_config->blankHourEnd));
        #endif

        spiffs_config->cycleSpeed = json["cycleSpeed"];
        #ifdef DEBUG_ON
        debugMsg("Loaded cycleSpeed: " + String(spiffs_config->cycleSpeed));
        #endif

        spiffs_config->mdTimeout = json["mdTimeout"];
        #ifdef DEBUG_ON
        debugMsg("Loaded mdTimeout: " + String(spiffs_config->mdTimeout));
        #endif

        spiffs_config->useLDR = json["useLDR"];
        #ifdef DEBUG_ON
        debugMsg("Loaded useLDR: " + String(spiffs_config->useLDR));
        #endif

        spiffs_config->thresholdBright = json["thresholdBright"];
        #ifdef DEBUG_ON
        debugMsg("Loaded thresholdBright: " + String(spiffs_config->thresholdBright));
        #endif

        spiffs_config->sensitivityLDR = json["sensitivityLDR"];
        #ifdef DEBUG_ON
        debugMsg("Loaded sensitivityLDR: " + String(spiffs_config->sensitivityLDR));
        #endif

        spiffs_config->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        #ifdef DEBUG_ON
        debugMsg("Loaded sensorSmoothCountLDR: " + String(spiffs_config->sensorSmoothCountLDR));
        #endif

        spiffs_config->slotsMode = json["slotsMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded slotsMode: " + String(spiffs_config->slotsMode));
        #endif

        spiffs_config->webAuthentication = json["webAuthentication"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded webAuthentication: " + String(spiffs_config->webAuthentication));
        #endif

        spiffs_config->webUsername = json["webUsername"].as<String>();
        #ifdef DEBUG_ON
        debugMsg("Loaded webUsername: " + spiffs_config->webUsername);
        #endif

        spiffs_config->webPassword = json["webPassword"].as<String>();
        #ifdef DEBUG_ON
        debugMsg("Loaded webPassword: " + spiffs_config->webPassword);
        #endif

        spiffs_config->acpMode = json["acpMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded acpMode: " + String(spiffs_config->acpMode));
        #endif

        spiffs_config->mdBlankMode = json["mdBlankMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded mdBlankMode: " + String(spiffs_config->mdBlankMode));
        #endif

        spiffs_config->alarmMode = json["alarmMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded alarmMode: " + String(spiffs_config->alarmMode));
        #endif

        spiffs_config->alarmHour = json["alarmHour"];
        #ifdef DEBUG_ON
        debugMsg("Loaded alarmHour: " + String(spiffs_config->alarmHour));
        #endif

        spiffs_config->alarmMinute = json["alarmMinute"];
        #ifdef DEBUG_ON
        debugMsg("Loaded alarmMinute: " + String(spiffs_config->alarmMinute));
        #endif

        spiffs_config->sepMode = json["sepMode"];
        #ifdef DEBUG_ON
        debugMsg("Loaded sepMode: " + String(spiffs_config->sepMode));
        #endif

        spiffs_config->hueOffset = json["hueOffset"];
        #ifdef DEBUG_ON
        debugMsg("Loaded hueOffset: " + String(spiffs_config->hueOffset));
        #endif

        spiffs_config->backlightDimFactor = json["backlightDimFactor"];
        #ifdef DEBUG_ON
        debugMsg("Loaded backlightDimFactor: " + String(spiffs_config->backlightDimFactor));
        #endif

        spiffs_config->testMode = json["testMode"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded testMode: " + String(spiffs_config->testMode));
        #endif

        spiffs_config->wasSetup = json["wasSetup"].as<bool>();
        #ifdef DEBUG_ON
        debugMsg("Loaded wasSetup: " + String(spiffs_config->wasSetup));
        #endif

        spiffs_config->WiFiSSID = json["WiFiSSID"].as<String>();
        #ifdef DEBUG_ON
        debugMsg("Loaded WiFiSSID: " + String(spiffs_config->WiFiSSID));
        #endif

        spiffs_config->WiFiPassword = json["WiFiPassword"].as<String>();
        #ifdef DEBUG_ON
        debugMsg("Loaded WiFiPassword: " + String(spiffs_config->WiFiPassword));
        #endif

        loaded = true;
      }
      else
      {
        #ifdef DEBUG_ON
        debugMsg("failed to load json config");
        #endif
      }
      #ifdef DEBUG_ON
      debugMsg("Closing config file");
      #endif
      configFile.close();
    }
  }

  return loaded;
}

// ************************************************************
// Save config back to the SPIFFS
// ************************************************************
void SpiffsStorage_::saveConfigToSpiffs(spiffs_config_t *spiffs_config)
{
  #ifdef DEBUG_ON
  debugMsg("saving config");
  #endif

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json["ntp_pool"] = spiffs_config->ntpPool;
  json["ntp_update_interval"] = spiffs_config->ntpUpdateInterval;
  json["time_zone_string"] = spiffs_config->tzs;
  json["hourMode"] = spiffs_config->hourMode;
  json["blankLeading"] = spiffs_config->blankLeading;
  json["dateFormat"] = spiffs_config->dateFormat;
  json["dayBlanking"] = spiffs_config->dayBlanking;
  json["fade"] = spiffs_config->fade;
  json["scrollback"] = spiffs_config->scrollback;
  json["fadeSteps"] = spiffs_config->fadeSteps;
  json["scrollSteps"] = spiffs_config->scrollSteps;
  json["suppressACP"] = spiffs_config->suppressACP;
  json["minDim"] = spiffs_config->minDim;
  json["backlightMode"] = spiffs_config->backlightMode;
  json["redCnl"] = spiffs_config->redCnl;
  json["grnCnl"] = spiffs_config->grnCnl;
  json["bluCnl"] = spiffs_config->bluCnl;
  json["blankMode"] = spiffs_config->blankMode;
  json["blankHourStart"] = spiffs_config->blankHourStart;
  json["blankHourEnd"] = spiffs_config->blankHourEnd;
  json["cycleSpeed"] = spiffs_config->cycleSpeed;
  json["mdTimeout"] = spiffs_config->mdTimeout;
  json["useLDR"] = spiffs_config->useLDR;
  json["thresholdBright"] = spiffs_config->thresholdBright;
  json["sensitivityLDR"] = spiffs_config->sensitivityLDR;
  json["sensorSmoothCountLDR"] = spiffs_config->sensorSmoothCountLDR;
  json["slotsMode"] = spiffs_config->slotsMode;
  json["webAuthentication"] = spiffs_config->webAuthentication;
  json["webUsername"] = spiffs_config->webUsername;
  json["webPassword"] = spiffs_config->webPassword;
  json["acpMode"] = spiffs_config->acpMode;
  json["mdBlankMode"] = spiffs_config->mdBlankMode;
  json["alarmMode"] = spiffs_config->alarmMode;
  json["alarmHour"] = spiffs_config->alarmHour;
  json["alarmMinute"] = spiffs_config->alarmMinute;
  json["sepMode"] = spiffs_config->sepMode;
  json["backlightDimFactor"] = spiffs_config->backlightDimFactor;
  json["hueOffset"] = spiffs_config->hueOffset;

  json["testMode"] = spiffs_config->testMode;
  json["wasSetup"] = spiffs_config->wasSetup;

  json["WiFiSSID"] = spiffs_config->WiFiSSID;
  json["WiFiPassword"] = spiffs_config->WiFiPassword;


  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    #ifdef DEBUG_ON
    debugMsg("failed to open config file for writing");
    #endif
    configFile.close();
    return;
  }

  json.printTo(configFile);
  configFile.close();
  #ifdef DEBUG_ON
  debugMsg("Saved config");
  #endif
}

// ************************************************************
// Get the statistics from the SPIFFS
// ************************************************************
bool SpiffsStorage_::getStatsFromSpiffs(spiffs_stats_t *spiffs_stats)
{
  bool loaded = false;
  if (SPIFFS.exists("/stats.json"))
  {
    //file exists, reading and loading
    #ifdef DEBUG_ON
    debugMsg("reading stats file");
    #endif
    File statsFile = SPIFFS.open("/stats.json", "r");
    if (statsFile)
    {
      #ifdef DEBUG_ON
      debugMsg("opened stats file");
      #endif
      size_t size = statsFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      statsFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());

      if (json.success())
      {
        #ifdef DEBUG_ON
        debugMsg("parsed stats json");
        #endif

        spiffs_stats->uptimeMins = json.get<unsigned long>("uptime");
        #ifdef DEBUG_ON
        debugMsg("Loaded uptime: " + String(spiffs_stats->uptimeMins));
        #endif

        spiffs_stats->tubeOnTimeMins = json.get<unsigned long>("tubeontime");
        #ifdef DEBUG_ON
        debugMsg("Loaded tubeontime: " + String(spiffs_stats->tubeOnTimeMins));
        #endif

        loaded = true;
      }
      else
      {
        #ifdef DEBUG_ON
        debugMsg("failed to load json config");
        #endif
      }
      #ifdef DEBUG_ON
      debugMsg("Closing stats file");
      #endif
      statsFile.close();
    }
  }
  return loaded;
}

// ************************************************************
// Save the statistics back to the SPIFFS
// ************************************************************
void SpiffsStorage_::saveStatsToSpiffs(spiffs_stats_t *spiffs_stats)
{
  #ifdef DEBUG_ON
  debugMsg("saving stats");
  #endif

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json.set("uptime", spiffs_stats->uptimeMins);
  json.set("tubeontime", spiffs_stats->tubeOnTimeMins);

  File statsFile = SPIFFS.open("/stats.json", "w");
  if (!statsFile)
  {
    #ifdef DEBUG_ON
    debugMsg("failed to open stats file for writing");
    #endif
    statsFile.close();
    return;
  }

  json.printTo(statsFile);
  statsFile.close();
  #ifdef DEBUG_ON
  debugMsg("Saved stats");
  #endif
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void SpiffsStorage_::debugMsg(String message)
{
  if (_dbcb != NULL && _debug)
  {
    _dbcb("[SPF]: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void SpiffsStorage_::setDebugCallback(DebugCallback dbcb)
{
  _dbcb = dbcb;
  #ifdef DEBUG_ON
  debugMsg("Debugging started, callback set");
  #endif
}

// ************************************************************
// set the update interval
// ************************************************************
void SpiffsStorage_::setDebugOutput(bool newDebug)
{
  _debug = newDebug;
}

SpiffsStorage_ &SpiffsStorage_::getInstance() {
  static SpiffsStorage_ instance;
  return instance;
}

SpiffsStorage_ &spiffsStorage = spiffsStorage.getInstance();