#include "utilities.h"

// --------------------------------------------------------------------------------------------------------
// ----------------------------------------  Utility functions  -------------------------------------------
// --------------------------------------------------------------------------------------------------------

// ************************************************************
// Format a time into an output string
// ************************************************************
String timeToReadableStringFromTm(tm timeToFormat) {
  char buf1[20];
  sprintf(buf1, "%04d-%02d-%02d %02d:%02d:%02d",
    timeToFormat.tm_year + 1900,
    timeToFormat.tm_mon + 1,
    timeToFormat.tm_mday,
    timeToFormat.tm_hour,
    timeToFormat.tm_min,
    timeToFormat.tm_sec);
  return String(buf1);
}

// ************************************************************
// Format a duration into an output string
// ************************************************************
String secsToReadableString (long secsValue) {
  long upDays = secsValue / 86400;
  long upHours = (secsValue - (upDays * 86400)) / 3600;
  long upMins = (secsValue - (upDays * 86400) - (upHours * 3600)) / 60;
  secsValue = secsValue - (upDays * 86400) - (upHours * 3600) - (upMins * 60);
  String uptimeString = "";
  if (upDays > 0) {
    uptimeString += upDays; 
    uptimeString += " d ";
  }
  if (upHours > 0) {
    uptimeString += upHours;
    uptimeString += " h "; 
  }
  if (upMins > 0) {
    uptimeString += upMins; 
    uptimeString += " m ";
  }
  if (secsValue > 0) {
    uptimeString += secsValue; 
    uptimeString += " s";
  }
  if (uptimeString == "") {
    uptimeString = "0 s";
  }

  return uptimeString;
}

// ************************************************************
// Calculate the status string for the web interface
// ************************************************************
String getStatusString() {
  String connectionInfo = "";

  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected) {
    connectionInfo += "W";
  } else {
    connectionInfo += "w";
  }
  if (ntpManager.ntpTimeValid()) {
    connectionInfo += "N";
  } else {
    connectionInfo += "n";
  }
  // if (spiffsStorage.testMountSpiffs()) {
  //   connectionInfo += "S";
  // } else {
  //   connectionInfo += "s";
  // }
  // On the ESP32 we always have enough space for OTA
  // 4MB default = 1310720 for program and the same for OTA
  // More info .platformio/packages/framework-arduinoespressif32/tools/partitions/default.csv
  connectionInfo += "U";
  
  if (cc->webAuthentication) {
    connectionInfo += "A";
  } else {
    connectionInfo += "a";
  }

  if (blankingManager.getCurrentBlankingStatus()) {
    connectionInfo += "B";
  } else {
    connectionInfo += "b";
  }

  if (oledTimeout > 0) {
    connectionInfo += "O";
  } else {
    connectionInfo += "o";
  }

#ifdef DEBUG
  if (debugManager.isDebugOn()) { 
    connectionInfo += "D";
  } else {
    connectionInfo += "d";
  }
#else
  connectionInfo += "-";
#endif

  if (gpsManager.getGPSTimeValid()) {
    connectionInfo += "G";
  } else {
    connectionInfo += "g";
  }

  return connectionInfo;
}

