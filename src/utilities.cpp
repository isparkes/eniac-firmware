#include "utilities.h"

// --------------------------------------------------------------------------------------------------------
// ----------------------------------------  Utility functions  -------------------------------------------
// --------------------------------------------------------------------------------------------------------

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void debugMsgUtl(String message) {
  debugManager.debugMsg("[UTL]: " + message);
}

// ************************************************************
// Split a separated string into individual array elements
// ************************************************************
void grabInts(String s, int *dest, String sep) {
  int end = 0;
  for (int start = 0; end != -1; start = end + 1) {
    end = s.indexOf(sep, start);

    if (end > 0) {
      *dest++ = s.substring(start, end).toInt();
    } else {
      *dest++ = s.substring(start).toInt();
    }
  }
}

// ************************************************************
// Format a time into an output string
// ************************************************************
String timeToReadableStringFromTm(tm timeToFormat) {
  char buf1[20];
  sprintf(buf1, "%04d:%02d:%02d %02d:%02d:%02d",
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
// See if we have enough flash space for OTA
// ************************************************************
bool getOTAvailable() {
  return ESP.getSketchSize() < ESP.getFreeSketchSpace();
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
  if (ntpManager.ntpTimeValid(nowMillis)) {
    connectionInfo += "N";
  } else {
    connectionInfo += "n";
  }
  // if (spiffsStorage.testMountSpiffs()) {
  //   connectionInfo += "S";
  // } else {
  //   connectionInfo += "s";
  // }
  if (getOTAvailable()) {
    connectionInfo += "U";

  } else {
    connectionInfo += "u";
  }
  
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

  if (configTime > 0) {
    connectionInfo += "O";
  } else {
    connectionInfo += "o";
  }

#ifdef DEBUG_ON
  connectionInfo += "D";
#else
  connectionInfo += "d";
#endif

  if (gpsManager.getGPSTimeValid(nowMillis)) {
    connectionInfo += "G";
  } else {
    connectionInfo += "g";
  }

  return connectionInfo;
}

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
  
  cc->fade = FADE_DEFAULT;
  cc->fadeSteps = FADE_STEPS_DEFAULT;
  cc->scrollback = SCROLLBACK_DEFAULT;
  cc->scrollSteps = SCROLL_STEPS_DEFAULT;
  cc->slotsMode = SLOTS_MODE_DEFAULT;
  
  cc->backlightMode = BACKLIGHT_DEFAULT;
  cc->useBLDim = true;
  cc->useBLPulse = false;
  cc->redCnl = COLOUR_RED_CNL_DEFAULT;
  cc->grnCnl = COLOUR_GRN_CNL_DEFAULT;
  cc->bluCnl = COLOUR_BLU_CNL_DEFAULT;
  cc->cycleSpeed = CYCLE_SPEED_DEFAULT;
  cc->backlightDimFactor = BACKLIGHT_DIM_FACTOR_DEFAULT;
//  cc->extDimFactor = EXT_DIM_FACTOR_DEFAULT;
//  cc->separatorDimFactor = SEPARATOR_DIM_FACTOR_DEFAULT;
  cc->ledMode = LED_BLINK_DEFAULT;

  cc->blankMode = BLANK_MODE_DEFAULT;
  cc->blankHourStart = 0;
  cc->blankHourEnd = 7;

  cc->mdTimeout = PIR_TIMEOUT_DEFAULT;
  
  // cc->webAuthentication = getWebAuthentication();
  // cc->webUsername = getWebUserName();
  // cc->webPassword = getWebPassword();
  // setWebAuthentication(WEB_AUTH_DEFAULT);
  // setWebUserName(WEB_USERNAME_DEFAULT);
  // setWebPassword(WEB_PASSWORD_DEFAULT);
  cc->hueOffset = HUE_OFFSET_DEFAULT;
  
  cc->testMode = true;
  cc->wasSetup = true;

  cc->WiFiSSID = "";
  cc->WiFiPassword = "";

  spiffsStorage.saveConfigToSpiffs(cc);
  #ifdef DEBUG_ON
  debugMsgUtl("Saved factory config");
  #endif
}

void resetAll() {
  resetOptions();
  resetWiFi();
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
  #ifdef DEBUG_ON
  debugMsgUtl("[UTL]: Got a new NTP time update: " + String(ntpManager.getLastTimeTFromServer()));
  #endif
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
#ifdef DEBUG_ON
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
  #ifdef DEBUG_ON
	debugMsgUtl("Got request");
  #endif
	request->send(SPIFFS, "/web/index.html");
}

// ************************************************************
// Main CSS handler
// ************************************************************
void cssHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
	debugMsgUtl("Got css request");
  #endif
	request->send(SPIFFS, "/web/style.css");
}

