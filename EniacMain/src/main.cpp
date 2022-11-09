#include "Defs.h"
#include "Globals.h"
#include "utilities.h"
#include "WiFi.h"
#include "TimerManager.h"
#include "LDRManager.h"
#include "GPSManager.h"
#include "BlankingManager.h"
#include "TZManager.h"
#include "RTCManager.h"
#include "NTPManager.h"
#include "DebugManager.h"
#include "MenuManager.h"
#include "WiFiManager.h"
#include "OutputManager.h"
#include "CountdownManager.h"

void setup()
{
  // Show that we booted - useful for remote debugging
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < 10 ; i++) {
    digitalWrite(LED_PIN, (i % 2) == 0);   
    delay(25);   
  }

  // -------------------------------------------------------------------------

  // for reliable startup with GPS connected, change line 200 of esp32-hal-uart.c from
  //
  //      uartFlush(uart);
  //  to
  //      uartFlushTxOnly(uart, false);
  //
  // Which causes the receive buffer to be flushed 

  Serial.begin(SERIAL_BAUD_RATE);

  #ifdef DEBUG_ON
  // Debug for 10 minutes
  debugManager.setDebugAutoOff(600);
  #endif

  // -------------------------------------------------------------------------

  debugMsgMain("Start up GPIOs");
  pinMode(CLKPin, OUTPUT);
  pinMode(DATA1Pin, OUTPUT);
  pinMode(LATCH1Pin, OUTPUT);
  pinMode(DATA2Pin, OUTPUT);
  pinMode(LATCH2Pin, OUTPUT);
  pinMode(DATA3Pin, OUTPUT);
  pinMode(LATCH3Pin, OUTPUT);

  pinMode(BLANKPin, OUTPUT);
  pinMode(PPSPin, OUTPUT);
  digitalWrite(PPSPin, LOW);
  
  pinMode(BTN1Pin, INPUT_PULLUP);
  pinMode(BTN2Pin, INPUT_PULLUP);
  pinMode(BTN3Pin, INPUT_PULLUP);

  // -------------------------------------------------------------------------

  nowMillis = millis();

  // -------------------------------------------------------------------------

  debugMsgMain("Start up Timers");

  // Starts the display and the status LED flashing
  startTimers();

  // -------------------------------------------------------------------------
  // Startup test
  ldrManager.setUpPWM();
  ldrManager.setLDRValueToMax();
  outputManager.setOutputMode(diagsMode);

  for (int i = 0 ; i <= 20 ; i++) {
    outputManager.loadNumberArraySameValue(i%10);
    outputManager.allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    outputManager.outputDisplay();
    delay(100);
  }

  // -------------------------------------------------------------------------
  
  // Default pins SDA 21, SCL 22 Frequency 400kHz 
  debugMsgMain("Start up I2C...");
  Wire.begin(SDAint, SCLint, 400000L);

  // -------------------------------------------------------------------------
  
  debugMsgMain("Starting OLED");
  oled.setUp();
  oled.clearDisplay();
  menuManager.flashMenuMessage(CLOCK_MENU_TITLE, "Starting");

  // -------------------------------------------------------------------------

  #ifdef FEATURE_BACKLIGHTS
  debugMsgMain("Start up neopixels");
  ledManager.setUp();
  ledManager.setLDRRange(LDR_VALUE_MAX);
  #endif

  // -------------------------------------------------------------------------
  
  debugMsgMain("Start up SPIFFS");

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    debugMsgMain("An Error has occurred while mounting SPIFFS");
    return;
  }

  bool statsLoaded = spiffsStorage.getStatsFromSpiffs();

  if (!statsLoaded) {
    debugMsgMain("SPIFFS storage: read stats failed");
    spiffsStorage.saveStatsToSpiffs();
  }

  bool configloaded = spiffsStorage.getConfigFromSpiffs();

  if (configloaded) {
    ntpManager.setNtpPool(cc->ntpPool);
    ntpManager.setUpdateInterval(cc->ntpUpdateInterval);
  } else {
    debugMsgMain("SPIFFS storage: read config failed - do factory reset");
    resetOptions();
  }

  // -------------------------------------------------------------------------

  debugMsgMain("Initialising WiFi");
  wifiManager.setUpWiFi();

  if (cc->WifiOnAtStart && wifiManager.wifiCredentialsReceived()) {
    debugMsgMain("Starting WiFi");
    menuManager.flashMenuMessage("WiFi", "Starting WiFi");

    debugMsgMain("Connecting to previous AP");
    
    wifiManager.connectToLastAP();
  } else {
    if (!cc->WifiOnAtStart) {
      debugMsgMain("Skipping connect to previous AP - told not to");
    } else if (!wifiManager.wifiCredentialsReceived()) {
      debugMsgMain("Skipping connect to previous AP - no AP defined");
    }
  }

  // -------------------------------------------------------------------------
  
  debugMsgMain("Start up TZM" );
  tzManager.setTZS(cc->tzs);

  // -------------------------------------------------------------------------
  
  debugMsgMain("Start up RTC...");
  if (rtcManager.testRTCTimeProvider()) {
    debugMsgMain("RTC found");

    // first time let's us figure out the UTC offset, it should be right within
    // the bounds of DST variation
    time_t rtctime = rtcManager.getRTCTimeAsTimeT();
    tzManager.calculateCurrentOffsetFromTimeT(rtctime);

    debugMsgMain("Recalculate offset based on recovered datetime");

    // Second time sets the time based on the calculated DST
    rtctime = rtcManager.getRTCTimeAsTimeT();
    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtctime);
    tzManager.calculateCurrentOffsetFromTimeT(rtctime);
  } else {
    debugMsgMain("RTC NOT found");
  }

  // Start showing the time now that we have something to say
  outputManager.setOutputMode(timeMode);

  // -------------------------------------------------------------------------
  
  debugMsgMain("Initialising NTP" );
  NewTimeCallback ntcb = newTimeUpdateReceived;
  ntpManager.setNewTimeCallback(ntcb);

  // -------------------------------------------------------------------------
  
  debugMsgMain("Initialising GPS" );
  gpsManager.setUp();

  // -------------------------------------------------------------------------
  
  debugMsgMain("Start up Menu Manager...");
  menuManager.setupMenuManager();

  // -------------------------------------------------------------------------
  
  debugMsgMain("Start up LDR...");
  // Not managing sensorSmoothCountLDR yet
  cc->sensorSmoothCountLDR = SENSOR_SMOOTH_READINGS_DEFAULT;
  ldrManager.setUp();

  // -------------------------------------------------------------------------
  
  debugMsgMain("Start up Blanking");
  blankingManager.begin();

  // -------------------------------------------------------------------------
  
  #ifdef COUNTDOWN
  debugMsgMain("Start up countdown manager");
  countdownManager.begin();
  #endif

  // -------------------------------------------------------------------------

  oled.setUp();
  oled.clearDisplay();
  menuManager.flashMenuMessage(CLOCK_MENU_TITLE, "Welcome to the\nNixie Chronometer\n" + String(SOFTWARE_VERSION));
  delay(2000);

  // -------------------------------------------------------------------------

  debugMsgMain("Start up WDT...");
  enableWatchdog();
  // -------------------------------------------------------------------------
  
  #ifdef SLAVE_OUTPUT
  debugMsgMain("Start up Slave");
  slaveManager.testSlave();
  #endif

}

