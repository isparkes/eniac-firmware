#include "defs.h"
#include "globals.h"
#include "utilities.h"
#include "WiFi.h"
#include "OLED.h"
#include "TimerManager.h"
#include "LDRManager.h"
#include "LEDManager.h"
#include "MyLib.h"
#include "GPSManager.h"
#include "BlankingManager.h"
#include "TZManager.h"
#include "RTCManager.h"
#include "BlinkenlightsManager.h"
#include "NTPManager.h"
#include "DebugManager.h"
#include "WiFiManager.h"
#include "MenuManager.h"
#include "OutputManager.h"

void debugMsg(String message) {
  #ifdef DEBUG_ON
  debugManager.debugMsg("[LNC]: " + message);
  #endif
}

void debugMsgCont(String message) {
  #ifdef DEBUG_ON
  debugManager.debugMsgCont("[LNC]: " + message);
  #endif
}

void setup()
{
  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up GPS/Serial...");
  #endif
  Serial.begin(115200);

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg((("Start up GPIOs")));
  #endif
  pinMode(LED_PIN, OUTPUT);

  pinMode(CLKPin, OUTPUT);
  pinMode(DATA1Pin, OUTPUT);
  pinMode(LATCH1Pin, OUTPUT);
  pinMode(DATA2Pin, OUTPUT);
  pinMode(LATCH2Pin, OUTPUT);
  pinMode(DATA3Pin, OUTPUT);
  pinMode(LATCH3Pin, OUTPUT);

  // make sure no ghosts are displayed
  shiftOut24S(0);
  shiftOut24M(0);
  shiftOut24H(0);

  pinMode(BLANKPin, OUTPUT);
  pinMode(PPSPin, OUTPUT);
  
  pinMode(BTN1Pin, INPUT_PULLUP);
  pinMode(BTN2Pin, INPUT_PULLUP);
  pinMode(BTN3Pin, INPUT_PULLUP);

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up Timers" );
  #endif
  // Starts the display and the status LED flashing
  startTimers();

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Starting OLED");
  #endif
  oled.setUp();
  oled.clearDisplay();

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up neopixels");
  #endif
  ledManager.setUp();
  ledManager.setLDRRange(LDR_VALUE_MAX);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up SPIFFS");
  #endif

  // define the debug callback
  DebugCallback dbcb = debugManagerLink;

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    #ifdef DEBUG_ON
    debugMsg("An Error has occurred while mounting SPIFFS");
    #endif
    return;
  }

  #ifdef DEBUG_ON
  debugMsg("Startup SPIFFS storage");
  spiffsStorage.setDebugCallback(dbcb);
  spiffsStorage.setDebugOutput(true);
  #endif
  bool statsLoaded = spiffsStorage.getStatsFromSpiffs(cs);

  if (!statsLoaded) {
    #ifdef DEBUG_ON
    debugMsg("SPIFFS storage: read stats failed");
    #endif
    spiffsStorage.saveStatsToSpiffs(cs);
  }

  bool configloaded = spiffsStorage.getConfigFromSpiffs(cc);

  if (configloaded) {
    ntpManager.setNtpPool(cc->ntpPool);
    ntpManager.setUpdateInterval(cc->ntpUpdateInterval);
  } else {
    #ifdef DEBUG_ON
    debugMsg("SPIFFS storage: read config failed - do factory reset");
    #endif
    resetOptions();
  }

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Initialising WiFi");
  #endif
  setUpWiFi();

  if (cc->wifiOnAtStart && wifiCredentialsReceived()) {
    #ifdef DEBUG_ON
    debugMsg("Starting WiFi");
    #endif
    oled.showScrollingMessage("Starting WiFi");

    #ifdef DEBUG_ON
    debugMsg("Connecting to previous AP");
    #endif
    
    connectToLastAP();
  } else {
    if (!cc->wifiOnAtStart) {
      #ifdef DEBUG_ON
      debugMsg("Skipping connent to previous AP - told not to");
      #endif
    } else if (!wifiCredentialsReceived()) {
      #ifdef DEBUG_ON
      debugMsg("Skipping connent to previous AP - no AP defined");
      #endif
    }
  }

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up TZM" );
  #endif

  tzManager.setTZS(cc->tzs);
  tzManager.setDebugCallback(dbcb);
  tzManager.setDebugOutput(true);
  tzManager.calculateCurrentOffsetFromTimeT();

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up NTP" );

  ntpManager.setDebugCallback(dbcb);
  ntpManager.setDebugOutput(true);
  #endif

  NewTimeCallback ntcb = newTimeUpdateReceived;
  ntpManager.setNewTimeCallback(ntcb);

  // -------------------------------------------------------------------------
  
  // Default pins SDA 21, SCL 22 Frequency 400kHz 
  #ifdef DEBUG_ON
  debugMsg("Start up I2C...");
  #endif
  Wire.begin(SDAint, SCLint, 400000L);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up RTC...");
  #endif
  rtcManager.setDebugCallback(dbcb);
  rtcManager.setDebugOutput(true);

  if (rtcManager.testRTCTimeProvider()) {
    time_t rtctime = rtcManager.getRTCTimeAsTimeT();
    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtctime);
    #ifdef DEBUG_ON
    debugMsg("RTC found");
    debugMsg("Recovered time: " + String(rtctime));
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsg("RTC NOT found");
    #endif
  }

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up Menu Manager...");
  #endif
  #ifdef DEBUG_ON
