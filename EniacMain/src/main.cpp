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
#ifdef FEATURE_MENU
#include "MenuManager.h"
#endif
#include "WiFiManager.h"
#include "OutputManager.h"
#include "CountdownManager.h"

#include "QuoteManager.h"

void setup()
{
  // Show that we booted - useful for remote debugging
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < 10 ; i++) {
    digitalWrite(LED_PIN, (i % 2) == 0);   
    delay(25);   
  }

  // -------------------------------------------------------------------------
  // for reliable startup with GPS connected on older versions of the SDK, you
  // might need to change the uart initialisation. Older versions you have to 
  // change line 200 of esp32-hal-uart.c from
  //
  //      uartFlush(uart);
  //  to
  //      uartFlushTxOnly(uart, false);
  //
  // Which causes the receive buffer NOT to be flushed
  // This has been fixed in 6.5.0 
  Serial.begin(SERIAL_BAUD_RATE);

  #ifdef DEBUG
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
  
  pinMode(Switch1Pin, INPUT_PULLUP);
  pinMode(Switch2Pin, INPUT_PULLUP);
  pinMode(BTN3Pin, INPUT_PULLUP);

  // -------------------------------------------------------------------------

  nowMillis = millis();

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

  debugMsgMain("Start up Timers");

  // Starts the display and the status LED flashing
  startTimers();

  // -------------------------------------------------------------------------
  // Startup test
  ldrManager.setUp();
  ldrManager.setLDRValueToMax(true);

  // Needed to action the PWM
  ldrManager.updateOncePerLoop();

  // First start digit test
  for (int i = 0 ; i <= 20 ; i++) {
    outputManager.loadNumberArraySameValue(i%10);
    outputManager.outputDisplay();
    delay(100);
  }
  ldrManager.setLDRValueToMax(false);

  // -------------------------------------------------------------------------
  
  // Default pins SDA 21, SCL 22 Frequency 400kHz 
  debugMsgMain("Start up I2C...");
  Wire.begin(SDAint, SCLint, 400000L);

  // -------------------------------------------------------------------------
  
  #ifdef FEATURE_MENU
  debugMsgMain("Starting OLED");
  oled.setUp();
  oled.clearDisplay();
  menuManager.flashMenuMessage(CLOCK_MENU_TITLE, "Starting");
  #endif

  // -------------------------------------------------------------------------

  #ifdef FEATURE_BACKLIGHTS
  debugMsgMain("Start up neopixels");
  ledManager.setUp();
  ledManager.setLDRRange(LDR_VALUE_MAX);
  #endif

  // -------------------------------------------------------------------------

  debugMsgMain("Initialising WiFi");
  wifiManager.setUpWiFi();

  if (cc->WifiOnAtStart && wifiManager.wifiCredentialsReceived()) {
    debugMsgMain("Starting WiFi");
    #ifdef FEATURE_MENU
    menuManager.flashMenuMessage("WiFi", "Starting WiFi");
    #endif

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
    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtctime);
    tzManager.calculateCurrentOffsetFromTimeT(rtctime);

    debugMsgMain("Recalculate offset based on recovered datetime");

    // Second time sets the time based on the calculated DST
    rtctime = rtcManager.getRTCTimeAsTimeT();
    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtctime);
    tzManager.calculateCurrentOffsetFromTimeT(rtctime);
  } else {
    debugMsgMain("RTC NOT found");
  }

  // ----------- test routine for UTC offset bug ------------

  // tzManager.setTZS("PDT8PST,M3.2.0,M11.1.0");

  // unsigned long t1 = 1699167600UL;
  // tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_NTP, t1, t1);
  // tzManager.calculateCurrentOffsetFromTimeT(t1);
  // debugMsgMain("!!! Offset: " + String(tzManager.getCurrentUTCOffset()));
  // debugMsgMain("!!! LocalTime t1: " + tzManager.localtimeToReadableString(t1));

  // unsigned long t2 = 1699167600UL + 3600UL;

  // tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_NTP, t2, t2);
  // tzManager.calculateCurrentOffsetFromTimeT(t2);
  // debugMsgMain("!!! Offset: " + String(tzManager.getCurrentUTCOffset()));
  // debugMsgMain("!!! LocalTime t2: " + tzManager.localtimeToReadableString(t2));

  // unsigned long t3 = 1699167600UL + 3600UL + 3600UL;

  // tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_NTP, t3, t3);
  // tzManager.calculateCurrentOffsetFromTimeT(t3);
  // debugMsgMain("!!! Offset: " + String(tzManager.getCurrentUTCOffset()));
  // debugMsgMain("!!! LocalTime t3: " + tzManager.localtimeToReadableString(t3));

  // -------------------------------------------------------------------------
  
  // Start showing the time now that we have something to say
  outputManager.setOutputMode(primaryMode);

  // -------------------------------------------------------------------------
  
  debugMsgMain("Initialising NTP" );
  NewTimeCallback ntcb = newTimeUpdateReceived;
  ntpManager.setNewTimeCallback(ntcb);

  // -------------------------------------------------------------------------
  
  debugMsgMain("Initialising GPS" );
  gpsManager.setUp();

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

  #ifdef FEATURE_MENU
  debugMsgMain("Start up Menu Manager...");
  menuManager.setupMenuManager();
  oled.setUp();
  oled.clearDisplay();
  menuManager.flashMenuMessage(CLOCK_MENU_TITLE, "Welcome to the\nNixie Chronometer\n" + String(SOFTWARE_VERSION));
  delay(2000);
  #endif

  // -------------------------------------------------------------------------

  // Emergency WiFi start
  if ((WiFi.isConnected() == false) && (ENC_BTN) == LOW) {
      debugMsgMain("Start emergency open AP");
      #ifdef FEATURE_MENU
      menuManager.flashMenuMessage("WiFi Start", "Start open access point");
      #endif
      wifiManager.openAccessPortal();
  }
  
  // -------------------------------------------------------------------------

  debugMsgMain("Start up WDT...");
  enableWatchdog();

  // -------------------------------------------------------------------------
  
  #ifdef NIXIE_SLAVE
  debugMsgMain("Start up Nixie Slave");
  slaveManagerNixie.testSlave();
  #endif
  #ifdef DECATRON_SLAVE
  debugMsgMain("Start up Decatron Slave");
  #endif

}

