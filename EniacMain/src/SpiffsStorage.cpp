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
bool SpiffsStorage_::getConfigFromSpiffs()
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
        cc->ntpPool = json["ntp_pool"].as<String>();
        debugMsgSpf("Loaded NTP pool: " + cc->ntpPool)
        cc->ntpUpdateInterval = json["ntp_update_interval"].as<int>();
        debugMsgSpf("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));

        cc->tzs = json["time_zone_string"].as<String>();
        debugMsgSpf("Loaded time zone string: " + cc->tzs);

        cc->hourMode = json["hourMode"].as<bool>();
        debugMsgSpf("Loaded 12/24H mode: " + String(cc->hourMode));

        cc->blankLeading = json["blankLeading"].as<bool>();
        debugMsgSpf("Loaded lead zero blanking: " + String(cc->blankLeading));

        cc->dateFormat = json["dateFormat"];
        debugMsgSpf("Loaded date format: " + String(cc->dateFormat));

        cc->dayBlanking = json["dayBlanking"];
        debugMsgSpf("Loaded dayBlanking: " + String(cc->dayBlanking));

        cc->fade = json["fade"].as<bool>();
        debugMsgSpf("Loaded fade: " + String(cc->fade));

        cc->fadeSteps = json["fadeSteps"];
        debugMsgSpf("Loaded fadeSteps: " + String(cc->fadeSteps));

        cc->scrollback = json["scrollback"].as<bool>();
        debugMsgSpf("Loaded scrollback: " + String(cc->scrollback));

        cc->scrollSteps = json["scrollSteps"];
        debugMsgSpf("Loaded scrollSteps: " + String(cc->scrollSteps));

        cc->suppressACP = json["suppressACP"];
        debugMsgSpf("Loaded suppressACP: " + String(cc->suppressACP));

        cc->thresholdBright = json["thresholdBright"];
        debugMsgSpf("Loaded thresholdBright: " + String(cc->thresholdBright));

        cc->sensitivityLDR = json["sensitivityLDR"];
        debugMsgSpf("Loaded sensitivityLDR: " + String(cc->sensitivityLDR));

        cc->minDim = json["minDim"];
        debugMsgSpf("Loaded minDim: " + String(cc->minDim));

        cc->setDim = json["setDim"];
        debugMsgSpf("Loaded setDim: " + String(cc->setDim));

        cc->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        debugMsgSpf("Loaded sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));

        cc->backlightMode = json["backlightMode"];
        debugMsgSpf("Loaded backlight mode: " + String(cc->backlightMode));

        cc->useBLPulse = json["useBLPulse"].as<bool>();
        debugMsgSpf("Loaded backlight pulse: " + String(cc->useBLPulse));

        cc->useBLDim = json["useBLDim"].as<bool>();
        debugMsgSpf("Loaded backlight dim: " + String(cc->useBLDim));

        cc->redCnl = json["redCnl"];
        debugMsgSpf("Loaded redCnl: " + String(cc->redCnl));

        cc->grnCnl = json["grnCnl"];
        debugMsgSpf("Loaded grnCnl: " + String(cc->grnCnl));

        cc->bluCnl = json["bluCnl"];
        debugMsgSpf("Loaded bluCnl: " + String(cc->bluCnl));

        cc->blankMode = json["blankMode"];
        debugMsgSpf("Loaded blankMode: " + String(cc->blankMode));

        cc->blankHourStart = json["blankHourStart"];
        debugMsgSpf("Loaded blankHourStart: " + String(cc->blankHourStart));

        cc->blankHourEnd = json["blankHourEnd"];
        debugMsgSpf("Loaded blankHourEnd: " + String(cc->blankHourEnd));

        cc->cycleSpeed = json["cycleSpeed"];
        debugMsgSpf("Loaded cycleSpeed: " + String(cc->cycleSpeed));

        cc->mdTimeout = json["mdTimeout"];
        debugMsgSpf("Loaded mdTimeout: " + String(cc->mdTimeout));

        cc->useLDR = json["useLDR"];
        debugMsgSpf("Loaded useLDR: " + String(cc->useLDR));

        cc->thresholdBright = json["thresholdBright"];
        debugMsgSpf("Loaded thresholdBright: " + String(cc->thresholdBright));

        cc->slotsMode = json["slotsMode"];
        debugMsgSpf("Loaded slotsMode: " + String(cc->slotsMode));

        cc->webAuthentication = json["webAuthentication"].as<bool>();
        debugMsgSpf("Loaded webAuthentication: " + String(cc->webAuthentication));

        cc->webUsername = json["webUsername"].as<String>();
        debugMsgSpf("Loaded webUsername: " + cc->webUsername);

        cc->webPassword = json["webPassword"].as<String>();
        debugMsgSpf("Loaded webPassword: " + cc->webPassword);

        cc->acpMode = json["acpMode"];
        debugMsgSpf("Loaded acpMode: " + String(cc->acpMode));

        cc->mdBlankMode = json["mdBlankMode"];
        debugMsgSpf("Loaded mdBlankMode: " + String(cc->mdBlankMode));

        cc->alarmMode = json["alarmMode"];
        debugMsgSpf("Loaded alarmMode: " + String(cc->alarmMode));

        cc->alarmHour = json["alarmHour"];
        debugMsgSpf("Loaded alarmHour: " + String(cc->alarmHour));

        cc->alarmMinute = json["alarmMinute"];
        debugMsgSpf("Loaded alarmMinute: " + String(cc->alarmMinute));

        cc->sepMode = json["sepMode"];
        debugMsgSpf("Loaded sepMode: " + String(cc->sepMode));

        cc->hueOffset = json["hueOffset"];
        debugMsgSpf("Loaded hueOffset: " + String(cc->hueOffset));

        cc->towerHueOffset = json["towerHueOffset"];
        debugMsgSpf("Loaded towerHueOffset: " + String(cc->towerHueOffset));

        cc->backlightDimFactor = json["backlightDimFactor"];
        debugMsgSpf("Loaded backlightDimFactor: " + String(cc->backlightDimFactor));

        cc->testMode = json["testMode"].as<bool>();
        debugMsgSpf("Loaded testMode: " + String(cc->testMode));

        cc->wasSetup = json["wasSetup"].as<bool>();
        debugMsgSpf("Loaded wasSetup: " + String(cc->wasSetup));

        cc->WiFiSSID = json["WiFiSSID"].as<String>();
        debugMsgSpf("Loaded WiFiSSID: " + String(cc->WiFiSSID));

        cc->WiFiPassword = json["WiFiPassword"].as<String>();
        debugMsgSpf("Loaded WiFiPassword: " + String(cc->WiFiPassword));

        cc->WifiOnAtStart = json["WifiOnAtStart"].as<bool>();
        debugMsgSpf("Loaded WifiOnAtStart: " + String(cc->WifiOnAtStart));

        cc->blinkenLightsMode = json["blinkenLightsMode"];
        debugMsgSpf("Loaded blinkenLightsMode: " + String(cc->blinkenLightsMode));

        cc->slaveMode = json["slaveMode"];
        debugMsgSpf("Loaded slaveMode: " + String(cc->slaveMode));

        cc->outputOnTime = json["outputOnTime"];
        debugMsgSpf("Loaded outputOnTime: " + String(cc->outputOnTime));

        cc->backlightGradient = json["backlightGradient"];
        debugMsgSpf("Loaded backlightGradient: " + String(cc->backlightGradient));

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
void SpiffsStorage_::saveConfigToSpiffs()
{
  debugMsgSpf("saving config");

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json["ntp_pool"] = cc->ntpPool;
  json["ntp_update_interval"] = cc->ntpUpdateInterval;
  json["time_zone_string"] = cc->tzs;
  json["hourMode"] = cc->hourMode;
  json["blankLeading"] = cc->blankLeading;
  json["dateFormat"] = cc->dateFormat;
  json["dayBlanking"] = cc->dayBlanking;
  json["fade"] = cc->fade;
  json["scrollback"] = cc->scrollback;
  json["fadeSteps"] = cc->fadeSteps;
  json["scrollSteps"] = cc->scrollSteps;
  json["suppressACP"] = cc->suppressACP;
  json["minDim"] = cc->minDim;
  json["setDim"] = cc->setDim;
  json["backlightMode"] = cc->backlightMode;
  json["useBLDim"] = cc->useBLDim;
  json["useBLPulse"] = cc->useBLPulse;
  json["redCnl"] = cc->redCnl;
  json["grnCnl"] = cc->grnCnl;
  json["bluCnl"] = cc->bluCnl;
  json["blankMode"] = cc->blankMode;
  json["blankHourStart"] = cc->blankHourStart;
  json["blankHourEnd"] = cc->blankHourEnd;
  json["cycleSpeed"] = cc->cycleSpeed;
  json["mdTimeout"] = cc->mdTimeout;
  json["useLDR"] = cc->useLDR;
  json["thresholdBright"] = cc->thresholdBright;
  json["sensitivityLDR"] = cc->sensitivityLDR;
  json["sensorSmoothCountLDR"] = cc->sensorSmoothCountLDR;
  json["slotsMode"] = cc->slotsMode;
  json["webAuthentication"] = cc->webAuthentication;
  json["webUsername"] = cc->webUsername;
  json["webPassword"] = cc->webPassword;
  json["acpMode"] = cc->acpMode;
  json["mdBlankMode"] = cc->mdBlankMode;
  json["alarmMode"] = cc->alarmMode;
  json["alarmHour"] = cc->alarmHour;
  json["alarmMinute"] = cc->alarmMinute;
  json["sepMode"] = cc->sepMode;
  json["backlightDimFactor"] = cc->backlightDimFactor;
  json["hueOffset"] = cc->hueOffset;
  json["towerHueOffset"] = cc->towerHueOffset;
  json["testMode"] = cc->testMode;
  json["wasSetup"] = cc->wasSetup;
  json["WiFiSSID"] = cc->WiFiSSID;
  json["WiFiPassword"] = cc->WiFiPassword;
  json["WifiOnAtStart"] = cc->WifiOnAtStart;
  json["blinkenLightsMode"] = cc->blinkenLightsMode;
  json["slaveMode"] = cc->slaveMode;
  json["outputOnTime"] = cc->outputOnTime;
  json["backlightGradient"] = cc->backlightGradient;
  
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
bool SpiffsStorage_::getStatsFromSpiffs()
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

        cs->uptimeMins = json.get<unsigned long>("uptime");
        debugMsgSpf("Loaded uptime: " + String(cs->uptimeMins));

        cs->tubeOnTimeMins = json.get<unsigned long>("tubeontime");
        debugMsgSpf("Loaded tubeontime: " + String(cs->tubeOnTimeMins));

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
void SpiffsStorage_::saveStatsToSpiffs()
{
  debugMsgSpf("saving stats");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json.set("uptime", cs->uptimeMins);
  json.set("tubeontime", cs->tubeOnTimeMins);
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