// ************************************************************
// Reset settings to factory defaults
// ************************************************************
void resetOptions() {
  cc->ntpPool = NTP_POOL_DEFAULT;
  cc->ntpUpdateInterval = NTP_UPDATE_INTERVAL_DEFAULT;
  cc->tzs = TIME_ZONE_STRING_DEFAULT;

  cc->hourMode = HOUR_MODE_DEFAULT;
  cc->blankLeading = LEAD_BLANK_DEFAULT;
  cc->dateFormat = DATE_FORMAT_DEFAULT;
  cc->dayBlanking = DAY_BLANKING_DEFAULT;
  
  cc->useLDR = USE_LDR_DEFAULT;
  cc->thresholdBright = SENSOR_THRSH_DEFAULT;
  cc->sensorSmoothCountLDR = SENSOR_SMOOTH_READINGS_DEFAULT;
  cc->sensitivityLDR = SENSOR_SENSIT_DEFAULT;
  cc->minDim = MIN_DIM_DEFAULT;
  cc->setDim = MIN_DIM_DEFAULT;
  
  cc->fade = FADE_DEFAULT;
  cc->fadeSteps = FADE_STEPS_DEFAULT;
  cc->scrollback = SCROLLBACK_DEFAULT;
  cc->scrollSteps = SCROLL_STEPS_DEFAULT;
  cc->slotsMode = SLOTS_MODE_DEFAULT;
  cc->acpMode = ACP_MODE_DEFAULT;
  cc->suppressACP = SUPPRESS_ACP_DEFAULT;
  
  cc->useBLDim = true;
  cc->useBLPulse = false;

  // --------------------------------------------------------------------------

  #ifdef FEATURE_BACKLIGHTS
  cc->backlightMode = BACKLIGHT_DEFAULT;
  cc->redCnl = COLOUR_RED_CNL_DEFAULT;
  cc->grnCnl = COLOUR_GRN_CNL_DEFAULT;
  cc->bluCnl = COLOUR_BLU_CNL_DEFAULT;
  cc->cycleSpeed = CYCLE_SPEED_DEFAULT;
  cc->backlightDimFactor = BACKLIGHT_DIM_FACTOR_DEFAULT;
  cc->ledMode = LED_BLINK_DEFAULT;
  cc->hueOffset = HUE_OFFSET_DEFAULT;
  #else
  cc->backlightMode      = 0;
  cc->redCnl             = 0;
  cc->grnCnl             = 0;
  cc->bluCnl             = 0;
  cc->cycleSpeed         = 0;
  cc->backlightDimFactor = 0;
  cc->ledMode            = 0;
  cc->hueOffset          = 0;
  #endif

  // --------------------------------------------------------------------------

  cc->blankMode = BLANK_MODE_DEFAULT;
  cc->blankHourStart = 0;
  cc->blankHourEnd = 7;
  cc->sepMode = SEP_BLINK_DEFAULT;

  cc->mdTimeout = PIR_TIMEOUT_DEFAULT;
  
  // ToDo implement these
  // cc->webAuthentication = getWebAuthentication();
  // cc->webUsername = getWebUserName();
  // cc->webPassword = getWebPassword();
  // setWebAuthentication(WEB_AUTH_DEFAULT);
  // setWebUserName(WEB_USERNAME_DEFAULT);
  // setWebPassword(WEB_PASSWORD_DEFAULT);
  
  cc->testMode = true;
  cc->wasSetup = true;

  cc->WiFiSSID = "";
  cc->WiFiPassword = "";
  cc->WifiOnAtStart = false;
  cc->sw1Mode = SW1_DEFAULT;
  cc->sw2Mode = SW2_DEFAULT;
  cc->pMode = DISPLAY_TIME;
  cc->sMode = DISPLAY_DATE;
  
  #ifdef FEATURE_BLINKENLIGHTS
  cc->blinkenLightsMode = BLNKN_MODE_DEFAULT;
  #else
  cc->blinkenLightsMode = 0;
  #endif
  #ifdef NIXIE_SLAVE
  cc->slaveMode = SLAVE_MODE_DEFAULT;
  #endif

  #ifdef COUNTDOWN
  cc->countdownTarget = "";
  #endif

  spiffsStorage.saveConfigToSpiffs();
  debugMsgUtl("Saved factory config");
}

//**********************************************************************************
//**********************************************************************************
//*                                  NTP Callback                                  *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Callback: When the NTP component tells us there is an update
// go and get it
// ************************************************************
void newTimeUpdateReceived() {
  debugMsgUtl("Got a new NTP time update: " + String(ntpManager.getLastTimeTFromServer()));
  tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_NTP, ntpManager.getLastUpdate(), ntpManager.getLastTimeTFromServer());
}