#if defined DIGIT_DIAGNOSTICS && defined FEATURE_BACKLIGHTS
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
void handleSwitchChanges();  // Forward declaration
void performOncePerLoop() {
  // -------------------------------------------------------------------------------

  // Diags mode on the encoder - we inject the encoder value into the output manager
  #if defined DIGIT_DIAGNOSTICS && defined FEATURE_MENU
  if (cc->diagsMode == DIGIT_DIAGS_MODE_ENCODER) {
    ldrManager.setLDRValueToMax(true);
    int rawEncPos = menuManager.getCurrentEncoderPos()/2;
    while (rawEncPos < 0) rawEncPos+=60;
    int burnVal = rawEncPos % 60;
    outputManager.setArbitraryValue(burnVal);
    outputManager.setArbitraryValueDisplayTime(10);
  }
  #endif

  // -------------------------------------------------------------------------------
  
  ldrManager.updateOncePerLoop();

  // -------------------------------------------------------------------------------
  
  #if defined DIGIT_DIAGNOSTICS && defined FEATURE_BACKLIGHTS
  // output the backlight/underlight LEDs
  if (cc->diagsMode > 0) {
    setLedsDiags();
  } else {
    setLeds();
  }

  #elif defined FEATURE_BACKLIGHTS
  // output the backlight/underlight LEDs
  ledManager.updateOncePerLoop();
  #endif

  // -------------------------------------------------------------------------------
  
  outputManager.outputDisplay();

  // -------------------------------------------------------------------------------
  
  #ifdef FEATURE_BLINKENLIGHTS
  blinkenlightsManager.updateBlinkenlights();
  #endif

  // -------------------------------------------------------------------------------
  
  #ifdef FEATURE_MENU
  menuManager.menuLoop();
  #endif

  // -------------------------------------------------------------------------------

  if (switchEventWaiting) {
    handleSwitchChanges();
    switchEventWaiting = false;
  }

  // -------------------------------------------------------------------------------

  wifiManager.manageDNSInOpenAP();
}

// ************************************************************
// Called once per second. Trigger all the things that do
// Not need processing continuously multiple times per second
// ************************************************************
void performOncePerSecondProcessing() {
  lastSecondStartMillis = nowMillis;

  // See if it is time for a new NTP update
  if (ntpManager.getNextUpdate() < 0 && WiFi.isConnected()) {
    ntpManager.getTimeFromNTP();
  }

  // -------------------------------------------------------------------------------
  
  countdownManager.calculateCountdown();

  // -------------------------------------------------------------------------------
  
  outputManager.updateOncePerSecond();

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

  ldrManager.updateOncePerSecond();
    
  // -------------------------------------------------------------------------------
  
  #ifdef FEATURE_BACKLIGHTS
  ledManager.updateOncePerSecond();
  #endif

  // -------------------------------------------------------------------------------
  
  // Feed the GPS parser. Collect any data that has arrived in the UART buffer and 
  // stream it into the GPS parser.
  while (Serial.available()) {
    char c = Serial.read();
    gpsManager.parseNMEAMsg(c);
  }

  // -------------------------------------------------------------------------------

  #if defined(NIXIE_SLAVE) 
  slaveManagerNixie.updateOncePerSecond();
  #endif

  #if defined(DECATRON_SLAVE) 
  slaveManagerDecatron.updateOncePerSecond();
  #endif

  // -------------------------------------------------------------------------------
  // Check time / motion detector blanking status
  // Blanking manager will push the blanking status to the other components
  blankingManager.updateBlankingStatus();

  // -------------------------------------------------------------------------------
  // Service the menu
  #ifdef FEATURE_MENU
  menuManager.menuOncePerSecond();
  #endif

  #ifdef FEATURE_TICKERS
  getBTCPrice();
  #endif

  // -------------------------------------------------------------------------------
  debugManager.debugAutoOffCheck();

  // -------------------------------------------------------------------------------

  #ifdef OTM_EXTENDED_DEBUG
  outputManager.dumpNumberArrayValues();
  #endif

  // -------------------------------------------------------------------------------

  feedWatchdog();
}

