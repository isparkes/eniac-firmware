#include "SpiffsStorage.h"

//**********************************************************************************
//**********************************************************************************
//*                               SPIFFS functions                                 *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Test SPIFFS
// ************************************************************
boolean SPIFFS_CLOCK::testMountSpiffs()
{
  boolean mounted = false;
  if (SPIFFS.begin())
  {
    mounted = true;
    SPIFFS.end();
  }

  return mounted;
}

// ************************************************************
// Retrieve the config from the SPIFFS
// ************************************************************
boolean SPIFFS_CLOCK::getConfigFromSpiffs(spiffs_config_t *spiffs_config)
{
  boolean loaded = false;

  debugMsg("mounted file system config read");
  if (SPIFFS.exists("/config.json"))
  {
    //file exists, reading and loading
    debugMsg("reading config file");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile)
    {
      debugMsg("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      //        json.printTo(Serial);
      //        debugMsg("");

      if (json.success())
      {
        debugMsg("parsed json");

        spiffs_config->ntpPool = json["ntp_pool"].as<String>();
        debugMsg("Loaded NTP pool: " + spiffs_config->ntpPool);

        spiffs_config->ntpUpdateInterval = json["ntp_update_interval"].as<int>();
        debugMsg("Loaded NTP update interval: " + String(spiffs_config->ntpUpdateInterval));

        spiffs_config->tzs = json["time_zone_string"].as<String>();
        debugMsg("Loaded time zone string: " + spiffs_config->tzs);

        spiffs_config->hourMode = json["hourMode"].as<bool>();
        debugMsg("Loaded 12/24H mode: " + String(spiffs_config->hourMode));

        spiffs_config->blankLeading = json["blankLeading"].as<bool>();
        debugMsg("Loaded lead zero blanking: " + String(spiffs_config->blankLeading));

        spiffs_config->dateFormat = json["dateFormat"];
        debugMsg("Loaded date format: " + String(spiffs_config->dateFormat));

        spiffs_config->dayBlanking = json["dayBlanking"];
        debugMsg("Loaded dayBlanking: " + String(spiffs_config->dayBlanking));

        spiffs_config->blankLeading = json["fade"].as<bool>();
        debugMsg("Loaded lead fade: " + String(spiffs_config->fade));

        spiffs_config->fadeSteps = json["fadeSteps"];
        debugMsg("Loaded fadeSteps: " + String(spiffs_config->fadeSteps));

        spiffs_config->scrollback = json["scrollback"].as<bool>();
        debugMsg("Loaded lead scrollback: " + String(spiffs_config->scrollback));

        spiffs_config->scrollSteps = json["scrollSteps"];
        debugMsg("Loaded scrollSteps: " + String(spiffs_config->scrollSteps));

        spiffs_config->thresholdBright = json["thresholdBright"];
        debugMsg("Loaded thresholdBright: " + String(spiffs_config->thresholdBright));

        spiffs_config->sensitivityLDR = json["sensitivityLDR"];
        debugMsg("Loaded sensitivityLDR: " + String(spiffs_config->sensitivityLDR));

        spiffs_config->minDim = json["minDim"];
        debugMsg("Loaded minDim: " + String(spiffs_config->minDim));

        spiffs_config->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        debugMsg("Loaded sensorSmoothCountLDR: " + String(spiffs_config->sensorSmoothCountLDR));

        spiffs_config->backlightMode = json["backlightMode"];
        debugMsg("Loaded backlight mode: " + String(spiffs_config->backlightMode));

        spiffs_config->useBLPulse = json["useBLPulse"].as<bool>();
        debugMsg("Loaded backlight pulse: " + String(spiffs_config->useBLPulse));

        spiffs_config->useBLDim = json["useBLDim"].as<bool>();
        debugMsg("Loaded backlight dim: " + String(spiffs_config->useBLDim));

        spiffs_config->redCnl = json["redCnl"];
        debugMsg("Loaded redCnl: " + String(spiffs_config->redCnl));

        spiffs_config->grnCnl = json["grnCnl"];
        debugMsg("Loaded grnCnl: " + String(spiffs_config->grnCnl));

        spiffs_config->bluCnl = json["bluCnl"];
        debugMsg("Loaded bluCnl: " + String(spiffs_config->bluCnl));

        spiffs_config->blankMode = json["blankMode"];
        debugMsg("Loaded blankMode: " + String(spiffs_config->blankMode));

        spiffs_config->blankHourStart = json["blankHourStart"];
        debugMsg("Loaded blankHourStart: " + String(spiffs_config->blankHourStart));

        spiffs_config->blankHourEnd = json["blankHourEnd"];
        debugMsg("Loaded blankHourEnd: " + String(spiffs_config->blankHourEnd));

        spiffs_config->cycleSpeed = json["cycleSpeed"];
        debugMsg("Loaded cycleSpeed: " + String(spiffs_config->cycleSpeed));

        spiffs_config->pirTimeout = json["pirTimeout"];
        debugMsg("Loaded pirTimeout: " + String(spiffs_config->pirTimeout));

        spiffs_config->useLDR = json["useLDR"];
        debugMsg("Loaded useLDR: " + String(spiffs_config->useLDR));

        spiffs_config->slotsMode = json["slotsMode"];
        debugMsg("Loaded slotsMode: " + String(spiffs_config->slotsMode));

        spiffs_config->usePIRPullup = json["usePIRPullup"];
        debugMsg("Loaded usePIRPullup: " + String(spiffs_config->usePIRPullup));

        spiffs_config->testMode = json["testMode"].as<bool>();
        debugMsg("Loaded testMode: " + String(spiffs_config->testMode));

        spiffs_config->webAuthentication = json["webAuthentication"].as<bool>();
        debugMsg("Loaded webAuthentication: " + String(spiffs_config->webAuthentication));

        spiffs_config->webUsername = json["webUsername"].as<String>();
        debugMsg("Loaded webUsername: " + spiffs_config->webUsername);

        spiffs_config->webPassword = json["webPassword"].as<String>();
        debugMsg("Loaded webPassword: " + spiffs_config->webPassword);

        spiffs_config->antiGhost = json["antiGhost"];
        debugMsg("Loaded antiGhost: " + String(spiffs_config->antiGhost));

        spiffs_config->dpEnable = json["dpEnable"];
        debugMsg("Loaded dpEnable: " + String(spiffs_config->dpEnable));

        spiffs_config->acpMode = json["acpMode"];
        debugMsg("Loaded acpMode: " + String(spiffs_config->acpMode));

        spiffs_config->pirBlankMode = json["pirBlankMode"];
        debugMsg("Loaded pirBlankMode: " + String(spiffs_config->pirBlankMode));

        spiffs_config->alarmMode = json["alarmMode"];
        debugMsg("Loaded alarmMode: " + String(spiffs_config->alarmMode));

        spiffs_config->alarmHour = json["alarmHour"];
        debugMsg("Loaded alarmHour: " + String(spiffs_config->alarmHour));

        spiffs_config->alarmMinute = json["alarmMinute"];
        debugMsg("Loaded alarmMinute: " + String(spiffs_config->alarmMinute));

        spiffs_config->sepMode = json["sepMode"];
        debugMsg("Loaded sepMode: " + String(spiffs_config->sepMode));

        loaded = true;
      }
      else
      {
        debugMsg("failed to load json config");
      }
      debugMsg("Closing config file");
      configFile.close();
    }
  }

  return loaded;
}