//**********************************************************************************
//**********************************************************************************
//*                                  Web Handlers                                  *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Debug server args
// ************************************************************
#ifdef DEBUG
void dumpArgs(AsyncWebServerRequest *request) {
  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    String message = "HEADER[" + h->name() + ":" + h->value();
    debugMsgUtl(message);
  }

  if (request->hasArg("body")) {
    debugMsgUtl("Body found arg");
  }
  if (request->hasParam("body")) {
    debugMsgUtl("Body found param");
  }
  int args = request->args();
  for(int i=0;i<args;i++){
    String message = "ARG[" + request->argName(i) + "]: " + request->arg(i); 
    debugMsgUtl(message);
  }  
}
#endif


// ************************************************************
// Main page handler
// ************************************************************
void mainHandler(AsyncWebServerRequest *request) {
	debugMsgUtl("Got Main Handler request");
	request->send(SPIFFS, "/web/index.html");
}

// ************************************************************
// Main CSS handler
// ************************************************************
void cssHandler(AsyncWebServerRequest *request) {
	debugMsgUtl("Got css request");
	request->send(SPIFFS, "/web/style.css");
}

// ************************************************************
// Summary page
// ************************************************************
void getSummaryDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api summary GET request");
  
  signed long absNextUpdate = abs(ntpManager.getNextUpdate());
  String overdueInd = "";

  if (absNextUpdate < 0) {
    overdueInd = " overdue";
    absNextUpdate = -absNextUpdate;
  }

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ip"] = WiFi.localIP().toString();
  root["mac"] = WiFi.macAddress();
  root["ssid"] = WiFi.SSID();
  root["tz"] = tzManager.getTZS();
  root["ntppool"] = ntpManager.getNtpPool();
  String clockUrl = "http://" + String(WiFi.getHostname()) + ".local";
  clockUrl.toLowerCase();
  root["clockurl"] = clockUrl;
  root["timeSource"] = tzManager.getPrimaryTimeSource();
  root["currentntptime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_NTP);
  root["lastntpupdate"] = secsToReadableString(tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_NTP));
  root["nextupdate"] = secsToReadableString(absNextUpdate) + overdueInd;
  if (ntpManager.ntpTimeValid()) {
    root["ntpvalid"] = 1;
  } else {
    root["ntpvalid"] = 0;
  }
  root["displaytime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_INT);

  if (gpsManager.getGPSTimeValid()) {
    root["lastgpstime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_GPS);
    root["lastgpsupdate"] = secsToReadableString(tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_GPS));
    root["gpsvalid"] = 0;
  } else {
    if (gpsManager.getGPSSyncStarted()) {
      root["gpsvalid"] = 1;
    } else {
      root["gpsvalid"] = 2;
    }
  }

  if (rtcManager.getRTCValid()) {
    root["lastrtctime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_RTC);
    root["lastrtcupdate"] = secsToReadableString(tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_GPS));
    root["rtcvalid"] = 1;
  } else {
    root["lastrtctime"] = "RTC not installed";
    root["lastrtcupdate"] = "";
    root["rtcvalid"] = 0;
  }

  float ldrPerc = ldrManager.getLDRValuePct();
  root["ldrvalue"] = String(ldrPerc, 2) + "% (" + String(ldrManager.getLDRValue()) + ")";

  bool pirInstalled = blankingManager.getCurrentPIRInstalled();
  root["mdInstalled"] = pirInstalled;
  if (pirInstalled) {
    root["mdLastSeen"] = secsToReadableString(blankingManager.getBlankAge());
  } else {
    root["mdLastSeen"] = "Motion detector not installed";
  }
  root["blankingReason"] = blankingManager.getBlankingReason();

  root["status"] = getStatusString();
  root["version"] = SOFTWARE_VERSION;

  response->setLength();
  request->send(response);
}