// ************************************************************
// Summary page
// ************************************************************
void getSummaryDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api summary GET request");
  #endif
  
  signed long absNextUpdate = abs(ntpManager.getNextUpdate(nowMillis));
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
  root["timeSource"] = tzManager.getPrimaryTimeSource(nowMillis);
  root["currentntptime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_NTP, nowMillis);
  root["lastntpupdate"] = secsToReadableString(tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_NTP, nowMillis));
  root["nextupdate"] = secsToReadableString(absNextUpdate) + overdueInd;
  if (ntpManager.ntpTimeValid(nowMillis)) {
    root["ntpvalid"] = 1;
  } else {
    root["ntpvalid"] = 0;
  }
  root["displaytime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_INT, nowMillis);

  if (gpsManager.getLastGPSReadTime() > 0) {
    root["lastgpstime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_GPS, nowMillis);
    root["lastgpsupdate"] = secsToReadableString(tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_GPS, nowMillis));
    if (gpsManager.getGPSTimeValid(nowMillis)) {
      root["gpsvalid"] = 1;
    } else {
      root["gpsvalid"] = 0;
    }
  } else {
    root["lastgpstime"] = "GPS Receiver not installed";
    root["lastgpsupdate"] = "";
    root["gpsvalid"] = 0;
  }

  if (rtcManager.getRTCValid()) {
    root["lastrtctime"] = tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis);
    root["lastrtcupdate"] = secsToReadableString(tzManager.getTimeLastSetFromTimeSource(TIME_SOURCE_GPS, nowMillis));
    root["rtcvalid"] = 1;
  } else {
    root["lastrtctime"] = "RTC not installed";
    root["lastrtcupdate"] = "";
    root["rtcvalid"] = 0;
  }

  float ldrPerc = (4095 - ldrValue) / 4095.0 * 100.0;
  root["ldrvalue"] = String(ldrPerc, 2) + "% (" + String(ldrValue) + ")";

  bool pirInstalled = blankingManager.getCurrentPIRInstalled();
  root["mdInstalled"] = pirInstalled;
  if (pirInstalled) {
    root["mdLastSeen"] = secsToReadableString(blankingManager.getBlankAge(nowMillis));
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
  #ifdef DEBUG_ON
  debugMsgUtl("Got api diagnostics GET request");
  #endif
  
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
  root["utcoffset"] = String(tzManager.getCurrentUTCOffset());
#ifdef DIGIT_DIAGNOSTICS
  root["diagsMode"] = cc->diagsMode;
#endif

  response->setLength();
  request->send(response);
}

void postDiagsDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got diags POST request");
  #endif
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    #ifdef DEBUG_ON
    debugMsgUtl("Diags mode before: " + String(cc->diagsMode));
    #endif
    cc->diagsMode = json["diagsMode"].as<int>();
    #ifdef DEBUG_ON
    debugMsgUtl("Diags mode after: " + String(cc->diagsMode));
    #endif
  }
   
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["diagsMode"] = cc->diagsMode;
  response->setLength();
  request->send(response);
}

void saveStatsHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got save stats request");
  #endif

  spiffsStorage.saveStatsToSpiffs(cs);
  
  request->send(200, "text/json", "{\"status\": \"Stats saved\"}");
}