// ************************************************************
// Save config back to the SPIFFS
// ************************************************************
void SPIFFS_CLOCK::saveConfigToSpiffs(spiffs_config_t *spiffs_config)
{
  debugMsg("saving config");

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
  json["pirTimeout"] = spiffs_config->pirTimeout;
  json["useLDR"] = spiffs_config->useLDR;
  json["slotsMode"] = spiffs_config->slotsMode;
  json["usePIRPullup"] = spiffs_config->usePIRPullup;
  json["testMode"] = spiffs_config->testMode;
  json["webAuthentication"] = spiffs_config->webAuthentication;
  json["webUsername"] = spiffs_config->webUsername;
  json["webPassword"] = spiffs_config->webPassword;
  json["antiGhost"] = spiffs_config->antiGhost;
  json["dpEnable"] = spiffs_config->dpEnable;
  json["acpMode"] = spiffs_config->acpMode;
  json["pirBlankMode"] = spiffs_config->pirBlankMode;
  json["alarmMode"] = spiffs_config->alarmMode;
  json["alarmHour"] = spiffs_config->alarmHour;
  json["alarmMinute"] = spiffs_config->alarmMinute;
  json["sepMode"] = spiffs_config->sepMode;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    debugMsg("failed to open config file for writing");
    configFile.close();
    return;
  }

  //    json.printTo(Serial);
  //    debugMsg("");

  json.printTo(configFile);
  configFile.close();
  debugMsg("Saved config");
}