// ************************************************************
// Set the seconds tick led(s) and the back lights
// ************************************************************
void setLeds()
{
  secsDeltaAbs = (nowMillis - lastMillis) % 1000;
  upOrDown = (second() % 2) == 0;
  
  #ifdef FEATURE_BACKLIGHTS
  unsigned int secsDelta;
  if (upOrDown) {
    secsDelta = secsDeltaAbs;
  } else {
    secsDelta = 1000 - secsDeltaAbs;
  }

  // output the backlight/underlight LEDs
  ledManager.setPulseValue(secsDelta);  
  ledManager.processLedStatus();
  #endif
}

#if defined(DIGIT_DIAGNOSTICS) && defined(FEATURE_BACKLIGHTS)
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
    ldrManager.setLDRValueToMax();
    int rawEncPos = menuManager.getCurrentEncoderPos()/2;
    while (rawEncPos < 0) rawEncPos+=60; 
    int burnVal = rawEncPos % 60;
    outputManager.loadNumberArrayBurn(burnVal);
  }
  #endif

  // -------------------------------------------------------------------------------
  
  // Dim except when we are in ACP mode
  if (outputManager.getOutputMode() == acpMode) {
    ldrManager.setLDRValueToMax();
  } else {
    ldrManager.resetFixedLDRValue();
    ldrManager.getDimmingFromLDR();
    ldrValue = ldrManager.getLDRValue();
    #ifdef FEATURE_BACKLIGHTS
    ledManager.setLDRValue(ldrValue);
    #endif
  }

  // -------------------------------------------------------------------------------
  
