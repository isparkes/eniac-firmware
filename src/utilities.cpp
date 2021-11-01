#include "utilities.h"

// --------------------------------------------------------------------------------------------------------
// ----------------------------------------  Utility functions  -------------------------------------------
// --------------------------------------------------------------------------------------------------------

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
String timeToReadableString(int y, int m, int d, int h, int mi, int s) {
  char buf1[20];
  sprintf(buf1, "%04d:%02d:%02d %02d:%02d:%02d", y, m, d, h, mi, s);
  return String(buf1);
}

// ************************************************************
// Format a time into an output string - takes a string imput
// like this: "yyyy,mm,dd,hh,mi,ss"
// ************************************************************
String timeStringToReadableString(String timeString) {
  int intValues[6];
  grabInts(timeString, &intValues[0], ",");

  return timeToReadableString(intValues[0],intValues[1],intValues[2],intValues[3],intValues[4],intValues[5]);
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

  if (oledTime > 0) {
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

  spiffsStorage.saveConfigToSpiffs(cc);
  #ifdef DEBUG_ON
  debugManager.debugMsg("Saved factory config");
  #endif
}

void resetAll() {
  WiFi.disconnect(false, true);  
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
  debugManager.debugMsg("Got a new time update: " + ntpManager.getLastTimeFromServer());
  #endif
  rtcManager.setTimeFromServer(ntpManager.getLastTimeFromServer(), nowMillis);
}

//**********************************************************************************
//**********************************************************************************
//*                              Display Scheduling                                *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Turn a display pair into a uint24 ready for output
// ************************************************************
uint32_t decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool blankUnits, bool bl1, bool bl2, bool led1, bool led2) {
  uint32_t decoded = 0;
  if (!blankTens) decoded = DECODE_DIGIT[valueToDecodeTens];
  if (!blankUnits) decoded = decoded | DECODE_DIGIT[valueToDecodeUnits] << 10;
  if (led1) decoded |= DECODE_LED[0];
  if (led2) decoded |= DECODE_LED[1];
  if (bl1)  decoded |= DECODE_BLINKENIGHTS[0];
  if (bl2)  decoded |= DECODE_BLINKENIGHTS[1];
  return decoded;
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void loadNumberArrayTime() {
  numberArray[S1]  = second() % 10;
  numberArray[S10] = second() / 10;
  numberArray[M1]  = minute() % 10;
  numberArray[M10] = minute() / 10;
  if (cc->hourMode) {
    numberArray[H1]  = hourFormat12() % 10;
    numberArray[H10] = hourFormat12() / 10;
  } else {
    numberArray[H1]  = hour() % 10;
    numberArray[H10] = hour() / 10;
  }
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void loadNumberArraySameValue(byte value) {
  byte val = value % 10;
  numberArray[S1]  = val;
  numberArray[S10] = val;
  numberArray[M1]  = val;
  numberArray[M10] = val;
  numberArray[H1]  = val;
  numberArray[H10] = val;
}

// ************************************************************
// Do a single complete display, including any fading and
// dimming requested. Prepares the display variables for
// the interrupt driven display output.
// This is the heart of the display processing!
// ************************************************************
void outputDisplay() {
  byte tmpDispType;
  byte tmpDispTypeArray[DIGIT_COUNT];
  byte tmpNumberArray[DIGIT_COUNT];

  for ( int i = DIGIT_COUNT - 1 ; i >= 0  ; i -- ) {
    // Blanking
    if (blankingManager.getCurrentBlankTubes()) {
      tmpDispType = BLANKED;
    } else {
      tmpDispType = displayType[i];
    }

    // Digit blinking
    if (tmpDispType == BLINK) {
      if (blinkState) {
      } else {
        tmpDispType = BLANKED;
      }
    }

    // Trigger scolling and fading - scolling takes precendence
    if (numberArray[i] != currNumberArray[i]) {
      // Do scrollback when we are going to 0
      if ((numberArray[i] == 0) && cc->scrollback && (scrollCounter[i] == 0)) {
        scrollCounter[i] = (currNumberArray[i]+1) * cc->scrollSteps;
      } else if ((fadeState == 0) && cc->fade) {
        // if we are not going to 0, set up the fade steps
        fadeState = cc->fadeSteps;
      } else if (fadeState == 0) {
        currNumberArray[i] = numberArray[i];
      }
    }


    if (scrollCounter[i] > 0) {
      scrollCounter[i] = scrollCounter[i] - 1;
      currNumberArray[i] = scrollCounter[i]/cc->scrollSteps;
      tmpNumberArray[i] = currNumberArray[i];
    } else {
      tmpNumberArray[i] = numberArray[i];
    }

    tmpDispTypeArray[i] = tmpDispType;
  }

  uint8_t tmpSwitchTime = 0;
  if (fadeState == 1) {
    fadeState = 0;
    for (byte j = 0 ; j < DIGIT_COUNT ; j++) {
      if (scrollCounter[j] == 0) {
        currNumberArray[j] = numberArray[j];
      }
    }
  } else if (fadeState > 0) {
    fadeState--;
    tmpSwitchTime = PHASE_MAX - (PHASE_MAX * fadeState / cc->fadeSteps);
  }

  uint32_t tmpval1 = decodeFromNumberArray( tmpNumberArray[H10], 
                                tmpNumberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl->bl1,
                                bl->bl2,
                                led1State,
                                led2State);
  uint32_t tmpval2 = decodeFromNumberArray( tmpNumberArray[M10], 
                                tmpNumberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl->bl3,
                                bl->bl4,
                                led1State,
                                led2State);
  uint32_t tmpval3 = decodeFromNumberArray( tmpNumberArray[S10], 
                                tmpNumberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl->bl5,
                                bl->bl6,
                                indLed1,
                                indLed2);

  // ToDo fading/scrolling
  uint32_t tmpnextVal1 = decodeFromNumberArray( currNumberArray[H10], 
                                currNumberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl->bl1,
                                bl->bl2,
                                led1State,
                                led2State);
  uint32_t tmpnextVal2 = decodeFromNumberArray( currNumberArray[M10], 
                                currNumberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl->bl3,
                                bl->bl4,
                                led1State,
                                led2State);
  uint32_t tmpnextVal3 = decodeFromNumberArray( currNumberArray[S10], 
                                currNumberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl->bl5,
                                bl->bl6,
                                indLed1,
                                indLed2);

  // move the values over, respect the MUTEX on the interrupt
  portENTER_CRITICAL_ISR(&timerMux1);
  val1 = tmpval1;
  val2 = tmpval2;
  val3 = tmpval3;
  nextVal1 = tmpnextVal1;
  nextVal2 = tmpnextVal2;
  nextVal3 = tmpnextVal3;
  switchTime = tmpSwitchTime;
  portEXIT_CRITICAL_ISR(&timerMux1);
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
    debugManager.debugMsg(message);
  }

  if (request->hasArg("body")) {
    debugManager.debugMsg("Body found arg");
  }
  if (request->hasParam("body")) {
    debugManager.debugMsg("Body found param");
  }
  int args = request->args();
  for(int i=0;i<args;i++){
    String message = "ARG[" + request->argName(i) + "]: " + request->arg(i); 
    debugManager.debugMsg(message);
  }  
}
#endif


// ************************************************************
// Main page handler
// ************************************************************
void mainHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
	debugManager.debugMsg("Got request");
  #endif
	request->send(SPIFFS, "/web/index.html");
}