JsonObject &SPIFFS_CLOCK::getConfigAsJsonObject(spiffs_config_t *spiffs_config)
{
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
  json["pirTimeout"] = spiffs_config->pirTimeout;
  json["useLDR"] = spiffs_config->useLDR;
  json["slotsMode"] = spiffs_config->slotsMode;
  json["usePIRPullup"] = spiffs_config->usePIRPullup;
  json["testMode"] = spiffs_config->testMode;
  json["webAuthentication"] = spiffs_config->webAuthentication;
  json["webUsername"] = spiffs_config->webUsername;
  json["webPassword"] = spiffs_config->webPassword;
  json["antiGhost"] = spiffs_config->antiGhost;
  json["dpEnable"] = spiffs_config->dpEnable;
  json["acpMode"] = spiffs_config->acpMode;
  json["pirBlankMode"] = spiffs_config->pirBlankMode;
  json["alarmMode"] = spiffs_config->alarmMode;
  json["alarmHour"] = spiffs_config->alarmHour;
  json["alarmMinute"] = spiffs_config->alarmMinute;
  json["sepMode"] = spiffs_config->sepMode;
  return json;
}

// ************************************************************
// Get the statistics from the SPIFFS
// ************************************************************
boolean SPIFFS_CLOCK::getStatsFromSpiffs(spiffs_stats_t *spiffs_stats)
{
  boolean loaded = false;
  if (SPIFFS.exists("/stats.json"))
  {
    //file exists, reading and loading
    debugMsg("reading stats file");
    File statsFile = SPIFFS.open("/stats.json", "r");
    if (statsFile)
    {
      debugMsg("opened stats file");
      size_t size = statsFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      statsFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      // json.printTo(Serial);
      // debugMsg("");

      if (json.success())
      {
        debugMsg("parsed stats json");

        spiffs_stats->uptimeMins = json.get<unsigned long>("uptime");
        debugMsg("Loaded uptime: " + String(spiffs_stats->uptimeMins));

        spiffs_stats->tubeOnTimeMins = json.get<unsigned long>("tubeontime");
        debugMsg("Loaded tubeontime: " + String(spiffs_stats->tubeOnTimeMins));

        loaded = true;
      }
      else
      {
        debugMsg("failed to load json config");
      }
      debugMsg("Closing stats file");
      statsFile.close();
    }
  }
  return loaded;
}

// ************************************************************
// Save the statistics back to the SPIFFS
// ************************************************************
void SPIFFS_CLOCK::saveStatsToSpiffs(spiffs_stats_t *spiffs_stats)
{
  debugMsg("saving stats");

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json.set("uptime", spiffs_stats->uptimeMins);
  json.set("tubeontime", spiffs_stats->tubeOnTimeMins);

  File statsFile = SPIFFS.open("/stats.json", "w");
  if (!statsFile)
  {
    debugMsg("failed to open stats file for writing");
    statsFile.close();
    return;
  }

  // json.printTo(Serial);
  // debugMsg("\n");

  json.printTo(statsFile);
  statsFile.close();
  debugMsg("Saved stats");
  //end save
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void SPIFFS_CLOCK::debugMsg(String message)
{
  if (_dbcb != NULL && _debug)
  {
    _dbcb("SPIFFS: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void SPIFFS_CLOCK::setDebugCallback(DebugCallback dbcb)
{
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// ************************************************************
// set the update interval
// ************************************************************
void SPIFFS_CLOCK::setDebugOutput(bool newDebug)
{
  _debug = newDebug;
}