#if defined(DIGIT_DIAGNOSTICS) && defined(FEATURE_BACKLIGHTS)
  // output the backlight/underlight LEDs
  if (cc->diagsMode > 0) {
    setLedsDiags();
  } else {
    setLeds();
  }
  #else
  setLeds();
  #endif

  // -------------------------------------------------------------------------------
  
  outputManager.outputDisplay();

  // -------------------------------------------------------------------------------
  
  #ifdef FEATURE_BLINKENLIGHTS
  blinkenlightsManager.updateBlinkenlights();
  #endif

  // -------------------------------------------------------------------------------
  
  menuManager.menuLoop();

  // -------------------------------------------------------------------------------

  wifiManager.manageDNSInOpenAP();
}

// ************************************************************
// Called once per second. Trigger all the things that do
// Not need processing continuously multiple times per second
// ************************************************************
void performOncePerSecondProcessing() {
  lastMillis = nowMillis;

  // See if it is time for a new NTP update
  if (ntpManager.getNextUpdate() < 0 && WiFi.isConnected()) {
    ntpManager.getTimeFromNTP();
  }

  // -------------------------------------------------------------------------------
  
  countdownManager.calculateCountdown();

  // -------------------------------------------------------------------------------
  
  outputManager.setOutputModeOncePerSecond();

  // -------------------------------------------------------------------------------
  
  // Maintain the LED next to the controller
  if (WiFi.status() == WL_CONNECTED) {
    setLedFlashType(0);
  } else {
    setLedFlashType(1);
  }

  // -------------------------------------------------------------------------------
  
  #ifdef COG_CRANK_OUTPUT
  if (cogCrankSecsLeft > 0) {
    cogCrankSecsLeft--;
    if (cogCrankSecsLeft == 0) {
      digitalWrite(PPSPin, LOW);
      debugMsgMain("Aux output OFF");
    }
  }
  #endif

  // -------------------------------------------------------------------------------
  
  #ifdef FEATURE_BACKLIGHTS
  ledManager.recalculateVariables();
  #endif

  // -------------------------------------------------------------------------------
  
  // Feed the GPS parser
  while (Serial.available()) {
    char c = Serial.read();
    gpsManager.parseNMEAMsg(c);
  }

  // -------------------------------------------------------------------------------

  #if defined(SLAVE_OUTPUT)
  slaveManager.updateOncePerSecond();
  #endif

  // ------------------------------ switch handling -----------------------------------
  
  // Switch 1 has various meanings

  // Countdown mode
  // If we are in Countdown mode, it switches the display back to "normal"
  // when countdown is finished, it controls the slave output

  // 
  #define SW1_NONE                0
  #define SW1_COUNTDOWN_INHIBIT   1
  #define SW1_SLAVE_INHIBIT       2
  #define SW1_MIN_DIM             3

  byte switch1Meaning = SW1_NONE;

  #if defined(COUNTDOWN)
  // If we are in countdown, the switch can turn it off
  if (countdownManager.getCountdownActiveInternal()) {
    switch1Meaning = SW1_COUNTDOWN_INHIBIT;
  } else {
    #if defined(SLAVE_OUTPUT)
    switch1Meaning = SW1_SLAVE_INHIBIT;
    #else
    switch1Meaning = SW1_MIN_DIM;
    #endif
  }
  #else
    #if defined(SLAVE_OUTPUT)
    switch1Meaning = SW1_SLAVE_INHIBIT;
    #else
    switch1Meaning = SW1_MIN_DIM;
    #endif
  #endif

  #ifdef NORMAL_SWITCHES
  bool BTNOnstate = LOW;
  #endif

  #ifdef INVERT_SWITCHES
  bool BTNOnstate = HIGH;
  #endif

  switch(switch1Meaning) {
    case SW1_NONE: {
      // nothing
      break;
    }
    case SW1_COUNTDOWN_INHIBIT: {
      if (digitalRead(BTN1Pin) == BTNOnstate) {
        if (!countdownManager.getCountdownInhibit()) {
          countdownManager.setCountdownInhibit(true);
          debugMsgMain("Set inhibit countdown via switch");
        }
      } else {
        if (countdownManager.getCountdownInhibit()) {
          countdownManager.setCountdownInhibit(false);
          debugMsgMain("Remove inhibit countdown via switch");
        }
      }
      break;
    }
    case SW1_SLAVE_INHIBIT: {
      slaveManager.setSlaveEnabled(digitalRead(BTN1Pin) == !BTNOnstate);
      break;
    }
    case SW1_MIN_DIM: {
      // The switch "on" imposes min dimming
      if ((digitalRead(BTN1Pin) == BTNOnstate) && !ldrManager.getIsFixedLDRValue()) {
        ldrManager.setLDRValueToMin();
      }
      // Switch off resets it
      if ((digitalRead(BTN1Pin) == !BTNOnstate) && ldrManager.getIsFixedLDRValue()) {
        ldrManager.resetFixedLDRValue();
      }
      break;
    }
  }

  // Switch 2 just blanks the LEDs
  blankingManager.setCurrentLEDBlankingOverride(digitalRead(BTN2Pin) == BTNOnstate);

  // -------------------------------------------------------------------------------
  
  blankingManager.getBlankingStatus(weekday(), hour());

  menuManager.menuOncePerSecond();

  outputManager.triggerStunts();

  triggerOnePulsePerSec();

  debugManager.debugAutoOffCheck();

  feedWatchdog();
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  debugMsgMain("---> OncePerMinuteProcessing");
  if (WiFi.isConnected()) {
    debugMsgMain("Next update in: " + String(ntpManager.getNextUpdate()));
  }

  // Usage stats
  cs->uptimeMins++;

  if (!blankingManager.getCurrentBlankingStatus()) {
    cs->tubeOnTimeMins++;
  }

  // Update the RTC time
  tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtcManager.getRTCTimeAsTimeT());

  // manage the primary source - it might have changed
  tzManager.getPrimaryTimeSource();

  slaveManager.updateOncePerMinute();
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  debugMsgMain("---> OncePerHourProcessing");

  menuManager.menuOncePerHour();
  
  tzManager.setUTCTimeFromTimeSourceHourly();
  tzManager.calculateCurrentOffsetFromTimeT(tzManager.getRawUTCTimeFromTimeSource(tzManager.getPrimaryTimeSource()));

  rtcManager.testRTCTimeProvider();

  #ifdef COG_CRANK_OUTPUT
  // Don't crank if we're blanked or we're configured not to
  debugMsgMain("Crank time:" + String(cc->outputOnTime));
  if (!blankingManager.getCurrentBlankingStatus() && (cc->outputOnTime > 0)) {
    cogCrankSecsLeft = cc->outputOnTime;
    digitalWrite(PPSPin, HIGH);
    debugMsgMain("Aux output  ON");
  }
  #endif
}

// ************************************************************
// Called once per day
// ************************************************************
void performOncePerDayProcessing() {
  debugMsgMain("---> OncePerDayProcessing");

  #ifdef FEATURE_BACKLIGHTS
  ledManager.setDayOfWeek(weekday() - 1);
  #endif

  spiffsStorage.saveStatsToSpiffs();
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

