#include "utilities.h"
#include "LDRManager.h"
#include "ESP_DS1307.h"
#include <rom/rtc.h>
#include "clock_timers.h"
#include "globals.h"
#include "GPSManager.h"

// --------------------------------------------------------------------------------------------------------
// ----------------------------------------  Utility functions  -------------------------------------------
// --------------------------------------------------------------------------------------------------------

void debugMsg(String message) {
    Serial.println(message);
    Serial.flush();
}

void debugMsgCont(String message) {
    Serial.print(message);
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
// Format a time into an output string
// ************************************************************
String timeToReadableString(int y, int m, int d, int h, int mi, int s) {
  char buf1[20];
  sprintf(buf1, "%04d:%02d:%02d %02d:%02d:%02d", y, m, d, h, mi, s);
  return String(buf1);
}

// ************************************************************
// Format a time into an output string
// ************************************************************
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

  if (blanked) {
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

  cc->pirTimeout = PIR_TIMEOUT_DEFAULT;
  cc->usePIRPullup = USE_PIR_PULLUP_DEFAULT;
  
  // cc->webAuthentication = getWebAuthentication();
  // cc->webUsername = getWebUserName();
  // cc->webPassword = getWebPassword();
  // setWebAuthentication(WEB_AUTH_DEFAULT);
  // setWebUserName(WEB_USERNAME_DEFAULT);
  // setWebPassword(WEB_PASSWORD_DEFAULT);
  
  cc->testMode = true;
  cc->wasSetup = true;

  spiffsStorage.saveConfigToSpiffs(cc);
  #ifdef DEBUG_ON
  debugMsg("Saved factory config");
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
//*                         RTC Module Time Provider                               *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Check that we still have access to the time from the RTC
// ************************************************************
void testRTCTimeProvider() {
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  useRTC = (Wire.endTransmission(true) == 0);
  #ifdef DEBUG_ON
  debugMsg("Set useRTC to: " + String(useRTC));
  if (!useRTC) {
    debugMsg("I2C error: " + String(Wire.getErrorText(Wire.lastError())));
  }
  #endif
}

// ************************************************************
// Get the time from the RTC
// ************************************************************
String getRTCTime(boolean setInternalTime) {
  if (useRTC) {
    rtclock.getTime();
    int years = rtclock.year + 2000;
    byte months = rtclock.month;
    byte days = rtclock.dayOfMonth;
    byte hours = rtclock.hour;
    byte mins = rtclock.minute;
    byte secs = rtclock.second;

    String returnValue = timeToReadableString(years, months, days, hours, mins, secs);
    #ifdef DEBUG_ON
    debugMsg("Got RTC time: " + returnValue);
    #endif

    if (setInternalTime) {
      // Set the internal time provider to the value we got
      setTime(hours, mins, secs, days, months, years);
      #ifdef DEBUG_ON
      debugMsg("Set Internal time to: " + returnValue);
      #endif
    }

    return returnValue;
  } else {
    return "";
  }
}

// ************************************************************
// Set the date/time in the RTC from the internal time
// Always hold the time in 24 format, we convert to 12 in the
// display.
// ************************************************************
void setRTCTime() {
  if (useRTC) {
    rtclock.fillByYMD(year() % 100, month(), day());
    rtclock.fillByHMS(hour(), minute(), second());
    rtclock.setTime();

    #ifdef DEBUG_ON
    debugMsg("Set RTC time to internal time: " + String(year()) + ":" + String(month()) + ":" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
    #endif
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

  // Push the update to the RTC chip
  setRTCTime();
  
  #ifdef DEBUG_ON
  debugMsg("Set RTC time to NTP time: " + String(year()) + ":" + String(month()) + ":" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
  #endif
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
  debugMsg("Got a new time update: " + ntpAsync.getLastTimeFromServer());
  #endif
  setTimeFromServer(ntpAsync.getLastTimeFromServer());
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
  numberArray[S1] = second() % 10;
  numberArray[S10] = second() / 10;
  numberArray[M1] = minute() % 10;
  numberArray[M10] = minute() / 10;
  if (cc->hourMode) {
    numberArray[H1] = hourFormat12() % 10;
    numberArray[H10] = hourFormat12() / 10;
  } else {
    numberArray[H1] = hour() % 10;
    numberArray[H10] = hour() / 10;
  }
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

  for ( int i = DIGIT_COUNT - 1 ; i >= 0  ; i -- ) {
    // Blanking
    if (blankTubes) {
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
        fadeState = PHASE_MAX * fadeStepsInternal;
      } else if (fadeState == 0) {
        currNumberArray[i] = numberArray[i];
      }
    }

    if (scrollCounter[i] > 0) {
      scrollCounter[i] = scrollCounter[i] - 1;
      currNumberArray[i] = scrollCounter[i]/cc->scrollSteps;
    }

    tmpDispTypeArray[i] = tmpDispType;
  }

  if (fadeState == 1) {
    fadeState = 0;
    switchTimeBuf = 0;
    for (byte j = 0 ; j < DIGIT_COUNT ; j++) {
      if (scrollCounter[j] == 0) {
        currNumberArray[j] = numberArray[j];
      }
    }
  } else if (fadeState > 0) {
    fadeState--;
    switchTimeBuf = (fadeState / fadeStepsInternal);
  }

  val1 = decodeFromNumberArray( numberArray[H10], 
                                numberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl1,
                                bl2,
                                led1State,
                                led2State);
  val2 = decodeFromNumberArray( numberArray[M10], 
                                numberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl3,
                                bl4,
                                led1State,
                                led2State);
  val3 = decodeFromNumberArray( numberArray[S10], 
                                numberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl5,
                                bl6,
                                indLed1,
                                indLed2);

  // ToDo fading/scrolling
  nextVal1 = decodeFromNumberArray( currNumberArray[H10], 
                                currNumberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl1,
                                bl2,
                                led1State,
                                led2State);
  nextVal2 = decodeFromNumberArray( currNumberArray[M10], 
                                currNumberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl3,
                                bl4,
                                led1State,
                                led2State);
  nextVal3 = decodeFromNumberArray( currNumberArray[S10], 
                                currNumberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl5,
                                bl6,
                                indLed1,
                                indLed2);
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
    debugMsg(message);
  }

  if (request->hasArg("body")) {
    debugMsg("Body found arg");
  }
  if (request->hasParam("body")) {
    debugMsg("Body found param");
  }
  int args = request->args();
  for(int i=0;i<args;i++){
    String message = "ARG[" + request->argName(i) + "]: " + request->arg(i); 
    debugMsg(message);
  }  
}
#endif


// ************************************************************
// Main page handler
// ************************************************************
void mainHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
	debugMsg("Got request");
  #endif
	request->send(SPIFFS, "/web/index.html");
}

// ************************************************************
// Main CSS handler
// ************************************************************
void cssHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
	debugMsg("Got css request");
  #endif
	request->send(SPIFFS, "/web/style.css");
}

void getSummaryDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api summary GET request");
  #endif
  
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
  if (ntpAsync.ntpTimeValid(nowMillis)) {
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

  if (useRTC) {
    root["lastrtctime"] = lastRTCTime;
    root["lastrtcupdate"] = secsToReadableString((nowMillis - lastRTCReadTime)/1000);
  } else {
    root["lastrtctime"] = "RTC not installed";
    root["lastrtcupdate"] = "";
  }

  float ldrPerc = (4095 - ldrValue) / 4095.0 * 100.0;
  root["ldrvalue"] = String(ldrPerc, 2) + "% (" + String(ldrValue) + ")";

  root["status"] = getStatusString();
  root["version"] = SOFTWARE_VERSION;

  response->setLength();
  request->send(response);
}

void getDiagsDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api diagnostics GET request");
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
  root["utcoffset"] = String(gpsManager.getCurrentUTCOffset());

  response->setLength();
  request->send(response);
}

void getConfigDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api config GET request");
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

  root["backlightMode"] = cc->backlightMode;
  root["redCnl"] = cc->redCnl;
  root["grnCnl"] = cc->grnCnl;
  root["bluCnl"] = cc->bluCnl;
  root["cycleSpeed"] = cc->cycleSpeed;
  root["useBLDim"] = cc->useBLDim;
  root["useBLPulse"] = cc->useBLPulse;

  response->setLength();
  request->send(response);
}

void postConfigDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api config POST request");
  #endif

  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {

    if (json.containsKey("hourMode")) {
      int newhourMode = json["hourMode"];
      if (cc->thresholdBright != newhourMode) {
        #ifdef DEBUG_ON
        debugMsg("hourMode before: " + String(cc->hourMode));
        #endif
        cc->thresholdBright = newhourMode;
        #ifdef DEBUG_ON
        debugMsg("Loaded new hourMode: " + String(cc->hourMode));
        #endif
      }
    }

    if (json.containsKey("blankLeading")) {
      int newblankLeading = json["blankLeading"];
      if (cc->blankLeading != newblankLeading) {
        #ifdef DEBUG_ON
        debugMsg("blankLeading before: " + String(cc->blankLeading));
        #endif
        cc->blankLeading = newblankLeading;
        #ifdef DEBUG_ON
        debugMsg("Loaded new blankLeading: " + String(cc->blankLeading));
        #endif
      }
    }

    if (json.containsKey("dateFormat")) {
      int newdateFormat = json["dateFormat"];
      if (cc->dateFormat != newdateFormat) {
        #ifdef DEBUG_ON
        debugMsg("dateFormat before: " + String(cc->dateFormat));
        #endif
        cc->dateFormat = newdateFormat;
        #ifdef DEBUG_ON
        debugMsg("Loaded new dateFormat: " + String(cc->dateFormat));
        #endif
      }
    }

    if (json.containsKey("scrollback")) {
      int newscrollback = json["scrollback"];
      if (cc->scrollback != newscrollback) {
        #ifdef DEBUG_ON
        debugMsg("scrollback before: " + String(cc->scrollback));
        #endif
        cc->scrollback = newscrollback;
        #ifdef DEBUG_ON
        debugMsg("Loaded new scrollback: " + String(cc->scrollback));
        #endif
      }
    }

    if (json.containsKey("scrollSteps")) {
      int newscrollSteps = json["scrollSteps"];
      if (cc->scrollSteps != newscrollSteps) {
        #ifdef DEBUG_ON
        debugMsg("scrollSteps before: " + String(cc->scrollSteps));
        #endif
        cc->scrollSteps = newscrollSteps;
        #ifdef DEBUG_ON
        debugMsg("Loaded new scrollSteps: " + String(cc->scrollSteps));
        #endif
      }
    }

    if (json.containsKey("fade")) {
      int newfade = json["fade"];
      if (cc->fade != newfade) {
        #ifdef DEBUG_ON
        debugMsg("fade before: " + String(cc->fade));
        #endif
        cc->fade = newfade;
        #ifdef DEBUG_ON
        debugMsg("Loaded new fade: " + String(cc->fade));
        #endif
      }
    }

    if (json.containsKey("suppressACP")) {
      int newsuppressACP = json["suppressACP"];
      if (cc->suppressACP != newsuppressACP) {
        #ifdef DEBUG_ON
        debugMsg("suppressACP before: " + String(cc->suppressACP));
        #endif
        cc->suppressACP = newsuppressACP;
        #ifdef DEBUG_ON
        debugMsg("Loaded new suppressACP: " + String(cc->suppressACP));
        #endif
      }
    }

    if (json.containsKey("slotsMode")) {
      int newslotsMode = json["slotsMode"];
      if (cc->slotsMode != newslotsMode) {
        #ifdef DEBUG_ON
        debugMsg("slotsMode before: " + String(cc->slotsMode));
        #endif
        cc->slotsMode = newslotsMode;
        #ifdef DEBUG_ON
        debugMsg("Loaded new slotsMode: " + String(cc->slotsMode));
        #endif
      }
    }

    // ------------------------------------------------------------

    if (json.containsKey("useLDR")) {
      int newUseLDR = json["useLDR"].as<bool>();
      if (cc->useLDR != newUseLDR) {
        #ifdef DEBUG_ON
        debugMsg("useLDR before: " + String(cc->useLDR));
        #endif
        cc->useLDR = newUseLDR;
        #ifdef DEBUG_ON
        debugMsg("Loaded new useLDR: " + String(cc->useLDR));
        #endif
      }
    }

    if (json.containsKey("minDim")) {
      int newMinDim = json["minDim"];
      if (cc->minDim != newMinDim) {
        #ifdef DEBUG_ON
        debugMsg("minDim before: " + String(cc->minDim));
        #endif
        cc->minDim = newMinDim;
        #ifdef DEBUG_ON
        debugMsg("Loaded new minDim: " + String(cc->minDim));
        #endif
      }
    }

    if (json.containsKey("thresholdBright")) {
      int newthresholdBright = json["thresholdBright"];
      if (cc->thresholdBright != newthresholdBright) {
        #ifdef DEBUG_ON
        debugMsg("thresholdBright before: " + String(cc->thresholdBright));
        #endif
        cc->thresholdBright = newthresholdBright;
        #ifdef DEBUG_ON
        debugMsg("Loaded new thresholdBright: " + String(cc->thresholdBright));
        #endif
      }
    }

    if (json.containsKey("sensitivityLDR")) {
      int newsensitivityLDR = json["sensitivityLDR"];
      if (cc->sensitivityLDR != newsensitivityLDR) {
        #ifdef DEBUG_ON
        debugMsg("sensitivityLDR before: " + String(cc->sensitivityLDR));
        #endif
        cc->sensitivityLDR = newsensitivityLDR;
        #ifdef DEBUG_ON
        debugMsg("Loaded new sensitivityLDR: " + String(cc->sensitivityLDR));
        #endif
      }
    }

    if (json.containsKey("useBLDim")) {
      int newUseBLDim = json["useBLDim"].as<bool>();
      if (cc->useBLDim != newUseBLDim) {
        #ifdef DEBUG_ON
        debugMsg("useBLDim before: " + String(cc->useBLDim));
        #endif
        cc->useBLDim = newUseBLDim;
        #ifdef DEBUG_ON
        debugMsg("Loaded new useBLDim: " + String(cc->useBLDim));
        #endif
      }
    }

    if (json.containsKey("useBLPulse")) {
      int newUseBLPulse = json["useBLDim"].as<bool>();
      if (cc->useBLPulse != newUseBLPulse) {
        #ifdef DEBUG_ON
        debugMsg("useBLPulse before: " + String(cc->useBLPulse));
        #endif
        cc->useBLPulse = newUseBLPulse;
        #ifdef DEBUG_ON
        debugMsg("Loaded new useBLPulse: " + String(cc->useBLPulse));
        #endif
      }
    }

    spiffsStorage.saveConfigToSpiffs(cc);
    #ifdef DEBUG_ON
    debugMsg("Saved new config");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsg("Json parse failure: " + String(request->arg("body")));
    #endif
  }

  // Return the updated values
  getConfigDataHandler(request);
}

void getTimeserverDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api timeserver GET request");
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
  debugMsg("Got api timeserver POST request");
  #endif
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    #ifdef DEBUG_ON
    debugMsg("NTP pool before: " + cc->ntpPool);
    #endif
    cc->ntpPool = json["ntpPool"].as<String>();
    #ifdef DEBUG_ON
    debugMsg("Loaded NTP pool: " + cc->ntpPool);
    #endif

    cc->ntpUpdateInterval = json["ntpUpdateInterval"].as<int>();
    #ifdef DEBUG_ON
    debugMsg("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));
    #endif

    cc->tzs = json["tzs"].as<String>();
    #ifdef DEBUG_ON
    debugMsg("Loaded time zone string: " + cc->tzs);
    #endif

    // Now apply the new confog
    ntpAsync.setNtpPool(cc->ntpPool);
    ntpAsync.setUpdateInterval(cc->ntpUpdateInterval);
    ntpAsync.setTZS(cc->tzs);
    #ifdef DEBUG_ON
    debugMsg("Applied new time config");
    #endif

    spiffsStorage.saveConfigToSpiffs(cc);
    #ifdef DEBUG_ON
    debugMsg("Saved new time config");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsg("Json parse failure: " + String(request->arg("body")));
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

void restartHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api restat request");
  #endif
  
  #ifdef DEBUG_ON
  dumpArgs(request);
  #endif

  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"Restart in 1s\"}");
  request->send(response);

  delay(1000);
  ESP.restart();
}

void postWiFiDataHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api wifi POST request");
  #endif
  
  // dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  if (json.success()) {
    String newSSID = json["SSID"].as<String>();
    #ifdef DEBUG_ON
    debugMsg("Received SSID: " + newSSID);
    #endif

    String newPassword = json["password"].as<String>();
    #ifdef DEBUG_ON
    debugMsg("Received password: " + newPassword);
    #endif

  } else {
    #ifdef DEBUG_ON
    debugMsg("Json parse failure: " + String(request->arg("body")));
    #endif
  }

  AsyncWebServerResponse* response = request->beginResponse(200, "text/json", "{\"status\": \"OK\"}");
  request->send(response);
        
}

void getI2CScanHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got I2C scan request");
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
      debugMsg("I2C device found at address 0x" + String(address, HEX));
      #endif
      root["I2C"+String(address)] = "found";
      nDevices++;
    }
    else if (error==4) {
      #ifdef DEBUG_ON
      debugMsg("Unknown error at address 0x" + String(address, HEX));
      #endif
    }    
  }
  if (nDevices == 0) {
    #ifdef DEBUG_ON
    debugMsg("No I2C devices found");
  #endif
  }
  else {
    #ifdef DEBUG_ON
    debugMsg("done");
    #endif
  }

  response->setLength();
  request->send(response);
}

void saveStatsHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got save stats request");
  #endif

  spiffsStorage.saveStatsToSpiffs(cs);
  
  request->send(200, "text/json", "{\"status\": \"Stats saved\"}");
}

void resetWifiHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got utils RESET request");
  #endif
  WiFi.disconnect();
  request->send(200, "text/json", "{\"status\": \"WiFi was reset\"}");
}

void getCredentialsHandler(AsyncWebServerRequest *request) {
  #ifdef DEBUG_ON
  debugMsg("Got api wifi credentials request");
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