// ************************************************************
// Main CSS handler
// ************************************************************
void cssHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
	debugManager.debugMsg("Got css request");
  #endif
	request->send(SPIFFS, "/web/style.css");
}

// ************************************************************
// Summary page
// ************************************************************
void getSummaryDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got api summary GET request");
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
  root["tz"] = ntpManager.getTZS();
  root["ntppool"] = ntpManager.getNtpPool();
  String clockUrl = "http://" + String(WiFi.getHostname()) + ".local";
  clockUrl.toLowerCase();
  root["clockurl"] = clockUrl;
  root["currentntptime"] = timeStringToReadableString(ntpManager.getEstimatedCurrentTime(nowMillis));
  root["lastntpupdate"] = secsToReadableString(ntpManager.getLastUpdateTimeSecs(nowMillis));
  root["nextupdate"] = secsToReadableString(absNextUpdate) + overdueInd;
  if (ntpManager.ntpTimeValid(nowMillis)) {
    root["ntpvalid"] = 1;
  } else {
    root["ntpvalid"] = 0;
  }
  root["displaytime"] = timeToReadableString(year(),month(),day(),hour(),minute(),second());

  unsigned long gpsAge = (nowMillis - gpsManager.getLastGPSReadTime())/1000;
  if (gpsManager.getLastGPSReadTime() > 0) {
    root["lastgpstime"] = timeStringToReadableString(gpsManager.getLastGPSTime());
    root["lastgpsupdate"] = secsToReadableString(gpsAge);
    if (gpsManager.getGPSTimeValid(nowMillis)) {
      root["gpsvalid"] = 1;
    } else {
      root["gpsvalid"] = 0;
    }
  } else {
    root["lastgpstime"] = "GPS Receiver not installed";
    root["lastgpsupdate"] = "";
  }

  if (rtcManager.getRTCValid()) {
    unsigned long rtcAge = (nowMillis - rtcManager.getLastRTCSetTime())/1000;
    root["lastrtctime"] = timeStringToReadableString(rtcManager.getEstimatedCurrentRTCTime(nowMillis));
    root["lastrtcupdate"] = secsToReadableString(rtcAge);
  } else {
    root["lastrtctime"] = "RTC not installed";
    root["lastrtcupdate"] = "";
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
  debugManager.debugMsg("Got api diagnostics GET request");
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
  debugManager.debugMsg("Got diags POST request");
  #endif
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    #ifdef DEBUG_ON
    debugManager.debugMsg("Diags mode before: " + String(cc->diagsMode));
    #endif
    cc->diagsMode = json["diagsMode"].as<int>();
    #ifdef DEBUG_ON
    debugManager.debugMsg("Diags mode after: " + String(cc->diagsMode));
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
  debugManager.debugMsg("Got save stats request");
  #endif

  spiffsStorage.saveStatsToSpiffs(cs);
  
  request->send(200, "text/json", "{\"status\": \"Stats saved\"}");
}

