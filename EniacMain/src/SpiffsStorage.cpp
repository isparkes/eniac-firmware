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
  debugMsgSpf("mounted file system config read");
  if (SPIFFS.exists("/config/config.json"))
  {
    // file exists, reading and loading
    debugMsgSpf("reading config file");
    File configFile = SPIFFS.open("/config/config.json", "r");
    if (configFile)
    {
      debugMsgSpf("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      // // Dump the raw JSON
      // if (_debug) json.printTo(Serial);
      //   debugMsgSpf("\n");
      if (json.success())
      {
        debugMsgSpf("parsed json");
        spiffs_config->ntpPool = json["ntp_pool"].as<String>();
        debugMsgSpf("Loaded NTP pool: " + spiffs_config->ntpPool);
        spiffs_config->ntpUpdateInterval = json["ntp_update_interval"].as<int>();
        debugMsgSpf("Loaded NTP update interval: " + String(spiffs_config->ntpUpdateInterval));
        spiffs_config->tzs = json["time_zone_string"].as<String>();
        debugMsgSpf("Loaded time zone string: " + spiffs_config->tzs);
        spiffs_config->hourMode = json["hourMode"].as<bool>();
        debugMsgSpf("Loaded 12/24H mode: " + String(spiffs_config->hourMode));
        spiffs_config->blankLeading = json["blankLeading"].as<bool>();
        debugMsgSpf("Loaded lead zero blanking: " + String(spiffs_config->blankLeading));
        spiffs_config->dateFormat = json["dateFormat"];
        debugMsgSpf("Loaded date format: " + String(spiffs_config->dateFormat));
        spiffs_config->dayBlanking = json["dayBlanking"];
        debugMsgSpf("Loaded dayBlanking: " + String(spiffs_config->dayBlanking));
        spiffs_config->fade = json["fade"].as<bool>();
        debugMsgSpf("Loaded fade: " + String(spiffs_config->fade));
        spiffs_config->fadeSteps = json["fadeSteps"];
        debugMsgSpf("Loaded fadeSteps: " + String(spiffs_config->fadeSteps));
        spiffs_config->scrollback = json["scrollback"].as<bool>();
        debugMsgSpf("Loaded scrollback: " + String(spiffs_config->scrollback));
        spiffs_config->scrollSteps = json["scrollSteps"];
        debugMsgSpf("Loaded scrollSteps: " + String(spiffs_config->scrollSteps));
        spiffs_config->thresholdBright = json["thresholdBright"];
        debugMsgSpf("Loaded thresholdBright: " + String(spiffs_config->thresholdBright));
        spiffs_config->sensitivityLDR = json["sensitivityLDR"];
        debugMsgSpf("Loaded sensitivityLDR: " + String(spiffs_config->sensitivityLDR));
        spiffs_config->minDim = json["minDim"];
        debugMsgSpf("Loaded minDim: " + String(spiffs_config->minDim));
        spiffs_config->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        debugMsgSpf("Loaded sensorSmoothCountLDR: " + String(spiffs_config->sensorSmoothCountLDR));
        spiffs_config->backlightMode = json["backlightMode"];
        debugMsgSpf("Loaded backlight mode: " + String(spiffs_config->backlightMode));
        spiffs_config->useBLPulse = json["useBLPulse"].as<bool>();
        debugMsgSpf("Loaded backlight pulse: " + String(spiffs_config->useBLPulse));
        spiffs_config->useBLDim = json["useBLDim"].as<bool>();
        debugMsgSpf("Loaded backlight dim: " + String(spiffs_config->useBLDim));
        spiffs_config->redCnl = json["redCnl"];
        debugMsgSpf("Loaded redCnl: " + String(spiffs_config->redCnl));
        spiffs_config->grnCnl = json["grnCnl"];
        debugMsgSpf("Loaded grnCnl: " + String(spiffs_config->grnCnl));
        spiffs_config->bluCnl = json["bluCnl"];
        debugMsgSpf("Loaded bluCnl: " + String(spiffs_config->bluCnl));
        spiffs_config->blankMode = json["blankMode"];
        debugMsgSpf("Loaded blankMode: " + String(spiffs_config->blankMode));
        spiffs_config->blankHourStart = json["blankHourStart"];
        debugMsgSpf("Loaded blankHourStart: " + String(spiffs_config->blankHourStart));
        spiffs_config->blankHourEnd = json["blankHourEnd"];
        debugMsgSpf("Loaded blankHourEnd: " + String(spiffs_config->blankHourEnd));
        spiffs_config->cycleSpeed = json["cycleSpeed"];
        debugMsgSpf("Loaded cycleSpeed: " + String(spiffs_config->cycleSpeed));
        spiffs_config->mdTimeout = json["mdTimeout"];
        debugMsgSpf("Loaded mdTimeout: " + String(spiffs_config->mdTimeout));
        spiffs_config->useLDR = json["useLDR"];
        debugMsgSpf("Loaded useLDR: " + String(spiffs_config->useLDR));
        spiffs_config->thresholdBright = json["thresholdBright"];
        debugMsgSpf("Loaded thresholdBright: " + String(spiffs_config->thresholdBright));
        spiffs_config->sensitivityLDR = json["sensitivityLDR"];
        debugMsgSpf("Loaded sensitivityLDR: " + String(spiffs_config->sensitivityLDR));
        spiffs_config->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        debugMsgSpf("Loaded sensorSmoothCountLDR: " + String(spiffs_config->sensorSmoothCountLDR));
        spiffs_config->slotsMode = json["slotsMode"];
        debugMsgSpf("Loaded slotsMode: " + String(spiffs_config->slotsMode));

        spiffs_config->webAuthentication = json["webAuthentication"].as<bool>();
        debugMsgSpf("Loaded webAuthentication: " + String(spiffs_config->webAuthentication));

        spiffs_config->webUsername = json["webUsername"].as<String>();
        debugMsgSpf("Loaded webUsername: " + spiffs_config->webUsername);

        spiffs_config->webPassword = json["webPassword"].as<String>();
        debugMsgSpf("Loaded webPassword: " + spiffs_config->webPassword);

        spiffs_config->acpMode = json["acpMode"];
        debugMsgSpf("Loaded acpMode: " + String(spiffs_config->acpMode));

        spiffs_config->mdBlankMode = json["mdBlankMode"];
        debugMsgSpf("Loaded mdBlankMode: " + String(spiffs_config->mdBlankMode));

        spiffs_config->alarmMode = json["alarmMode"];
        debugMsgSpf("Loaded alarmMode: " + String(spiffs_config->alarmMode));

        spiffs_config->alarmHour = json["alarmHour"];
        debugMsgSpf("Loaded alarmHour: " + String(spiffs_config->alarmHour));

        spiffs_config->alarmMinute = json["alarmMinute"];
        debugMsgSpf("Loaded alarmMinute: " + String(spiffs_config->alarmMinute));

        spiffs_config->sepMode = json["sepMode"];
        debugMsgSpf("Loaded sepMode: " + String(spiffs_config->sepMode));

        spiffs_config->hueOffset = json["hueOffset"];
        debugMsgSpf("Loaded hueOffset: " + String(spiffs_config->hueOffset));

        spiffs_config->backlightDimFactor = json["backlightDimFactor"];
        debugMsgSpf("Loaded backlightDimFactor: " + String(spiffs_config->backlightDimFactor));

        spiffs_config->testMode = json["testMode"].as<bool>();
        debugMsgSpf("Loaded testMode: " + String(spiffs_config->testMode));

        spiffs_config->wasSetup = json["wasSetup"].as<bool>();
        debugMsgSpf("Loaded wasSetup: " + String(spiffs_config->wasSetup));

        spiffs_config->WiFiSSID = json["WiFiSSID"].as<String>();
        debugMsgSpf("Loaded WiFiSSID: " + String(spiffs_config->WiFiSSID));

        spiffs_config->WiFiPassword = json["WiFiPassword"].as<String>();
        debugMsgSpf("Loaded WiFiPassword: " + String(spiffs_config->WiFiPassword));

        spiffs_config->WifiOnAtStart = json["WifiOnAtStart"].as<bool>();
        debugMsgSpf("Loaded WifiOnAtStart: " + String(spiffs_config->WifiOnAtStart));

        spiffs_config->blinkenLightsMode = json["blinkenLightsMode"];
        debugMsgSpf("Loaded blinkenLightsMode: " + String(spiffs_config->blinkenLightsMode));

        spiffs_config->slaveMode = json["slaveMode"];
        debugMsgSpf("Loaded slaveMode: " + String(spiffs_config->slaveMode));

        loaded = true;
      }
      else
      {
        debugMsgSpf("failed to load json config");
      }
      debugMsgSpf("Closing config file");

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
  debugMsgSpf("saving config");

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
  json["WifiOnAtStart"] = spiffs_config->WifiOnAtStart;
  json["blinkenLightsMode"] = spiffs_config->blinkenLightsMode;
  json["slaveMode"] = spiffs_config->slaveMode;
  File configFile = SPIFFS.open("/config/config.json", "w");
  if (!configFile)
  {
    debugMsgSpf("failed to open config file for writing");

    configFile.close();
    return;
  }
  json.printTo(configFile);
  configFile.close();
  debugMsgSpf("Saved config");
}
// ************************************************************
// Get the statistics from the SPIFFS
// ************************************************************
bool SpiffsStorage_::getStatsFromSpiffs(spiffs_stats_t *spiffs_stats)
{
  bool loaded = false;
  if (SPIFFS.exists("/config/stats.json"))
  {
    // file exists, reading and loading
    debugMsgSpf("reading stats file");

    File statsFile = SPIFFS.open("/config/stats.json", "r");
    if (statsFile)
    {
      debugMsgSpf("opened stats file");

      size_t size = statsFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      statsFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      if (json.success())
      {
        debugMsgSpf("parsed stats json");

        spiffs_stats->uptimeMins = json.get<unsigned long>("uptime");
        debugMsgSpf("Loaded uptime: " + String(spiffs_stats->uptimeMins));

        spiffs_stats->tubeOnTimeMins = json.get<unsigned long>("tubeontime");
        debugMsgSpf("Loaded tubeontime: " + String(spiffs_stats->tubeOnTimeMins));

        loaded = true;
      }
      else
      {
        debugMsgSpf("failed to load json config");
      }
      debugMsgSpf("Closing stats file");

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
  debugMsgSpf("saving stats");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json.set("uptime", spiffs_stats->uptimeMins);
  json.set("tubeontime", spiffs_stats->tubeOnTimeMins);
  File statsFile = SPIFFS.open("/config/stats.json", "w");
  if (!statsFile)
  {
    debugMsgSpf("failed to open stats file for writing");
    statsFile.close();
    return;
  }
  json.printTo(statsFile);
  statsFile.close();
  debugMsgSpf("Saved stats");
}
SpiffsStorage_ &SpiffsStorage_::getInstance()
{
  static SpiffsStorage_ instance;
  return instance;
}
SpiffsStorage_ &spiffsStorage = spiffsStorage.getInstance();