// ************************************************************
// Diags page
// ************************************************************
void getDiagsDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api diagnostics GET request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();

  const char compile_date[] = __DATE__ " " __TIME__;
  // Total ontime for the life of the clock
  root["uptime"] = secsToReadableString(cs->uptimeMins * 60);

  // Total time the tubes have been on for
  root["ontime"] = secsToReadableString(cs->tubeOnTimeMins * 60);

  root["heap"] = ESP.getFreeHeap();
  root["freesketch"] = ESP.getFreeSketchSpace();
  root["sketchsize"] = ESP.getSketchSize();
  root["flashsize"] = ESP.getFlashChipSize();
  root["compiledate"] = String(compile_date);
  root["cpufreq"] = ESP.getCpuFreqMHz();
  root["sdkversion"] = ESP.getSdkVersion();
  root["sketchmd5"] = ESP.getSketchMD5();

  // Time since last reboot
  root["runtime"] = secsToReadableString(nowMillis/1000);
  root["cyclecount"] = ESP.getCycleCount();
  root["minfreepsram"] = ESP.getMinFreePsram();
  root["minfreeheap"] = ESP.getMinFreeHeap();
  root["resetreason"] = String(rtc_get_reset_reason(0)) + "/" + String(rtc_get_reset_reason(1));
  root["lastgpsraw"] = gpsManager.getLastGPSTimeRaw();
  root["utcgpsraw"] = tzManager.getRawUTCTimeFromTimeSource(TIME_SOURCE_GPS);
  root["utcntpraw"] = tzManager.getRawUTCTimeFromTimeSource(TIME_SOURCE_NTP);
  root["utcrtcraw"] = tzManager.getRawUTCTimeFromTimeSource(TIME_SOURCE_RTC);
  root["utcrtcnative"] = rtcManager.getRTCTimeAsTimeT();
  root["utcintraw"] = tzManager.getRawUTCTimeFromTimeSource(TIME_SOURCE_INT);
  root["utcgpsat"] = tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_GPS);
  root["utcntpat"] = tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_NTP);
  root["utcrtcat"] = tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_RTC);
  root["utcintat"] = tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_INT);
  root["utcoffset"] = String(tzManager.getCurrentUTCOffset());
  #if defined NIXIE_SLAVE
  root["slavetrycount"] = String(slaveManagerNixie.getTryCount());
  root["slavefailcount"] = String(slaveManagerNixie.getFailCount());
  #else
  root["slavetrycount"] = "0";
  root["slavefailcount"] = "0";
  #endif
  root["sw1Mode"] = cc->sw1Mode;
  root["sw2Mode"] = cc->sw2Mode;
  #ifdef NORMAL_SWITCHES
  root["sw1val"] = (digitalRead(Switch1Pin) == LOW) ? "1" : "0";
  root["sw2val"] = (digitalRead(Switch2Pin) == LOW) ? "1" : "0";
  #else
  root["sw1val"] = (digitalRead(Switch1Pin) == HIGH) ? "1" : "0";
  root["sw2val"] = (digitalRead(Switch2Pin) == HIGH) ? "1" : "0";
  #endif
  
  #ifdef DIGIT_DIAGNOSTICS
  root["diagsMode"] = cc->diagsMode;
  #endif

  String featureString = "";
  #ifdef DEBUG
  featureString += "DEB ";
  #endif

  #ifdef DIGIT_DIAGNOSTICS
  featureString += "DIAG ";
  #endif

  #ifdef COUNTDOWN
  featureString += "CNT ";
  #endif

  #ifdef OLED_SSD1306
  featureString += "SSD1306 ";
  #endif

  #ifdef OLED_SH1106
  featureString += "SH1106 ";
  #endif

  #ifdef FEATURE_BACKLIGHTS
  featureString += "BKCL ";
  #endif

  #ifdef FEATURE_SEP_LED
  featureString += "SEP ";
  #endif

  #ifdef WS2812B
  featureString += "WS2 ";
  #endif

  #ifdef APA106
  featureString += "APA ";
  #endif
  
  #ifdef REVERSE_BL_OUTPUT
  featureString += "BLR ";
  #endif

  #ifdef REVERSE_UL_OUTPUT
  featureString += "ULR ";
  #endif

  #ifdef FEATURE_BLINKENLIGHTS
  featureString += "BLNK ";
  #endif

  #ifdef NIXIE_SLAVE
  featureString += "NSLV ";
  #endif

  #ifdef DECATRON_SLAVE
  featureString += "DSLV ";
  #endif

  #ifdef COG_CRANK_OUTPUT
  featureString += "COG ";
  #endif

  #ifdef INVERT_SWITCHES
  featureString += "INV ";
  #endif

  root["features"] = featureString;

  response->setLength();
  request->send(response);
}