// ************************************************************
// Config page
// ************************************************************
void getConfigDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got api config GET request");
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
      debugManager.debugMsg(String(key) + " old: " + String(*variable));
      #endif
      *variable = newVal;
      #ifdef DEBUG_ON
      debugManager.debugMsg(String(key) + " new: " + String(*variable));
      #endif
    }
  }
}

void compareAndUpdateInt(JsonObject& json, const char* key, int* variable) {
  if (json.containsKey(key)) {
    int newVal = json[key];
    if (*variable != newVal) {
      #ifdef DEBUG_ON
      debugManager.debugMsg(String(key) + " old: " + String(*variable));
      #endif
      *variable = newVal;
      #ifdef DEBUG_ON
      debugManager.debugMsg(String(key) + " new: " + String(*variable));
      #endif
    }
  }
}
void compareAndUpdateBool(JsonObject& json, const char* key, bool* variable) {
  if (json.containsKey(key)) {
    bool newVal = json[key].as<bool>();
    if (*variable != newVal) {
      #ifdef DEBUG_ON
      debugManager.debugMsg(String(key) + " old: " + String(*variable));
      #endif
      *variable = newVal;
      #ifdef DEBUG_ON
      debugManager.debugMsg(String(key) + " new: " + String(*variable));
      #endif
    }
  }
}

void postConfigDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got api config POST request");
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
    debugManager.debugMsg("Saved new config");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugManager.debugMsg("Json parse failure: " + String(request->arg("body")));
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
  debugManager.debugMsg("Got api timeserver GET request");
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
  debugManager.debugMsg("Got api timeserver POST request");
  #endif
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    #ifdef DEBUG_ON
    debugManager.debugMsg("NTP pool before: " + cc->ntpPool);
    #endif
    cc->ntpPool = json["ntpPool"].as<String>();
    #ifdef DEBUG_ON
    debugManager.debugMsg("Loaded NTP pool: " + cc->ntpPool);
    #endif

    cc->ntpUpdateInterval = json["ntpUpdateInterval"].as<int>();
    #ifdef DEBUG_ON
    debugManager.debugMsg("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));
    #endif

    cc->tzs = json["tzs"].as<String>();
    #ifdef DEBUG_ON
    debugManager.debugMsg("Loaded time zone string: " + cc->tzs);
    #endif

    // Now apply the new confog
    ntpManager.setNtpPool(cc->ntpPool);
    ntpManager.setUpdateInterval(cc->ntpUpdateInterval);
    ntpManager.setTZS(cc->tzs);
    #ifdef DEBUG_ON
    debugManager.debugMsg("Applied new time config");
    #endif

    spiffsStorage.saveConfigToSpiffs(cc);
    #ifdef DEBUG_ON
    debugManager.debugMsg("Saved new time config");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugManager.debugMsg("Json parse failure: " + String(request->arg("body")));
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
  debugManager.debugMsg("Got api wifi credentials request");
  #endif
  
  #ifdef DEBUG_ON
  dumpArgs(request);
  #endif

  if ((request->hasArg("ssid")) && (request->hasArg("password"))) {
    ssid = request->arg("ssid");
    password = request->arg("password");
    credentialsReceived = true;
  }

  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"OK\"}");
  request->send(response);        
}

void getWifiConnected(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got api wifi connected request");
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

void postWiFiDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got api wifi POST request");
  #endif
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    String newSSID = json["SSID"].as<String>();
    #ifdef DEBUG_ON
    debugManager.debugMsg("Received SSID: " + newSSID);
    #endif

    String newPassword = json["password"].as<String>();
    #ifdef DEBUG_ON
    debugManager.debugMsg("Received password: " + newPassword);
    #endif

  } else {
    #ifdef DEBUG_ON
    debugManager.debugMsg("Json parse failure: " + String(request->arg("body")));
    #endif
  }

  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"OK\"}");
  request->send(response);
        
}

// ************************************************************
// Reset / restart
// ************************************************************
void restartHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got api restart request");
  #endif
  
  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"Restart in 1s\"}");
  request->send(response);

  delay(1000);
  ESP.restart();
}

void resetWifiHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got utils RESET request");
  #endif
  WiFi.disconnect();
  request->send(200, "text/json", "{\"status\": \"WiFi was reset\"}");
}

// ************************************************************
// Utilities
// ************************************************************
void getI2CScanHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("Got I2C scan request");
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
      debugManager.debugMsg("I2C device found at address 0x" + String(address, HEX));
      #endif
      root["I2C"+String(address)] = "found";
      nDevices++;
    }
    else if (error==4) {
      #ifdef DEBUG_ON
      debugManager.debugMsg("Unknown error at address 0x" + String(address, HEX));
      #endif
    }    
  }
  if (nDevices == 0) {
    #ifdef DEBUG_ON
    debugManager.debugMsg("No I2C devices found");
  #endif
  }
  else {
    #ifdef DEBUG_ON
    debugManager.debugMsg("done");
    #endif
  }

  response->setLength();
  request->send(response);
}