//  encoderManager.setDebugCallback(dbcb);
//  encoderManager.setDebugOutput(false);
  #endif

  setupMenuManager();

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up LDR...");
  #endif
  // Not managing sensorSmoothCountLDR yet
  cc->sensorSmoothCountLDR = SENSOR_SMOOTH_READINGS_DEFAULT;
  #ifdef DEBUG_ON
  ldrManager.setDebugOutput(true);
  ldrManager.setDebugCallback(dbcb);
  #endif
  ldrManager.setUp();
  ldrManager.setDebugOutput(false);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up Blanking");
  #endif
  blankingManager.begin();

  // -------------------------------------------------------------------------

  MyLib.begin();
  MyLib.doStuff();

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up WDT...");
  #endif
  enableWatchdog();
}

// ************************************************************
// Set the seconds tick led(s) and the back lights
// ************************************************************
void setLeds()
{
  unsigned int secsDelta;
  int secsDeltaAbs = (nowMillis - lastMillis);

  bool upOrDown = (second() % 2) == 0;
  
  if (upOrDown) {
    secsDelta = (nowMillis - lastMillis);
  } else {
    secsDelta = 1000 - (nowMillis - lastMillis);
  }

  // --------------------------------------- separators --------------------------------------
  
  switch (cc->ledMode) {
    case LED_RAILROAD:
      {
        if (upOrDown) {
          led1State = true;
          led2State = false;
        } else {
          led1State = false;
          led2State = true;
        }
        break;
      }
    case LED_BLINK_SLOW:
      {
        if (upOrDown) {
          led1State = true;
          led2State = true;
        } else {
          led1State = false;
          led2State = false;
        }
        break;
      }
    case LED_BLINK_FAST:
      {
        if (secsDeltaAbs < 500) {
          led1State = true;
          led2State = true;
        } else {
          led1State = false;
          led2State = false;
        }
        break;
      }
    case LED_BLINK_DBL:
      {
        if ((secsDeltaAbs < 100) || ((secsDeltaAbs > 200) && (secsDeltaAbs < 300))) {
          led1State = true;
          led2State = true;
        } else {
          led1State = false;
          led2State = false;
        }
        break;
      }
    case LED_ON:
      {
          led1State = true;
          led2State = true;
        break;
      }
    case LED_OFF:
      {
          led1State = false;
          led2State = false;
        break;
      }
  }

  // output the backlight/underlight LEDs
  ledManager.setPulseValue(secsDelta);  
  ledManager.processLedStatus();
}

#ifdef DIGIT_DIAGNOSTICS
// ************************************************************
// Set the seconds tick led(s) and the back lights for diags
// ************************************************************
void setLedsDiags()
{
  ledManager.setTestValue(second() % 10);
}
#endif

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  lastMillis = nowMillis;

  // If we have gained wifi connectivity but didnt initialise yet
  if (!wifiServicesWereInitalised) {
    if (WiFi.isConnected()) {
      startWiFiServices();
    }
  }
  // See if it is time for a new NTP update
  if (ntpManager.getNextUpdate() < 0 && WiFi.isConnected()) {
    ntpManager.getTimeFromNTP();
  }

  // Maintain the LED next to the controller
  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected) {
    setLedFlashType(0);
  } else {
    setLedFlashType(1);
  }

  if (oledTimeout > 0 && configTimeout == 0) {
    oled.showStatusLine();
    // Show the info menu
    char time_c[11];
    sprintf(time_c, "%02d:%02d:%02d", hour(), minute(), second());
    oled.setTimeString(String(time_c));

    oled.setWiFiStatus(connected);
    oled.setNTPStatus(ntpManager.ntpTimeValid());
    oled.setGStatus(gpsManager.getGPSTimeValid());
    oled.setBlankStatus(false);
    if (digitalRead(PIRPin) == false) {
      oled.setPIRInstalled(true);  
    }
    oled.setPIRStatus(digitalRead(PIRPin));
    oled.setYStatus(digitalRead(BTN2Pin) == LOW);

    oled.clearScrollingMessage();
    if (WiFi.isConnected()) {
      oled.showScrollingMessage("IP: " + WiFi.localIP().toString());
      oled.showScrollingMessage(String(WiFi.getHostname()) + ".local");
      oled.showScrollingMessage(String(WiFi.SSID()));
    } else {
      oled.showScrollingMessage("WiFi not connected");
    }
  }

  // ************************************************************
  // send time display to the drivers
  // ************************************************************
  // ToDo move into output manager
  indLed1 = (second() % 2 == 0);
  indLed2 = (second() % 2 == 1);

  if (digitalRead(BTN2Pin)) {
    blinkenlightsManager.setBlinkenlightsMode(MODE_CHASE);  
  } else {
    blinkenlightsManager.setBlinkenlightsMode(MODE_STATUS);  
  }

  blinkenlightsManager.updateBlinkenlights();