// ************************************************************
// Diags page POST
// ************************************************************
void postDiagsDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got diags POST request");
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    #ifdef DIGIT_DIAGNOSTICS
    debugMsgUtl("Diags mode before: " + String(cc->diagsMode));
    cc->diagsMode = json["diagsMode"].as<int>();
    debugMsgUtl("Diags mode after: " + String(cc->diagsMode));
    #else
    debugMsgUtl("Diags POST ignored");
    #endif
  }
   
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["diagsMode"] = cc->diagsMode;
  response->setLength();
  request->send(response);
}

// ************************************************************
// Command to save current stats
// ************************************************************
void saveStatsHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got save stats request");

  spiffsStorage.saveStatsToSpiffs();
  
  request->send(200, "text/json", "{\"status\": \"Stats saved\"}");
}

// ************************************************************
// Config page
// ************************************************************
void getConfigDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api config GET request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["hourMode"] = cc->hourMode;
  root["blankLeading"] = cc->blankLeading;
  root["dateFormat"] = cc->dateFormat;
  root["scrollback"] = cc->scrollback;
  root["scrollSteps"] = cc->scrollSteps;
  root["fade"] = cc->fade;
  root["fadeSteps"] = cc->fadeSteps;
  root["slotsMode"] = cc->slotsMode;
  root["acpMode"] = cc->acpMode;
  root["suppressACP"] = cc->suppressACP;

  root["useLDR"] = cc->useLDR;
  root["minDim"] = cc->minDim;
  root["setDim"] = cc->setDim;
  root["thresholdBright"] = cc->thresholdBright;
  root["sensitivityLDR"] = cc->sensitivityLDR;

  root["mdinstalled"] = blankingManager.getCurrentPIRInstalled();
  root["mdTimeout"] = cc->mdTimeout;
  root["mdBlankMode"] = cc->mdBlankMode;
  root["dayBlanking"] = cc->dayBlanking;
  root["blankMode"] = cc->blankMode;
  root["blankHourStart"] = cc->blankHourStart;
  root["blankHourEnd"] = cc->blankHourEnd;
  root["sepMode"] = cc->sepMode;

  root["backlightMode"] = cc->backlightMode;
  root["redCnl"] = cc->redCnl;
  root["grnCnl"] = cc->grnCnl;
  root["bluCnl"] = cc->bluCnl;
  root["useBLDim"] = cc->useBLDim;
  root["useBLPulse"] = cc->useBLPulse;
  root["cycleSpeed"] = cc->cycleSpeed;
  root["backlightDimFactor"] = cc->backlightDimFactor;
  root["hueOffset"] = cc->hueOffset;
  root["towerHueOffset"] = cc->towerHueOffset;
  root["backlightGradient"] = cc->backlightGradient;
  root["blinkenLightsMode"] = cc->blinkenLightsMode;
  #ifdef NIXIE_SLAVE
  root["slaveMode"] = cc->slaveMode;
  #endif
  root["WifiOnAtStart"] = cc->WifiOnAtStart;
  root["sw1Mode"] = cc->sw1Mode;
  root["sw2Mode"] = cc->sw2Mode;
  root["pMode"] = cc->pMode;
  root["sMode"] = cc->sMode;

  #ifdef COG_CRANK_OUTPUT
  root["outputOnTime"] = cc->outputOnTime;
  #endif

  #ifdef COUNTDOWN
  root["countdownTarget"] = cc->countdownTarget;
  #endif

  response->setLength();
  request->send(response);
}

void compareAndUpdateByte(JsonObject& json, const char* key, byte* variable) {
  if (json.containsKey(key)) {
    byte newVal = json[key];
    if (*variable != newVal) {
      debugMsgUtl(String(key) + " old: " + String(*variable));
      *variable = newVal;
      debugMsgUtl(String(key) + " new: " + String(*variable));
    }
  }
}

void compareAndUpdateInt(JsonObject& json, const char* key, int* variable) {
  if (json.containsKey(key)) {
    int newVal = json[key];
    if (*variable != newVal) {
      debugMsgUtl(String(key) + " old: " + String(*variable));
      *variable = newVal;
      debugMsgUtl(String(key) + " new: " + String(*variable));
    }
  }
}