// ************************************************************
// called to process switch changes. An interrupt sets a 
// trigger and the mail loop calls this to process the 
// waiting changes
// ************************************************************
void handleSwitchChange(byte mode, bool state) {
  switch(mode) {
    case SW_NONE: {
      // nothing
      break;
    }
    case SW_COUNTDOWN_INHIBIT: {
      if (state) {
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
    case SW_SLAVE_INHIBIT: {
      #ifdef NIXIE_SLAVE
      slaveManagerNixie.setSlaveEnabled(!state);
      #endif
      #ifdef DECATRON_SLAVE
      slaveManagerDecatron.setSlaveEnabled(!state);
      #endif
      break;
    }
    case SW_MIN_DIM: {
      // The switch "on" imposes min dimming
      if ((state) && !ldrManager.getLDRValueSetToMin()) {
        ldrManager.setLDRValueToMin(true);
      }
      // Switch off resets it
      if ((!state) && ldrManager.getLDRValueSetToMin()) {
        ldrManager.setLDRValueToMin(false);
      }
      break;
    }
    case SW_BLANK_LEDS: {
      blankingManager.setCurrentLEDBlankingOverride(state);
      break;
    }
  }
}

// ************************************************************
// Each of the switches can be configured to perform the same
// tasks, therefore call the routine for each of the switch
// contexts in turn
// ************************************************************
void handleSwitchChanges() {
  #ifdef NORMAL_SWITCHES
  bool BTNOnstate = LOW;
  #endif

  #ifdef INVERT_SWITCHES
  bool BTNOnstate = HIGH;
  #endif

  #if defined(COUNTDOWN)
  // Handle the case that we have finished the countdown and the mode needs to be reset
  if ((cc->sw1Mode == SW_COUNTDOWN_INHIBIT) && (countdownManager.getCountdownActiveInternal() == false)) {
      // We are not in a countdown, reset the switch
      cc->sw1Mode = SW1_DEFAULT;
  }
  if ((cc->sw2Mode == SW_COUNTDOWN_INHIBIT) && (countdownManager.getCountdownActiveInternal() == false)) {
      // We are not in a countdown, reset the switch
      cc->sw2Mode = SW2_DEFAULT;
  }
  #endif

  bool sw1State = (digitalRead(Switch1Pin) == BTNOnstate);
  bool sw2State = (digitalRead(Switch2Pin) == BTNOnstate);

  bool switchvalues[4] = {false, false, false, false};

  if (sw1State) {
    switchvalues[cc->sw1Mode] = true;
  }

  if (sw2State) {
    switchvalues[cc->sw2Mode] = true;
  }
  
  // Skip the "none" value
  handleSwitchChange(SW_SLAVE_INHIBIT    , switchvalues[SW_SLAVE_INHIBIT    ]);
  handleSwitchChange(SW_MIN_DIM          , switchvalues[SW_MIN_DIM          ]);
  handleSwitchChange(SW_BLANK_LEDS       , switchvalues[SW_BLANK_LEDS       ]);
  handleSwitchChange(SW_COUNTDOWN_INHIBIT, switchvalues[SW_COUNTDOWN_INHIBIT]);
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

  #if defined NIXIE_SLAVE
  slaveManagerNixie.updateOncePerMinute();
  #endif

  // If we are using the Decatron slave, let the manager control the 1PPS
  #ifdef DECATRON_SLAVE
  slaveManagerDecatron.updateOncePerMinute();
  #endif

  quoteManager.getQuote();
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  debugMsgMain("---> OncePerHourProcessing");

  #ifdef FEATURE_MENU
  // Menu updates once per hour - currently not used
  menuManager.menuOncePerHour();
  #endif
  
  // Update UTC offset and sync RTC to GPS if used
  tzManager.tzmOncePerHour();

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

  spiffsStorage.saveStatsToSpiffs();
}

// ************************************************************
// Main loop
// ************************************************************
void loop()
{
  nowMillis = millis();

  if (lastSecondStartMillis > nowMillis) {
    // rollover
    lastSecondStartMillis = 0;
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

  // Calculate the intra second millis
  secsDeltaAbs = (nowMillis - lastSecondStartMillis);
  if (secsDeltaAbs > 1000) {secsDeltaAbs = 1000;}
  if (secsDeltaAbs < 0) {secsDeltaAbs = 0;}
  upOrDown = (second() % 2) == 0;
  
  if (upOrDown) {
    secsDelta = secsDeltaAbs;
  } else {
    secsDelta = 1000 - secsDeltaAbs;
  }

  delay(10);
}

