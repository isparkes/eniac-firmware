#include "utilities.h"
#include "LDRManager.h"

// --------------------------------------------------------------------------------------------------------
// ----------------------------------------  Utility functions  -------------------------------------------
// --------------------------------------------------------------------------------------------------------

void debugMsg(String message) {
    Serial.println(message);
    Serial.flush();
}

// ************************************************************
// Set the time from the value we get back from the time server
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
// Set the time from the value we get back from the time server
// ************************************************************
void setTimeFromServer(String timeString) {
  #define SYNC_HOURS 3
  #define SYNC_MINS 4
  #define SYNC_SECS 5
  #define SYNC_DAY 2
  #define SYNC_MONTH 1
  #define SYNC_YEAR 0

  int intValues[6];
  grabInts(timeString, &intValues[0], ",");
  setTime(intValues[SYNC_HOURS], intValues[SYNC_MINS], intValues[SYNC_SECS], intValues[SYNC_DAY], intValues[SYNC_MONTH], intValues[SYNC_YEAR]);
  debugMsg("Set internal time to NTP time: " + String(year()) + ":" + String(month()) + ":" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
}

 // ************************************************************
  // Callback: When the NTP component tells us there is an update
  // go and get it
  // ************************************************************
  void newTimeUpdateReceived() {
    debugMsg("Got a new time update: " + ntpAsync.getLastTimeFromServer());
    setTimeFromServer(ntpAsync.getLastTimeFromServer());
  }

void mainHandler(AsyncWebServerRequest *request) {
	debugMsg("Got request");
	request->send(SPIFFS, "/web/index.html");
}

void cssHandler(AsyncWebServerRequest *request) {
	debugMsg("Got css request");
	request->send(SPIFFS, "/web/style.css");
}

String timeToReadableString(int y, int m, int d, int h, int mi, int s) {
  char buf1[20];
  sprintf(buf1, "%04d:%02d:%02d %02d:%02d:%02d", y, m, d, h, mi, s);
  return String(buf1);
}

String timeStringToReadableString(String timeString){
  char* ptr = strtok((char *)timeString.c_str(), ",");
  int y = atoi(ptr);
  ptr = strtok(NULL, ",");
  int m = atoi(ptr);
  ptr = strtok(NULL, ",");
  int d = atoi(ptr);
  ptr = strtok(NULL, ",");
  int h = atoi(ptr);
  ptr = strtok(NULL, ",");
  int mi = atoi(ptr);
  ptr = strtok(NULL, ",");
  int s = atoi(ptr);
  return timeToReadableString(y,m,d,h,mi,s);
}

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
  uptimeString += secsValue; 
  uptimeString += " s";

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
  if (ntpAsync.ntpTimeValid(nowMillis)) {
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

#ifdef DEBUG
  connectionInfo += "D";
#else
  connectionInfo += "d";
#endif

  return connectionInfo;
}

uint32_t decodeBCD(byte valueToDecode, bool led1, bool led2) {
  uint32_t decoded = DECODE_DIGIT[(valueToDecode%10)] << 10 | DECODE_DIGIT[(valueToDecode/10)];
  if (led1) decoded |= DECODE_LED[0];
  if (led2) decoded |= DECODE_LED[1];
  return decoded;
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------    Web Interface     -------------------------------------------
// --------------------------------------------------------------------------------------------------------

void getSummaryDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api summary GET request");
  
  long nowMillis = millis();

  signed long absNextUpdate = abs(ntpAsync.getNextUpdate(nowMillis));
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
  root["tz"] = ntpAsync.getTZS();
  root["ntppool"] = ntpAsync.getNtpPool();
  String clockUrl = "http://" + String(WiFi.getHostname()) + ".local";
  clockUrl.toLowerCase();
  root["clockurl"] = clockUrl;
  root["lastntptime"] = timeStringToReadableString(ntpAsync.getLastTimeFromServer());
  root["lastntpupdate"] = secsToReadableString(ntpAsync.getLastUpdateTimeSecs(nowMillis));
  root["nextupdate"] = secsToReadableString(absNextUpdate) + overdueInd;
  root["displaytime"] = timeToReadableString(year(),month(),day(),hour(),minute(),second());

  root["ldrvalue"] = ldrValue;

  root["uptime"] = cs->uptimeMins;
  root["ontime"] = cs->tubeOnTimeMins;
  root["status"] = getStatusString();
  root["version"] = SOFTWARE_VERSION;

  root["heap"] = ESP.getFreeHeap();
  root["freesketch"] = ESP.getFreeSketchSpace();
  root["sketchsize"] = ESP.getSketchSize();
  root["compiledate"] = String(CONFIG_APP_COMPILE_TIME_DATE);
  root["cpufreq"] = ESP.getCpuFreqMHz();
  root["sdkversion"] = ESP.getSdkVersion();
  root["sketchmd5"] = ESP.getSketchMD5();
  root["cyclecount"] = ESP.getCycleCount();

  response->setLength();
  request->send(response);
}

void getConfigDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api config GET request");
  
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

  response->setLength();
  request->send(response);
}

void postConfigDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api config POST request");

  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    if (json.containsKey("thresholdBright")) {
      int newthresholdBright = json["thresholdBright"];
      if (cc->thresholdBright != newthresholdBright) {
        debugMsg("thresholdBright before: " + String(cc->thresholdBright));
        cc->thresholdBright = newthresholdBright;
        debugMsg("thresholdBright new minDim: " + String(cc->thresholdBright));
      }
    }

    if (json.containsKey("sensitivityLDR")) {
      int newsensitivityLDR = json["sensitivityLDR"];
      if (cc->sensitivityLDR != newsensitivityLDR) {
        debugMsg("sensitivityLDR before: " + String(cc->sensitivityLDR));
        cc->sensitivityLDR = newsensitivityLDR;
        debugMsg("Loaded new sensitivityLDR: " + String(cc->sensitivityLDR));
      }
    }

    if (json.containsKey("minDim")) {
      int newMinDim = json["minDim"];
      if (cc->minDim != newMinDim) {
        debugMsg("minDim before: " + String(cc->minDim));
        cc->minDim = newMinDim;
        debugMsg("Loaded new minDim: " + String(cc->minDim));
      }
    }

    if (json.containsKey("useLDR")) {
      int newUseLDR = json["useLDR"].as<bool>();
      if (cc->useLDR != newUseLDR) {
        debugMsg("useLDR before: " + String(cc->useLDR));
        cc->useLDR = newUseLDR;
        debugMsg("Loaded new useLDR: " + String(cc->useLDR));
      }
    }

    spiffsStorage.saveConfigToSpiffs(cc);
    debugMsg("Saved new config");
  } else {
    debugMsg("Json parse failure: " + String(request->arg("body")));
  }

  // Return the updated values
  getConfigDataHandler(request);
}

void getTimeserverDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api timeserver GET request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ntpPool"] = cc->ntpPool;
  root["ntpUpdateInterval"] = cc->ntpUpdateInterval;
  root["tzs"] = cc->tzs;
  response->setLength();
  request->send(response);
}

void dumpArgs(AsyncWebServerRequest *request) {
  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    Serial.printf("HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  if (request->hasArg("body")) {
    Serial.println("Body found arg");
  }
  if (request->hasParam("body")) {
    Serial.println("Body found param");
  }
  int args = request->args();
  for(int i=0;i<args;i++){
    Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }  
}

void postTimeserverDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api timeserver POST request");
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    debugMsg("NTP pool before: " + cc->ntpPool);
    cc->ntpPool = json["ntpPool"].as<String>();
    debugMsg("Loaded NTP pool: " + cc->ntpPool);

    cc->ntpUpdateInterval = json["ntpUpdateInterval"].as<int>();
    debugMsg("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));

    cc->tzs = json["tzs"].as<String>();
    debugMsg("Loaded time zone string: " + cc->tzs);

    // Now apply the new confog
    ntpAsync.setNtpPool(cc->ntpPool);
    ntpAsync.setUpdateInterval(cc->ntpUpdateInterval);
    ntpAsync.setTZS(cc->tzs);
    debugMsg("Applied new time config");

    spiffsStorage.saveConfigToSpiffs(cc);
    debugMsg("Saved new time config");
  } else {
    debugMsg("Json parse failure: " + String(request->arg("body")));
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

void postWiFiDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api wifi POST request");
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    String newSSID = json["SSID"].as<String>();
    debugMsg("Received SSID: " + newSSID);

    String newPassword = json["password"].as<String>();
    debugMsg("Received password: " + newPassword);

  } else {
    debugMsg("Json parse failure: " + String(request->arg("body")));
  }

  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"OK\"}");
  request->send(response);
        
}

void resetWifi() {
  WiFi.disconnect(false, true);
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
//  cc->backlightDimFactor = BACKLIGHT_DIM_FACTOR_DEFAULT;
//  cc->extDimFactor = EXT_DIM_FACTOR_DEFAULT;
//  cc->separatorDimFactor = SEPARATOR_DIM_FACTOR_DEFAULT;
//  cc->ledMode = LED_BLINK_DEFAULT;

  cc->blankMode = BLANK_MODE_DEFAULT;
  cc->blankHourStart = 0;
  cc->blankHourEnd = 7;

  cc->useLDR = USE_LDR_DEFAULT;
  
  cc->pirTimeout = PIR_TIMEOUT_DEFAULT;
  cc->usePIRPullup = USE_PIR_PULLUP_DEFAULT;
  
  // cc->webAuthentication = getWebAuthentication();
  // cc->webUsername = getWebUserName();
  // cc->webPassword = getWebPassword();
  // setWebAuthentication(WEB_AUTH_DEFAULT);
  // setWebUserName(WEB_USERNAME_DEFAULT);
  // setWebPassword(WEB_PASSWORD_DEFAULT);
  
  cc->antiGhost = ANTI_GHOST_DEFAULT;
  cc->testMode = true;

  spiffsStorage.saveConfigToSpiffs(cc);
  debugMsg("Saved factory config");
}

void resetAll() {
  WiFi.disconnect(false, true);  
}

void getCredentialsHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api wifi credntials request");
  
  dumpArgs(request);

  if ((request->hasArg("ssid")) && (request->hasArg("password"))) {
    ssid = request->arg("ssid");
    password = request->arg("password");
    credentialsReceived = true;
  }

  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"OK\"}");
  request->send(response);        
}

bool gotCredentials() {
  return credentialsReceived;
}

void wifiBeginWithCredentials() {
  WiFi.disconnect();
  delay(1000);
  WiFi.mode(WIFI_MODE_STA);
  delay(1000);
  delay(1000);
  WiFi.begin(ssid.c_str(), password.c_str());

  // reset the credentials so that we may have another go if necessary
  credentialsReceived = false;
}

void getI2CScanHandler(AsyncWebServerRequest *request) {
  debugMsg("Got I2C scan request");
  
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
      debugMsg("I2C device found at address 0x" + String(address, HEX));
      root["I2C"+String(address)] = "found";
      nDevices++;
    }
    else if (error==4) {
      debugMsg("Unknown error at address 0x" + String(address, HEX));
    }    
  }
  if (nDevices == 0) {
    debugMsg("No I2C devices found");
  }
  else {
    debugMsg("done");
  }

  response->setLength();
  request->send(response);
}

void saveStatsHandler(AsyncWebServerRequest *request) {
  debugMsg("Got save stats request");

  spiffsStorage.saveStatsToSpiffs(cs);
  
  request->send(200, "text/plain", "Stats saved");
}

void resetWifiHandler(AsyncWebServerRequest *request) {
  debugMsg("Got utils RESET request");
  WiFi.disconnect();
  request->send(200, "text/plain", "WiFi was reset");
}