void compareAndUpdateBool(JsonObject& json, const char* key, bool* variable) {
  if (json.containsKey(key)) {
    bool newVal = json[key].as<bool>();
    if (*variable != newVal) {
      debugMsgUtl(String(key) + " old: " + String(*variable));
      *variable = newVal;
      debugMsgUtl(String(key) + " new: " + String(*variable));
    }
  }
}

void compareAndUpdateString(JsonObject& json, const char* key, String* variable) {
  if (json.containsKey(key)) {
    String newVal = json[key];
    if (*variable != newVal) {
      debugMsgUtl(String(key) + " old: " + String(*variable));
      *variable = newVal;
      debugMsgUtl(String(key) + " new: " + String(*variable));
    }
  }
}

bool checkPresence(JsonObject& json, const char* key) {
  return  json.containsKey(key);
}

void postConfigDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api config POST request");

  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {

    // ------------------------------------------------------------

    compareAndUpdateBool(json, "hourMode",     &cc->hourMode);
    compareAndUpdateBool(json, "blankLeading", &cc->blankLeading);
    compareAndUpdateByte(json, "dateFormat",   &cc->dateFormat);
    compareAndUpdateBool(json, "blankLeading", &cc->blankLeading);
    compareAndUpdateBool(json, "scrollback",   &cc->scrollback);
    compareAndUpdateByte(json, "scrollSteps",  &cc->scrollSteps);
    compareAndUpdateBool(json, "fade",         &cc->fade);
    compareAndUpdateByte(json, "fadeSteps",    &cc->fadeSteps);
    compareAndUpdateByte(json, "slotsMode",    &cc->slotsMode);
    compareAndUpdateByte(json, "acpMode",      &cc->acpMode);
    compareAndUpdateBool(json, "suppressACP",  &cc->suppressACP);
    compareAndUpdateBool(json, "WifiOnAtStart",&cc->WifiOnAtStart);
    compareAndUpdateByte(json, "sw1Mode",      &cc->sw1Mode);
    compareAndUpdateByte(json, "sw2Mode",      &cc->sw2Mode);
    compareAndUpdateByte(json, "pMode",        &cc->pMode);
    compareAndUpdateByte(json, "sMode",        &cc->sMode);

    // ------------------------------------------------------------

    compareAndUpdateBool(json, "useLDR",          &cc->useLDR);
    compareAndUpdateInt (json, "minDim",          &cc->minDim);
    compareAndUpdateInt (json, "setDim",          &cc->setDim);
    compareAndUpdateInt (json, "thresholdBright", &cc->thresholdBright);
    compareAndUpdateInt (json, "sensitivityLDR",  &cc->sensitivityLDR);

    // ------------------------------------------------------------

    compareAndUpdateInt (json, "mdTimeout",      &cc->mdTimeout);
    compareAndUpdateByte(json, "mdBlankMode",    &cc->mdBlankMode);
    compareAndUpdateByte(json, "dayBlanking",    &cc->dayBlanking);
    compareAndUpdateByte(json, "blankMode",      &cc->blankMode);
    compareAndUpdateByte(json, "blankHourStart", &cc->blankHourStart);
    compareAndUpdateByte(json, "blankHourEnd",   &cc->blankHourEnd);
    compareAndUpdateByte(json, "sepMode",        &cc->sepMode);

    // ------------------------------------------------------------

    compareAndUpdateByte(json, "backlightMode",      &cc->backlightMode);
    compareAndUpdateByte(json, "redCnl",             &cc->redCnl);
    compareAndUpdateByte(json, "grnCnl",             &cc->grnCnl);
    compareAndUpdateByte(json, "bluCnl",             &cc->bluCnl);
    compareAndUpdateBool(json, "useBLDim",           &cc->useBLDim);
    compareAndUpdateBool(json, "useBLPulse",         &cc->useBLPulse);
    compareAndUpdateByte(json, "cycleSpeed",         &cc->cycleSpeed);
    compareAndUpdateByte(json, "backlightDimFactor", &cc->backlightDimFactor);
    compareAndUpdateInt (json, "hueOffset",          &cc->hueOffset);
    compareAndUpdateInt (json, "towerHueOffset",     &cc->towerHueOffset);
    compareAndUpdateInt (json, "backlightGradient",  &cc->backlightGradient);
    compareAndUpdateByte(json, "blinkenLightsMode",  &cc->blinkenLightsMode);
    compareAndUpdateByte(json, "slaveMode",          &cc->slaveMode);

    #ifdef COG_CRANK_OUTPUT
    compareAndUpdateByte(json, "outputOnTime",       &cc->outputOnTime);
    #endif

    #ifdef COUNTDOWN
    compareAndUpdateString(json, "countdownTarget",  &cc->countdownTarget);
    #endif


    // ------------------------------------------------------------

    spiffsStorage.saveConfigToSpiffs();
    debugMsgUtl("Saved new config");
  } else {
    debugMsgUtl("Json parse failure: " + String(request->arg("body")));
  }

  // Return the updated values
  getConfigDataHandler(request);
}