#ifdef DIGIT_DIAGNOSTICS
  if (cc->diagsMode == DIGIT_DIAGS_MODE_NONE) {
    loadNumberArrayTime();
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_FAST) {
    loadNumberArraySameValue(second());
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_SLOW) {
    loadNumberArraySameValue(minute());
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_ENCODER) {
    int burnVal = rotaryEncoder.encoder0Pos;
    if (burnVal < 0) {
      burnVal = -burnVal;
    }
    byte digit = burnVal%6;
    byte value = burnVal/10;
    loadNumberArrayBurn(digit, value);
  }
#else
    loadNumberArrayTime();
#endif

  // Deal with turning off config mode and the OLED
  if (configTimeout > 0) {
    configTimeout--;
    if (configTimeout == 0) {
      oled.clearDisplay();
    }
  }

  if (oledTimeout > 0) {
    oledTimeout--;
    if (oledTimeout == 0) {
      oled.blankDisplay();
      debugMsg("OLED: OFF");
    }
  }

  blankingManager.getBlankingStatus(weekday(), hour());

  ledManager.setBlanked(blankingManager.getCurrentBlankLEDs());

  ledManager.recalculateVariables();

  // Feed the GPS parser
  while (Serial.available()) {
    char c = Serial.read();
    gpsManager.parseNMEAMsg(c);
  }

  // Trigger 1PPS signal
  triggerTimer2();

  feedWatchdog();
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerMinuteProcessing");
  if (WiFi.isConnected()) {
    debugMsg("Next update in: " + String(ntpManager.getNextUpdate()));
  }
  #endif

  // Usage stats
  cs->uptimeMins++;

  if (!blankingManager.getCurrentBlankingStatus()) {
    cs->tubeOnTimeMins++;
  }

  // Update the RTC time
  tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtcManager.getRTCTimeAsTimeT());

  // manage the primary source - it might have changed
  tzManager.getPrimaryTimeSource();

  // #ifdef DEBUG_ON
  // tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_GPS, nowMillis);
  // tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_NTP, nowMillis);
  // tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis);
  // tzManager.getLocalTimeFromTimeSource(TIME_SOURCE_INT, nowMillis);
  // #endif
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerHourProcessing");
  #endif
  oled.setAMStatus(isAM());
  tzManager.setUTCTimeFromTimeSourceHourly();
  tzManager.calculateCurrentOffsetFromTimeT();

  rtcManager.testRTCTimeProvider();
}

// ************************************************************
// Called once per day
// ************************************************************
void performOncePerDayProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerDayProcessing");
  #endif

  ledManager.setDayOfWeek(weekday() - 1);

  spiffsStorage.saveStatsToSpiffs(cs);
}

// ************************************************************
// Main loop
// ************************************************************
void loop()
{
  nowMillis = millis();

  if (lastMillis > nowMillis) {
    // rollover
    lastMillis = 0;
  }

  // -------------------------------------------------------------------------------

  if (lastSecond != second()) {
    lastSecond = second();
    performOncePerSecondProcessing();

    if ((second() == 0) && (!triggeredThisSec)) {
      if ((minute() == 0)) {
        if (hour() == 0) {
          performOncePerDayProcessing();
        }
        performOncePerHourProcessing();
      }
      performOncePerMinuteProcessing();
    }

    // Make sure we don't call multiple times
    triggeredThisSec = true;
    if ((second() > 0) && triggeredThisSec) {
      triggeredThisSec = false;
    }
  }

  // -------------------------------------------------------------------------------

  ldrManager.getDimmingFromLDR();
  ldrValue = ldrManager.getLDRValue();
  ledManager.setLDRValue(ldrValue);

  // -------------------------------------------------------------------------------
  
  outputDisplay();

  #ifdef DIGIT_DIAGNOSTICS
  // output the backlight/underlight LEDs
  if (cc->diagsMode > 0) {
    setLedsDiags();
  } else {
    setLeds();
  }
  #else
  setLeds();
  #endif

  menuLoop();

  delay(10);
}