// ************************************************************
// Config page
// ************************************************************
void getConfigDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api config GET request");
  #endif
  
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
  root["suppressACP"] = cc->suppressACP;

  root["useLDR"] = cc->useLDR;
  root["minDim"] = cc->minDim;
  root["thresholdBright"] = cc->thresholdBright;
  root["sensitivityLDR"] = cc->sensitivityLDR;

  root["mdinstalled"] = blankingManager.getCurrentPIRInstalled();
  root["mdTimeout"] = cc->mdTimeout;
  root["mdBlankMode"] = cc->mdBlankMode;
  root["dayBlanking"] = cc->dayBlanking;
  root["blankMode"] = cc->blankMode;
  root["blankHourStart"] = cc->blankHourStart;
  root["blankHourEnd"] = cc->blankHourEnd;

  root["backlightMode"] = cc->backlightMode;
  root["redCnl"] = cc->redCnl;
  root["grnCnl"] = cc->grnCnl;
  root["bluCnl"] = cc->bluCnl;
  root["useBLDim"] = cc->useBLDim;
  root["useBLPulse"] = cc->useBLPulse;
  root["cycleSpeed"] = cc->cycleSpeed;
  root["backlightDimFactor"] = cc->backlightDimFactor;
  root["hueOffset"] = cc->hueOffset;

  response->setLength();
  request->send(response);
}

void compareAndUpdateByte(JsonObject& json, const char* key, byte* variable) {
  if (json.containsKey(key)) {
    byte newVal = json[key];
    if (*variable != newVal) {
      #ifdef DEBUG_ON
      debugMsgUtl(String(key) + " old: " + String(*variable));
      #endif
      *variable = newVal;
      #ifdef DEBUG_ON
      debugMsgUtl(String(key) + " new: " + String(*variable));
      #endif
    }
  }
}

void compareAndUpdateInt(JsonObject& json, const char* key, int* variable) {
  if (json.containsKey(key)) {
    int newVal = json[key];
    if (*variable != newVal) {
      #ifdef DEBUG_ON
      debugMsgUtl(String(key) + " old: " + String(*variable));
      #endif
      *variable = newVal;
      #ifdef DEBUG_ON
      debugMsgUtl(String(key) + " new: " + String(*variable));
      #endif
    }
  }
}
void compareAndUpdateBool(JsonObject& json, const char* key, bool* variable) {
  if (json.containsKey(key)) {
    bool newVal = json[key].as<bool>();
    if (*variable != newVal) {
      #ifdef DEBUG_ON
      debugMsgUtl(String(key) + " old: " + String(*variable));
      #endif
      *variable = newVal;
      #ifdef DEBUG_ON
      debugMsgUtl(String(key) + " new: " + String(*variable));
      #endif
    }
  }
}

void postConfigDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api config POST request");
  #endif

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
    compareAndUpdateBool(json, "suppressACP",  &cc->suppressACP);

    // ------------------------------------------------------------

    compareAndUpdateBool(json, "useLDR",          &cc->useLDR);
    compareAndUpdateInt (json, "minDim",          &cc->minDim);
    compareAndUpdateInt (json, "thresholdBright", &cc->thresholdBright);
    compareAndUpdateInt (json, "sensitivityLDR",  &cc->sensitivityLDR);

    // ------------------------------------------------------------

    compareAndUpdateInt (json, "mdTimeout",      &cc->mdTimeout);
    compareAndUpdateByte(json, "mdBlankMode",    &cc->mdBlankMode);
    compareAndUpdateByte(json, "dayBlanking",    &cc->dayBlanking);
    compareAndUpdateByte(json, "blankMode",      &cc->blankMode);
    compareAndUpdateByte(json, "blankHourStart", &cc->blankHourStart);
    compareAndUpdateByte(json, "blankHourEnd",   &cc->blankHourEnd);

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

    // ------------------------------------------------------------

    spiffsStorage.saveConfigToSpiffs(cc);
    #ifdef DEBUG_ON
    debugMsgUtl("Saved new config");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsgUtl("Json parse failure: " + String(request->arg("body")));
    #endif
  }

  // Return the updated values
  getConfigDataHandler(request);
}

// ************************************************************
// Time server page
// ************************************************************
void getTimeserverDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api timeserver GET request");
  #endif
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ntpPool"] = cc->ntpPool;
  root["ntpUpdateInterval"] = cc->ntpUpdateInterval;
  root["tzs"] = cc->tzs;
  response->setLength();
  request->send(response);
}

void postTimeserverDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api timeserver POST request");
  #endif
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    #ifdef DEBUG_ON
    debugMsgUtl("NTP pool before: " + cc->ntpPool);
    #endif
    cc->ntpPool = json["ntpPool"].as<String>();
    #ifdef DEBUG_ON
    debugMsgUtl("Loaded NTP pool: " + cc->ntpPool);
    #endif

    cc->ntpUpdateInterval = json["ntpUpdateInterval"].as<int>();
    #ifdef DEBUG_ON
    debugMsgUtl("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));
    #endif

    cc->tzs = json["tzs"].as<String>();
    #ifdef DEBUG_ON
    debugMsgUtl("Loaded time zone string: " + cc->tzs);
    #endif

    // Now apply the new confog
    ntpManager.setNtpPool(cc->ntpPool);
    ntpManager.setUpdateInterval(cc->ntpUpdateInterval);
    tzManager.setTZS(cc->tzs);
    #ifdef DEBUG_ON
    debugMsgUtl("Applied new time config");
    #endif

    spiffsStorage.saveConfigToSpiffs(cc);
    #ifdef DEBUG_ON
    debugMsgUtl("Saved new time config");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsgUtl("Json parse failure: " + String(request->arg("body")));
    #endif
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
// WiFi
// ************************************************************
void getCredentialsHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api wifi credentials request");
  #endif
  
  #ifdef DEBUG_ON
  dumpArgs(request);
  #endif

  if (WiFi.isConnected()) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"connected\": \"true\", \"SSID\": \"" + WiFi.SSID() + "\"}");
    request->send(response);        
  } else {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"connected\": \"false\"}");
    request->send(response);        
  }
}

void getWifiConnected(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api wifi connected request");
  #endif
  
  String isConnected = "";
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = "Connected";
  } else {
    isConnected = "Offlne";
  }
  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"" + isConnected + "\"}");
  request->send(response);        
}

void postWiFiCredentialsHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api wifi POST request");
  #endif
  
  #ifdef DEBUG_ON
  dumpArgs(request);
  #endif

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  String newSSID = "";
  String newPassword = "";

  if (json.success()) {
    newSSID = json["SSID"].as<String>();
    #ifdef DEBUG_ON
    debugMsgUtl("Received SSID: " + newSSID);
    #endif

    newPassword = json["password"].as<String>();
    #ifdef DEBUG_ON
    debugMsgUtl("Received password: " + newPassword);
    #endif

  } else {
    #ifdef DEBUG_ON
    debugMsgUtl("Json parse failure: " + String(request->arg("body")));
    #endif
  }

  if (newSSID.length() > 0 && newPassword.length() > 0) {
    #ifdef DEBUG_ON
    debugMsgUtl("Setting new WiFi credentials");
    #endif
    saveWiFiCredentials(newSSID, newPassword);

    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"Saved " + newSSID + "\"}");
    request->send(response);
  } else {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"No changes saved\"}");
    request->send(response);
  }

}

// ************************************************************
// Reset / restart
// ************************************************************
void restartHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got api restart request");
  #endif
  
  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"Restart in 1s\"}");
  request->send(response);

  delay(1000);
  ESP.restart();
}

void resetWifiHandler(AsyncWebServerRequest *request) {
  resetWiFi();
  request->send(200, "text/json", "{\"status\": \"WiFi was reset\"}");
}

void resetWiFi() {
  #ifdef DEBUG_ON
  debugMsgUtl("Got utils RESET request");
  #endif
  WiFi.disconnect();

  cc->WiFiSSID = "";
  cc->WiFiPassword = "";
  spiffsStorage.saveConfigToSpiffs(cc);
}

// ************************************************************
// Utilities
// ************************************************************
void getI2CScanHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsgUtl("Got I2C scan request");
  #endif
  
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
      #ifdef DEBUG_ON
      debugMsgUtl("I2C device found at address 0x" + String(address, HEX));
      #endif
      root["I2C"+String(address)] = "found";
      nDevices++;
    }
    else if (error==4) {
      #ifdef DEBUG_ON
      debugMsgUtl("Unknown error at address 0x" + String(address, HEX));
      #endif
    }    
  }
  if (nDevices == 0) {
    #ifdef DEBUG_ON
    debugMsgUtl("No I2C devices found");
  #endif
  }
  else {
    #ifdef DEBUG_ON
    debugMsgUtl("done");
    #endif
  }

  response->setLength();
  request->send(response);
}