// ************************************************************
// Time server page
// ************************************************************
void getTimeserverDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api timeserver GET request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ntpPool"] = cc->ntpPool;
  root["ntpUpdateInterval"] = cc->ntpUpdateInterval;
  root["tzs"] = cc->tzs;
  response->setLength();
  request->send(response);
}

// ************************************************************
// Time server config Post
// ************************************************************
void postTimeserverDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api timeserver POST request");
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    debugMsgUtl("NTP pool before: " + cc->ntpPool);
    cc->ntpPool = json["ntpPool"].as<String>();
    debugMsgUtl("Loaded NTP pool: " + cc->ntpPool);

    cc->ntpUpdateInterval = json["ntpUpdateInterval"].as<int>();
    debugMsgUtl("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));

    cc->tzs = json["tzs"].as<String>();
    debugMsgUtl("Loaded time zone string: " + cc->tzs);

    // Now apply the new confog
    ntpManager.setNtpPool(cc->ntpPool);
    ntpManager.setUpdateInterval(cc->ntpUpdateInterval);
    tzManager.setTZS(cc->tzs);
    debugMsgUtl("Applied new time config");

    spiffsStorage.saveConfigToSpiffs();
    debugMsgUtl("Saved new time config");
  } else {
    debugMsgUtl("Json parse failure: " + String(request->arg("body")));
  }

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ntpPool"] = cc->ntpPool;
  root["ntpUpdateInterval"] = cc->ntpUpdateInterval;
  root["tzs"] = cc->tzs;
  response->setLength();
  request->send(response);
}

// ************************************************************
// Time server page
// ************************************************************
void getZonesListDataHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api timeserver zone list GET request");
  
  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", spiffsStorage.getZoneConfigSpiffs());
  request->send(response);        
}

// ************************************************************
// Set a new arbitrary value
// ************************************************************
void postValueHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api value POST request");
  
//  #ifdef DEBUG
//  dumpArgs(request);
//  #endif

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    if (!checkPresence(json, "value")) {
      request->send(400, "text/json", "{\"error\": \"value parameter not found\"}");
      return;
    }
    int newValue = json["value"].as<int>();

    outputManager.setArbitraryValue(newValue);
    outputManager.setArbitraryValueDisplayTime(10);
  }
   
  request->send(200, "text/json", "{\"status\": \"Value set\"}");
}

// ************************************************************
// WiFi
// ************************************************************
void getCredentialsHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api wifi credentials request");
  
//  #ifdef DEBUG
//  dumpArgs(request);
//  #endif

  if (WiFi.isConnected()) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"connected\": \"true\", \"SSID\": \"" + WiFi.SSID() + "\"}");
    request->send(response);        
  } else {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"connected\": \"false\"}");
    request->send(response);        
  }
}

// ************************************************************
// WiFi Credentials
// ************************************************************
void postWiFiCredentialsHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api wifi credentials POST request");
  
