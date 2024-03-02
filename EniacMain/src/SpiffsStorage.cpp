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
  debugMsgSpfX("mounted file system config read");
  if (SPIFFS.exists("/config/config.json"))
  {
    // file exists, reading and loading
    debugMsgSpf("Reading config file");
    File configFile = SPIFFS.open("/config/config.json", "r");
    if (configFile)
    {
      debugMsgSpfX("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      // // Dump the raw JSON
      // if (_debug) json.printTo(Serial);
      //   debugMsgSpfX("\n");
      if (json.success())
      {
        debugMsgSpfX("parsed config json");
        cc->ntpPool = json["ntp_pool"].as<String>();
        debugMsgSpfX("Loaded NTP pool: " + cc->ntpPool);
        cc->ntpUpdateInterval = json["ntp_update_interval"].as<int>();
        debugMsgSpfX("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));

        cc->tzs = json["time_zone_string"].as<String>();
        debugMsgSpfX("Loaded time zone string: " + cc->tzs);

        cc->hourMode = json["hourMode"].as<bool>();
        debugMsgSpfX("Loaded 12/24H mode: " + String(cc->hourMode));

        cc->blankLeading = json["blankLeading"].as<bool>();
        debugMsgSpfX("Loaded lead zero blanking: " + String(cc->blankLeading));

        cc->dateFormat = json["dateFormat"];
        debugMsgSpfX("Loaded date format: " + String(cc->dateFormat));

        cc->dayBlanking = json["dayBlanking"];
        debugMsgSpfX("Loaded dayBlanking: " + String(cc->dayBlanking));

        cc->fade = json["fade"].as<bool>();
        debugMsgSpfX("Loaded fade: " + String(cc->fade));

        cc->fadeSteps = json["fadeSteps"];
        debugMsgSpfX("Loaded fadeSteps: " + String(cc->fadeSteps));

        cc->scrollback = json["scrollback"].as<bool>();
        debugMsgSpfX("Loaded scrollback: " + String(cc->scrollback));

        cc->scrollSteps = json["scrollSteps"];
        debugMsgSpfX("Loaded scrollSteps: " + String(cc->scrollSteps));

        cc->suppressACP = json["suppressACP"];
        debugMsgSpfX("Loaded suppressACP: " + String(cc->suppressACP));

        cc->thresholdBright = json["thresholdBright"];
        debugMsgSpfX("Loaded thresholdBright: " + String(cc->thresholdBright));

        cc->sensitivityLDR = json["sensitivityLDR"];
        debugMsgSpfX("Loaded sensitivityLDR: " + String(cc->sensitivityLDR));

        cc->minDim = json["minDim"];
        debugMsgSpfX("Loaded minDim: " + String(cc->minDim));

        cc->setDim = json["setDim"];
        debugMsgSpfX("Loaded setDim: " + String(cc->setDim));

        cc->sensorSmoothCountLDR = json["sensorSmoothCountLDR"];
        debugMsgSpfX("Loaded sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));

        cc->backlightMode = json["backlightMode"];
        debugMsgSpfX("Loaded backlight mode: " + String(cc->backlightMode));

        cc->useBLPulse = json["useBLPulse"].as<bool>();
        debugMsgSpfX("Loaded backlight pulse: " + String(cc->useBLPulse));

        cc->useBLDim = json["useBLDim"].as<bool>();
        debugMsgSpfX("Loaded backlight dim: " + String(cc->useBLDim));

        cc->redCnl = json["redCnl"];
        debugMsgSpfX("Loaded redCnl: " + String(cc->redCnl));

        cc->grnCnl = json["grnCnl"];
        debugMsgSpfX("Loaded grnCnl: " + String(cc->grnCnl));

        cc->bluCnl = json["bluCnl"];
        debugMsgSpfX("Loaded bluCnl: " + String(cc->bluCnl));

        cc->blankMode = json["blankMode"];
        debugMsgSpfX("Loaded blankMode: " + String(cc->blankMode));

        cc->blankHourStart = json["blankHourStart"];
        debugMsgSpfX("Loaded blankHourStart: " + String(cc->blankHourStart));

        cc->blankHourEnd = json["blankHourEnd"];
        debugMsgSpfX("Loaded blankHourEnd: " + String(cc->blankHourEnd));

        cc->cycleSpeed = json["cycleSpeed"];
        debugMsgSpfX("Loaded cycleSpeed: " + String(cc->cycleSpeed));

        cc->mdTimeout = json["mdTimeout"];
        debugMsgSpfX("Loaded mdTimeout: " + String(cc->mdTimeout));

        cc->useLDR = json["useLDR"];
        debugMsgSpfX("Loaded useLDR: " + String(cc->useLDR));

        cc->thresholdBright = json["thresholdBright"];
        debugMsgSpfX("Loaded thresholdBright: " + String(cc->thresholdBright));

        cc->slotsMode = json["slotsMode"];
        debugMsgSpfX("Loaded slotsMode: " + String(cc->slotsMode));

        cc->webAuthentication = json["webAuthentication"].as<bool>();
        debugMsgSpfX("Loaded webAuthentication: " + String(cc->webAuthentication));

        cc->webUsername = json["webUsername"].as<String>();
        debugMsgSpfX("Loaded webUsername: " + cc->webUsername);

        cc->webPassword = json["webPassword"].as<String>();
        debugMsgSpfX("Loaded webPassword: " + cc->webPassword);

        cc->acpMode = json["acpMode"];
        debugMsgSpfX("Loaded acpMode: " + String(cc->acpMode));

        cc->mdBlankMode = json["mdBlankMode"];
        debugMsgSpfX("Loaded mdBlankMode: " + String(cc->mdBlankMode));

        cc->alarmMode = json["alarmMode"];
        debugMsgSpfX("Loaded alarmMode: " + String(cc->alarmMode));

        cc->alarmHour = json["alarmHour"];
        debugMsgSpfX("Loaded alarmHour: " + String(cc->alarmHour));

        cc->alarmMinute = json["alarmMinute"];
        debugMsgSpfX("Loaded alarmMinute: " + String(cc->alarmMinute));

        cc->sepMode = json["sepMode"];
        debugMsgSpfX("Loaded sepMode: " + String(cc->sepMode));

        cc->hueOffset = json["hueOffset"];
        debugMsgSpfX("Loaded hueOffset: " + String(cc->hueOffset));

        cc->towerHueOffset = json["towerHueOffset"];
        debugMsgSpfX("Loaded towerHueOffset: " + String(cc->towerHueOffset));

        cc->backlightDimFactor = json["backlightDimFactor"];
        debugMsgSpfX("Loaded backlightDimFactor: " + String(cc->backlightDimFactor));

        cc->testMode = json["testMode"].as<bool>();
        debugMsgSpfX("Loaded testMode: " + String(cc->testMode));

        cc->wasSetup = json["wasSetup"].as<bool>();
        debugMsgSpfX("Loaded wasSetup: " + String(cc->wasSetup));

        cc->WiFiSSID = json["WiFiSSID"].as<String>();
        debugMsgSpfX("Loaded WiFiSSID: " + String(cc->WiFiSSID));

        cc->WiFiPassword = json["WiFiPassword"].as<String>();
        debugMsgSpfX("Loaded WiFiPassword: " + String(cc->WiFiPassword));

        cc->WifiOnAtStart = json["WifiOnAtStart"].as<bool>();
        debugMsgSpfX("Loaded WifiOnAtStart: " + String(cc->WifiOnAtStart));

        cc->blinkenLightsMode = json["blinkenLightsMode"];
        debugMsgSpfX("Loaded blinkenLightsMode: " + String(cc->blinkenLightsMode));

        cc->slaveMode = json["slaveMode"];
        debugMsgSpfX("Loaded slaveMode: " + String(cc->slaveMode));

        cc->outputOnTime = json["outputOnTime"];
        debugMsgSpfX("Loaded outputOnTime: " + String(cc->outputOnTime));

        cc->backlightGradient = json["backlightGradient"];
        debugMsgSpfX("Loaded backlightGradient: " + String(cc->backlightGradient));

        cc->sw1Mode = json["sw1Mode"];
        debugMsgSpfX("Loaded sw1Mode: " + String(cc->sw1Mode));

        cc->sw2Mode = json["sw2Mode"];
        debugMsgSpfX("Loaded sw2Mode: " + String(cc->sw2Mode));

        cc->pMode = json["pMode"];
        debugMsgSpfX("Loaded pmode: " + String(cc->pMode));

        cc->sMode = json["sMode"];
        debugMsgSpfX("Loaded smode: " + String(cc->sMode));

        #ifdef COUNTDOWN
        cc->countdownTarget = json["countdownTarget"].as<String>();
        debugMsgSpfX("Loaded countdownTarget: " + String(cc->countdownTarget));
        #endif

        loaded = true;
      }
      else
      {
        debugMsgSpf("failed to load json config");
      }
      debugMsgSpfX("Closing config file");

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
  debugMsgSpf("Saving config");

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
  json["sw1Mode"] = cc->sw1Mode;
  json["sw2Mode"] = cc->sw2Mode;
  json["pMode"] = cc->pMode;
  json["sMode"] = cc->sMode;
  #ifdef COUNTDOWN
  json["countdownTarget"] = cc->countdownTarget;
  #endif
  
  File configFile = SPIFFS.open("/config/config.json", "w");
  if (!configFile)
  {
    debugMsgSpf("Failed to open config file for writing");

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
    debugMsgSpf("Reading stats file");

    File statsFile = SPIFFS.open("/config/stats.json", "r");
    if (statsFile)
    {
      debugMsgSpfX("opened stats file");

      size_t size = statsFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      statsFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      if (json.success())
      {
        debugMsgSpfX("parsed stats json");

        cs->uptimeMins = json.get<unsigned long>("uptime");
        debugMsgSpfX("Loaded uptime: " + String(cs->uptimeMins));

        cs->tubeOnTimeMins = json.get<unsigned long>("tubeontime");
        debugMsgSpfX("Loaded tubeontime: " + String(cs->tubeOnTimeMins));

        loaded = true;
      }
      else
      {
        debugMsgSpf("Failed to load json config");
      }
      debugMsgSpfX("Closing stats file");

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
  debugMsgSpf("Saving stats");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json.set("uptime", cs->uptimeMins);
  json.set("tubeontime", cs->tubeOnTimeMins);
  File statsFile = SPIFFS.open("/config/stats.json", "w");
  if (!statsFile)
  {
    debugMsgSpf("Failed to open stats file for writing");
    statsFile.close();
    return;
  }
  json.printTo(statsFile);
  statsFile.close();
  debugMsgSpf("Saved stats");
}

// ************************************************************
// Get the zones object from SPIFFS
// ************************************************************
void SpiffsStorage_::getZoneInfoFromSpiffs() {
  if (SPIFFS.exists("/config/zones.json"))
  {
    // file exists, reading and loading
    debugMsgSpfX("Reading zones file");

    File zonesFile = SPIFFS.open("/config/zones.json", "r");
    if (zonesFile) {
      debugMsgSpfX("Opened zones file");

      size_t size = zonesFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      zonesFile.readBytes(buf.get(), size);
      JsonObject &zonesJson = _jsonBuffer.parseObject(buf.get());

      #ifdef SPF_EXTENDED_DEBUG
      debugMsgSpfX("DUMP cacheZonesFromSpiffs");
      zonesJson.printTo(Serial);
      debugMsgSpfX("\n");
      #endif

      if (zonesJson.success())
      {
        debugMsgSpfX("parsed zones json");

        _cachedZonesObj = &zonesJson;
      } else {
        debugMsgSpf("Failed to load json zones");
      }
      debugMsgSpfX("Closing zones file");

      zonesFile.close();
    }
  }
}

// ************************************************************
// Get the zones object from SPIFFS and return as a string
// Used for the web config page
// ************************************************************
String SpiffsStorage_::getZoneConfigSpiffs() {
  debugMsgSpfX("getZoneConfigSpiffs");
  String result = "";
  if (SPIFFS.exists("/config/zones.json")) {
    // file exists, reading and loading
    debugMsgSpfX("Reading zones file");

    File zonesFile = SPIFFS.open("/config/zones.json", "r");
    if (zonesFile) {
      debugMsgSpfX("Opened zones file");
      result = zonesFile.readString();

      debugMsgSpfX("Closing zones file");

      zonesFile.close();
    }
  }

  return result;
}

// ************************************************************
// Get the number of Zone Areas
// ************************************************************
int SpiffsStorage_::getZoneAreaCountFromSpiffs() {
  debugMsgSpfX("getZoneAreaCountFromSpiffs");
  getZoneInfoFromSpiffs();
  int count = 0;
  for (JsonPair keyValue : *_cachedZonesObj) {
    String key = String(keyValue.key);
    count++;
  }
  debugMsgSpfX("getZoneAreaCountFromSpiffs count: " + String(count));
  return count;
}

// ************************************************************
// Get the given Zone Area
// ************************************************************
String SpiffsStorage_::getZoneAreaFromSpiffs(int index) {
  debugMsgSpfX("getZoneAreaFromSpiffs idx: " + String(index));
  getZoneInfoFromSpiffs();
  int count = 0;
  String result = "";
  for (JsonPair keyValue : *_cachedZonesObj) {
    String key = String(keyValue.key);
    if (count == index) {
      result = key;
    }
    count++;
  }
  debugMsgSpfX("getZoneAreaFromSpiffs result: " + result);
  return result;
}

// ************************************************************
// Get the number of Zone Locations in the Area
// ************************************************************
int SpiffsStorage_::getZoneLocationCountFromSpiffs(String location) {
  debugMsgSpfX("getZoneLocationCountFromSpiffs for location: " + location);
  getZoneInfoFromSpiffs();
  int count = 0;

  for (JsonPair keyValue : *_cachedZonesObj) {
    String key = String(keyValue.key);

    if (key == location) {
      // String value = String(keyValue.value.as<char*>());
      // debugMsgSpfX("Found location object: " + key + " with value " + value);
      JsonObject &locationObj = keyValue.value.as<JsonObject>();

      #ifdef SPF_EXTENDED_DEBUG
      debugMsgSpfX("DUMP getZoneLocationCountFromSpiffs");
      locationObj.printTo(Serial);
      debugMsgSpfX("\n");
      #endif

      for (JsonPair keyValueLocation : locationObj) {
        String key = String(keyValueLocation.key);
        count++;
      }
    }
  }
  debugMsgSpfX("getZoneLocationCountFromSpiffs count: " + String(count));
  return count;
}

// ************************************************************
// Get a given Zone Locations in the Area
// ************************************************************
String SpiffsStorage_::getZoneLocationFromSpiffs(String location, int index) {
  debugMsgSpfX("getZoneLocationFromSpiffs for location: " + location + " and index: " + String(index));
  getZoneInfoFromSpiffs();
  int count = 0;
  String result = "";

  for (JsonPair keyValue : *_cachedZonesObj) {
    String key = String(keyValue.key);

    if (key == location) {
      // String value = String(keyValue.value.as<char*>());
      // debugMsgSpfX("Found location object: " + key + " with value " + value);
      JsonObject &locationObj = keyValue.value.as<JsonObject>();

      #ifdef SPF_EXTENDED_DEBUG
      debugMsgSpfX("DUMP getZoneLocationFromSpiffs");
      locationObj.printTo(Serial);
      debugMsgSpfX("\n");
      #endif

      for (JsonPair keyValueLocation : locationObj) {
        String keyLocation = String(keyValueLocation.key);
        if (count == index) {
          result = keyLocation;
        }
        count++;
      }
    }
  }
  debugMsgSpfX("getZoneLocationFromSpiffs result: " + result);
  return result;
}

// ************************************************************
// Get the TZ of a given Zone Locations in the Area
// ************************************************************
String SpiffsStorage_::getLocationTZFromSpiffs(String location, int index) {
  debugMsgSpfX("getLocationTZFromSpiffs for location: " + location + " and index: " + String(index));
  getZoneInfoFromSpiffs();
  int count = 0;
  String result = "";

  for (JsonPair keyValue : *_cachedZonesObj) {
    String key = String(keyValue.key);

    if (key == location) {
      JsonObject &locationObj = keyValue.value.as<JsonObject>();

      #ifdef SPF_EXTENDED_DEBUG
      debugMsgSpfX("DUMP getLocationTZFromSpiffs");
      locationObj.printTo(Serial);
      debugMsgSpfX("\n");
      #endif

      for (JsonPair keyValueLocation : locationObj) {
        String keyLocation = String(keyValueLocation.value.as<char*>());
        if (count == index) {
          result = keyLocation;
        }
        count++;
      }
    }
  }
  debugMsgSpfX("getLocationTZFromSpiffs result: " + result);
  return result;
}


// ************************************************************
// Internal plumbing
// ************************************************************

SpiffsStorage_ &SpiffsStorage_::getInstance() {
  static SpiffsStorage_ instance;
  return instance;
}

SpiffsStorage_ &spiffsStorage = spiffsStorage.getInstance();