#include "defs.h"
#include "globals.h"
#include "utilities.h"
#include "WiFi.h"
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
#include "MenuManager.h"
#include "WiFiManager.h"
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
  debugMsg("Start up Serial...");
  Serial.begin(SERIAL_BAUD_RATE);
  #endif

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

  nowMillis = millis();

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up output manager" );
  #endif

  // define the debug callback
  DebugCallback dbcb = debugManagerLink;

  // Starts the display and the status LED flashing
  outputManager.setDebugCallback(dbcb);
  outputManager.setDebugOutput(true);

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up Timers" );
  #endif
  // Starts the display and the status LED flashing
  startTimers();

  // -------------------------------------------------------------------------
  // // Startup test
  // for (int i = 0 ; i <= 10 ; i++) {
  //   loadNumberArraySameValue(i);
  //   delay(500);
  // }

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

  if (cc->WifiOnAtStart && wifiCredentialsReceived()) {
    #ifdef DEBUG_ON
    debugMsg("Starting WiFi");
    #endif
    oled.showScrollingMessage("Starting WiFi");

    #ifdef DEBUG_ON
    debugMsg("Connecting to previous AP");
    #endif
    
    connectToLastAP();
  } else {
    if (!cc->WifiOnAtStart) {
      #ifdef DEBUG_ON
      debugMsg("Skipping connect to previous AP - told not to");
      #endif
    } else if (!wifiCredentialsReceived()) {
      #ifdef DEBUG_ON
      debugMsg("Skipping connect to previous AP - no AP defined");
      #endif
    }
  }

  // -------------------------------------------------------------------------
  
  // Default pins SDA 21, SCL 22 Frequency 400kHz 
  #ifdef DEBUG_ON
  debugMsg("Start up I2C...");
  #endif
  Wire.begin(SDAint, SCLint, 400000L);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up TZM" );
  #endif

  tzManager.setTZS(cc->tzs);
  tzManager.setDebugCallback(dbcb);
  tzManager.setDebugOutput(true);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up RTC...");
  #endif
  rtcManager.setDebugCallback(dbcb);
  rtcManager.setDebugOutput(true);

  if (rtcManager.testRTCTimeProvider()) {
    #ifdef DEBUG_ON
    debugMsg("RTC found");
    #endif

    // first time let's us figure out the UTC offset
    time_t rtctime = rtcManager.getRTCTimeAsTimeT();
    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtctime);
    tzManager.calculateCurrentOffsetFromTimeT();

    // Second time sets the time
    rtctime = rtcManager.getRTCTimeAsTimeT();
    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtctime);
    tzManager.calculateCurrentOffsetFromTimeT();
  } else {
    #ifdef DEBUG_ON
    debugMsg("RTC NOT found");
    #endif
  }

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Initialising NTP" );

  ntpManager.setDebugCallback(dbcb);
  ntpManager.setDebugOutput(true);
  #endif

  NewTimeCallback ntcb = newTimeUpdateReceived;
  ntpManager.setNewTimeCallback(ntcb);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Initialising GPS" );

  gpsManager.setDebugCallback(dbcb);
  gpsManager.setDebugOutput(true);
  #endif

  gpsManager.setUp();

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

  flashMenuMessage("ENIAC", "Welcome to the ENIAC\nNixie clock!\n" + String(SOFTWARE_VERSION));
  delay(2000);

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
// Called every 10mS or so
// ************************************************************
void performOncePerLoop() {
  // -------------------------------------------------------------------------------

  #ifdef DIGIT_DIAGNOSTICS
  if (cc->diagsMode == DIGIT_DIAGS_MODE_ENCODER) {
    ldrValue = ldrManager.getMaxLDRValue();
    int rawEncPos = getCurrentEncoderPos()/2;
    while (rawEncPos < 0) rawEncPos+=60; 
    int burnVal = rawEncPos % 60;
    outputManager.loadNumberArrayBurn(burnVal);
  }
  #endif

  // -------------------------------------------------------------------------------
  
  // Dim except when we are in ACP mode
  if (outputManager.getOutputMode() == acpMode) {
    ldrValue = ldrManager.getMaxLDRValue();
  } else {
    ldrManager.getDimmingFromLDR();
    ldrValue = ldrManager.getLDRValue();
    ledManager.setLDRValue(ldrValue);
  }

  // -------------------------------------------------------------------------------
  
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

  outputManager.outputDisplay();

  menuLoop();
}

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  lastMillis = nowMillis;

  // See if it is time for a new NTP update
  if (ntpManager.getNextUpdate() < 0 && WiFi.isConnected()) {
    ntpManager.getTimeFromNTP();
  }

  #ifdef DIGIT_DIAGNOSTICS
  if (cc->diagsMode == DIGIT_DIAGS_MODE_NONE) {
    if (outputManager.getOutputMode() == timeMode) {
      outputManager.allNormal(APPLY_LEAD_0_BLANK);
      outputManager.loadNumberArrayTime();
    }
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_FAST) {
    outputManager.loadNumberArraySameValue(second());
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_SLOW) {
    outputManager.loadNumberArraySameValue(minute());
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_ENCODER) {
    int rawEncPos = getCurrentEncoderPos()/2;
    while (rawEncPos < 0) rawEncPos+=60; 
    #ifdef DEBUG_ON
    // int burnVal = rawEncPos % 60;
    // debugMsg("DIGIT BURN Value: " + String(burnVal));
    // debugMsg("-> Val: " + String(burnVal % 10));
    // debugMsg("-> Dig: " + String(burnVal / 10));
    #endif
  }
  #else
  if (outputManager.getOutputMode() == timeMode) {
    outputManager.allNormal(APPLY_LEAD_0_BLANK);
    outputManager.loadNumberArrayTime();
  }
  #endif

  // Maintain the LED next to the controller
  if (WiFi.status() == WL_CONNECTED) {
    setLedFlashType(0);
  } else {
    setLedFlashType(1);
  }

  menuOncePerSecond();

  // ************************************************************
  // send time display to the drivers
  // ************************************************************
  // ToDo move into output manager
  indLed1 = (second() % 2 == 0);
  indLed2 = (second() % 2 == 1);

  blinkenlightsManager.updateBlinkenlights();

  countdownMenuTimeouts();

  blankingManager.getBlankingStatus(weekday(), hour());

  ledManager.setBlanked(blankingManager.getCurrentBlankLEDs());

  ledManager.recalculateVariables();

  // Feed the GPS parser
  while (Serial.available()) {
    char c = Serial.read();
    gpsManager.parseNMEAMsg(c);
  }

  outputManager.triggerStunts();

  triggerOnePulsePerSec();

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
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerHourProcessing");
  #endif

  menuOncePerHour();
  
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

  performOncePerLoop();

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

  delay(10);
}