//  #ifdef DEBUG
//  dumpArgs(request);
//  #endif

  String newSSID = "";
  String newPassword = "";

  if (request->hasArg("SSID")) {
    newSSID = request->arg("SSID");
  }
  if (request->hasArg("password")) {
    newPassword = request->arg("password");
  }

  if (newSSID.length() > 0 && newPassword.length() > 0) {
    debugMsgUtl("Setting new WiFi credentials - " + newSSID + ":" + newPassword);

    wifiManager.saveWiFiCredentials(newSSID, newPassword);

    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"Saved " + newSSID + "\"}");
    request->send(response);
  } else {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"No changes saved\"}");
    request->send(response);
  }
}

// ************************************************************
// Return a list of WiFi Networks
// ************************************************************
void getWiFiNetworksHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api wifi networks request");
  
  if (WiFi.isConnected()) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"connected\": \"true\", \"SSID\": \"" + WiFi.SSID() + "\"}");
    request->send(response);        
    debugMsgUtl("Scan aborted because we are already connected");
  } else {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"connected\": \"false\", \"SSIDs\": \"" + lastWiFiScan + "\"}");
    request->send(response);        
    debugMsgUtl("Scan done");

    // trigger a new scan
    wifiManager.startScanWiFiNetworks();
  }
}

// ************************************************************
// Reset / restart
// ************************************************************
void restartHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got api restart request");
  
  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"Restart in 1s\"}");
  request->send(response);

  delay(1000);
  ESP.restart();
}

// ************************************************************
// Web Handler for reset WiFi
// ************************************************************
void resetWifiHandler(AsyncWebServerRequest *request) {
  resetWiFi();
  request->send(200, "text/json", "{\"status\": \"WiFi was reset\"}");
}

// ************************************************************
// Reset the WiFi credentials we have stored
// ************************************************************
void resetWiFi() {
  debugMsgUtl("Got utils RESET request");
  WiFi.disconnect();

  wifiManager.resetWiFiCredentials();
}

// ************************************************************
// Reset everything
// ************************************************************
void resetAll() {
  resetOptions();
  resetWiFi();
}

// ************************************************************
// Utilities
// ************************************************************
void getI2CScanHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got I2C scan request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();

  byte error, address;
  int nDevices;
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      debugMsgUtl("I2C device found at address 0x" + String(address, HEX));
      root["I2C"+String(address)] = "found";
      nDevices++;
    }
    else if (error==4) {
      debugMsgUtl("Unknown error at address 0x" + String(address, HEX));
    }    
  }
  if (nDevices == 0) {
    debugMsgUtl("No I2C devices found");
  }
  else {
    debugMsgUtl("done");
  }

  response->setLength();
  request->send(response);
}

// ************************************************************
// Utilities
// ************************************************************
void getSPIFFSScanHandler(AsyncWebServerRequest *request) {
  debugMsgUtl("Got SPIFFS scan request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& responseRoot = response->getRoot();

  responseRoot["FILE Listing"] = "";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
 
  int i = 1;
  while(file){ 
      responseRoot["FILE " + String(i) + ": "] = String(file.name());
      file = root.openNextFile();
      i++;
  }

  debugMsgUtl("done");

  response->setLength();
  request->send(response);
}

// ************************************************************
// Turn on Watchdog
// ************************************************************
void enableWatchdog() {
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
}

// ************************************************************
// Turn off Watchdog
// ************************************************************
void disableWatchdog() {
  esp_task_wdt_delete(NULL);
  esp_task_wdt_deinit();
}

// ************************************************************
// Reset the Watchdog timeout
// ************************************************************
void feedWatchdog() {
  esp_task_wdt_reset();
}

#ifdef TICKERS
int getBTCPrice() {
  int intBTCPrice = 0;
  HTTPClient http;  
  http.begin("https://api.binance.com/api/v3/avgPrice?symbol=BTCUSDT");

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    debugMsgUtl("HTTP reponse code: " + String(httpResponseCode));
    String payload = http.getString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parse(payload);
    intBTCPrice = json["price"];
    debugMsgUtl("BTC: " + String(intBTCPrice));
  }
  else {
    debugMsgUtl("HTTP failure code: " + String(httpResponseCode));
  }

  return intBTCPrice;
}
#endif