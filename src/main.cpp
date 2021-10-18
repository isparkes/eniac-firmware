#include "defs.h"
#include "globals.h"
#include "utilities.h"
#include "WiFi.h"
#include <ESPmDNS.h>
#include "wps.h"
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include "OLED.h"
#include <AsyncElegantOTA.h>
#include "TimerManager.h"
#include "LDRManager.h"
#include "LEDManager.h"
#include "MyLib.h"
#include "GPSManager.h"
#include "BlankingManager.h"
#include "TZManager.h"
#include "EncoderManager.h"
#include "RTCManager.h"
#include "BlinkenlightsManager.h"
#include "NTPManager.h"

// ToDo move to display manager
const int PWMFreq = 1000; /* 1 KHz */
const int LDRPWMChannel = 0;
const int PWMResolution = 12;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    #ifdef DEBUG_ON
    debugMsg("Station Mode Started");
    #endif
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    #ifdef DEBUG_ON
    debugMsg("Connected to :" + WiFi.SSID() + ", password: " + WiFi.psk());
    #endif
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    #ifdef DEBUG_ON
    debugMsg("Disconnected from station, attempting reconnection");
    #endif
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    #ifdef DEBUG_ON
    debugMsg("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
    #endif
    esp_wifi_wps_disable();
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    #ifdef DEBUG_ON
    debugMsg("WPS Failed, retrying");
    #endif
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    #ifdef DEBUG_ON
    debugMsg("WPS Timedout, retrying");
    #endif
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    #ifdef DEBUG_ON
    debugMsg("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
    #endif
    break;
  default:
    break;
  }
}

void setup()
{
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

  pinMode(BLANKPin, OUTPUT);

  pinMode(ENC_BTN, INPUT_PULLUP);

  pinMode(PPSPin, OUTPUT);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up dimming PWM");
  #endif
  ledcSetup(LDRPWMChannel, PWMFreq, PWMResolution);
  ledcAttachPin(BLANKPin, LDRPWMChannel);
  ledcWrite(LDRPWMChannel, MAX_DUTY_CYCLE);

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up Timers" );
  #endif
  startTimers();

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Starting OLED");
  #endif
  oled.setUp();
  oled.clearDisplay();
  oledTime = OLED_ON_TIME;

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up neopixels");
  #endif
  ledManager.setUp();
  ledManager.setLDRRange(LDR_VALUE_MAX);

  // -------------------------------------------------------------------------

  // debugMsg("Test pattern");
  // for (int j = 0 ; j < 10 ; j++) {
  //   for (int i = 0 ; i < 10 ; i++) {
  //     uint32_t val = decodeBCD(i*10+i, i%2 == 0, i %2 == 1);
  //     val1 = val2 = val3 = decodeBCD(i*10+i, i%2 == 0, i %2 == 1);
  //     ledManager.setTestValue(i);
  //     delay(100);
  //   }
  // }
  
  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("");
  debugMsg("Starting WiFi");
  #endif
  oled.showScrollingMessage("Starting WiFi");

  WiFi.onEvent(WiFiEvent);

  String mac = String(WiFi.macAddress());
  mac.replace(":","");
  String uniqHostname = "ESP32-"+mac.substring(6);

  #ifdef DEBUG_ON
  debugMsg("Unique hostname: " + uniqHostname);
  #endif

  WiFi.setHostname(uniqHostname.c_str());
  WiFi.begin();

  #ifdef DEBUG_ON
  debugMsg("");
  debugMsg("Trying to reconnect to last known AP");
  #endif
  oled.showScrollingMessage("Connect to last AP");

  unsigned long maxMillisWiFiWait = millis() + INTERVAL_WIFI;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (previousMillisWiFi < maxMillisWiFiWait)
    {
      previousMillisWiFi = millis();
      #ifdef DEBUG_ON
      debugMsgCont(".");
      #endif

      delay(500);
    }
    else {
        #ifdef DEBUG_ON
        debugMsg("");
        debugMsg("Failed to connect");
        debugMsg("");
        #endif
        break;
    }
  }

  // -------------------------------------------------------------------------
  
  // Try WPS
  if (WiFi.status() != WL_CONNECTED) {  
    #ifdef DEBUG_ON
    debugMsg("");
    debugMsg("Connect using WPS");
    #endif
    oled.showScrollingMessage("Connect using WPS");
    maxMillisWiFiWait = millis() + INTERVAL_WPS;
    while (WiFi.status() != WL_CONNECTED)
    {
      wpsInitConfig();
      esp_wifi_wps_enable(&wps_config);

      if (previousMillisWiFi < maxMillisWiFiWait)
      {
        previousMillisWiFi = millis();
        #ifdef DEBUG_ON
        debugMsgCont(".");
        #endif

        esp_wifi_wps_start(500);
        delay(500);
      } else {
        #ifdef DEBUG_ON
        debugMsg("");
        debugMsg("Failed to connect");
        debugMsg("");
        #endif
        break;
      }
    }
  }

  // -------------------------------------------------------------------------
  
  // Captive portal
  if (WiFi.status() != WL_CONNECTED) {  
    #ifdef DEBUG_ON
    debugMsg("");
    debugMsg("Portal mode");
    #endif
    oled.showScrollingMessage("Portal mode");

    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_MODE_APSTA);
    delay(100);
    // WiFi.softAPsetHostname(uniqHostname.c_str());
    delay(100);
    #ifdef DEBUG_ON
    debugMsg("Setting soft-AP configuration ... ");
    #endif
    WiFi.softAP(uniqHostname.c_str());
    delay(100);
    #ifdef DEBUG_ON
    debugMsg("Soft-AP IP address = ");
    debugMsg(WiFi.softAPIP().toString());
    #endif
   oled.showScrollingMessage("IP: " + WiFi.softAPIP().toString());
    delay(500);
    server.on("/api/credentials", HTTP_GET, getCredentialsHandler);

    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Go to /api/credentials");
    });

    server.begin();
  }

  maxMillisWiFiWait = millis() + INTERVAL_PORTAL;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (previousMillisWiFi < maxMillisWiFiWait)
    {
      previousMillisWiFi = millis();
      if (gotCredentials()) {
        #ifdef DEBUG_ON
        debugMsgCont("o");
        #endif
        wifiBeginWithCredentials();
      } else {
        #ifdef DEBUG_ON
        debugMsgCont(".");
        #endif
      }
      delay(500);
    } else {
      #ifdef DEBUG_ON
      debugMsg("");
      debugMsg("Failed to connect");
      debugMsg("");
      #endif
      break;
    }
  }

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("");
  debugMsg("Connected to: " + WiFi.SSID());
  debugMsg("IP Address: " + WiFi.localIP().toString());
  debugMsg("MAC Address: " + WiFi.macAddress());
  debugMsg("Host name: " + String(WiFi.getHostname()));
  #endif

  // Connected, show only the IP
  oled.clearDisplay();
  oled.showScrollingMessage("IP: " + WiFi.localIP().toString());
  oled.showScrollingMessage(String(WiFi.getHostname()) + ".local");
  
  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up SPIFFS");

  // define the debug callback
  DebugCallback dbcb = debugMsg;
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
    #ifdef DEBUG_ON
    debugMsg("Got TZS: " + cc->tzs);
    #endif
    ntpManager.setTZS(cc->tzs);
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
  debugMsg("Start up WebServer" );
  #endif

  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");

  // Summary and diagnostics
  server.on("/api/getSummary", HTTP_GET, getSummaryDataHandler);
  server.on("/api/getDiags", HTTP_GET, getDiagsDataHandler);
  
  // Configure time server
  server.on("/api/getTimeserver", HTTP_GET, getTimeserverDataHandler);
  server.on("/api/postTimeserver", HTTP_POST, postTimeserverDataHandler);
  
  // Configure options
  server.on("/api/getConfig", HTTP_GET, getConfigDataHandler);
  server.on("/api/postConfig", HTTP_POST, postConfigDataHandler);

  // wifi credentials
  server.on("/api/postWiFiCredentials", HTTP_POST, postWiFiDataHandler);

  // Utilities
  server.on("/utils/resetWifi", HTTP_GET, resetWifiHandler);
  server.on("/utils/scanI2C", HTTP_GET, getI2CScanHandler);
  server.on("/utils/saveStats", HTTP_GET, saveStatsHandler);
  server.on("/utils/ntpupdate", HTTP_GET, [] (AsyncWebServerRequest *request) {
    ntpManager.resetNextUpdate();
        request->redirect("/utility.html");;
    });
  server.on("/utils/resetwifi", HTTP_GET, [] (AsyncWebServerRequest *request) {
    resetWifi();
        request->redirect("/utility.html");;
    });
  server.on("/utils/resetoptions", HTTP_GET, [] (AsyncWebServerRequest *request) {
    resetOptions();
        request->redirect("/utility.html");;
    });
  server.on("/utils/resetall", HTTP_GET, [] (AsyncWebServerRequest *request) {
    resetAll();
        request->redirect("/utility.html");;
    });
  server.on("/utils/restart", HTTP_GET, restartHandler);

  server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "text/plain", "The content you are looking for was not found.");
  });

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up OTA");
  #endif
  AsyncElegantOTA.begin(&server, "admin", "update");

  server.begin();

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up mDNS on http://" + String(WiFi.getHostname()) + ".local");
  #endif

  // The MDNS host name does not seem to work at the moment - it is being set by OTA
  if(!MDNS.begin(uniqHostname.c_str())) {
      #ifdef DEBUG_ON
      debugMsg("Error starting mDNS");
      #endif
      return;
  }

  MDNS.addService("http", "tcp", 80);

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
  if (rtcManager.testRTCTimeProvider()) {
    rtcManager.getRTCTime(true, nowMillis);
    #ifdef DEBUG_ON
    debugMsg("RTC found");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsg("RTC NOT found");
    #endif
  }

  // -------------------------------------------------------------------------
  
  // kick off NTP updates
  nowMillis = millis();
  ntpManager.getTimeFromNTP(nowMillis);

  // debugMsg("Current uptime: " + String(cs->uptimeMins));

  // -------------------------------------------------------------------------

  #ifdef DEBUG_ON
  debugMsg("Start up encoder");
  #endif

  encoderManager.setup();

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
  debugMsg("Start up GPS/Serial...");
  #endif
  // Serial.begin(115200);

  // -------------------------------------------------------------------------
  
  #ifdef DEBUG_ON
  debugMsg("Start up Blanking");
  #endif
  blankingManager.begin();

  // -------------------------------------------------------------------------

  MyLib.begin();
  MyLib.doStuff();

  #ifdef DEBUG_ON
  debugMsg("Start up WDT...");
  #endif
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
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

int encoderCount;

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  lastMillis = nowMillis;

  if (ntpManager.getNextUpdate(nowMillis) < 0) {
    ntpManager.getTimeFromNTP(nowMillis);
  }

  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected) {
    setLedFlashType(0);
  } else {
    setLedFlashType(1);
  }

  // Touch sensor
  bool btn1 = touchRead(BTN1Pin) < TOUCH_THRESHOLD;
  bool btn2 = touchRead(BTN2Pin) < TOUCH_THRESHOLD;
  bool btn3 = touchRead(BTN3Pin) < TOUCH_THRESHOLD;

  if (btn2) {
    oledTime = 1;
  }

  if (btn1 && oledTime == 0) {
    oledTime = OLED_ON_TIME;
    oled.setUp();
    oled.clearDisplay();
    oled.showScrollingMessage("IP: " + WiFi.localIP().toString());
    oled.showScrollingMessage(String(WiFi.getHostname()) + ".local");
  }

  if (oledTime > 0) {
    // ************************************************************
    // send time update to OLED and set the other status flags 
    // ************************************************************
    char time_c[11];
    sprintf(time_c, "%02d:%02d:%02d", hour(), minute(), second());
    oled.setTimeString(String(time_c));

    oled.setWiFiStatus(connected);
    oled.setBlankStatus(false);
    oled.setNTPStatus(ntpManager.ntpTimeValid(nowMillis));
    if (digitalRead(PIRPin) == false) {
      oled.setPIRInstalled(true);  
    }
    oled.setPIRStatus(digitalRead(PIRPin));

    oled.setXStatus(btn1);
    oled.setYStatus(btn2);
    oled.setZStatus(btn3);
  }

  // ************************************************************
  // send time display to the drivers
  // ************************************************************
  // ToDo move into output manager
  indLed1 = (second() % 2 == 0);
  indLed2 = (second() % 2 == 1);

  blinkenlightsManager.setBlinkenlightsStatus(bl);

  encoderCount = encoderManager.getCount();

  loadNumberArrayTime();

  // -------------------------------------------------------------------------------
  if (oledTime > 0) {
    oledTime--;
    if (oledTime == 0) {
      oled.setUp();
      oled.blankDisplay();
    }
  }

  // debugMsg("EncBTN: " + String(digitalRead(encBTN)));
  // debugMsg("EncCount: " + String((int) encoder.getCount()));

  // debugMsg("Enc Attached: " + String(encoder.isAttached()));

  // if (useRTC) {
  //   debugMsg("RTC Enabled");
  //   debugMsg("RTC result: " + getRTCTime(false));
  // } else {
  //   debugMsg("RTC NOT Enabled");
  // }

  // debugMsg("Impressions: " + String(impressions));
  // debugMsg("Outputs:  " + String(outputs1) + ":" + String(outputs2) + ":" + String(outputs3));
  // debugMsg("Switches: " + String(switches1) + ":" + String(switches2) + ":" + String(switches3));

  // debugMsg("val1: " + String(val1) + ":" + String(nextVal1));

  blankingManager.getBlankingStatus(nowMillis, weekday(), hour());
  ledManager.setBlanked(blankingManager.getCurrentBlankLEDs());

  // Feed the GPS parser
  while (Serial.available()) {
    char c = Serial.read();
    gpsManager.parseNMEAMsg(c, nowMillis);
  }

  // Trigger 1PPS signal
  triggerTimer2();

  // Feed the watchdog - woof
  esp_task_wdt_reset();
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerMinuteProcessing");
  debugMsg("nu: " + String(ntpManager.getNextUpdate(nowMillis)));
  #endif

  // Set the internal time to the time from the RTC even if we are still in
  // NTP valid time. This is more accurate than using the internal time source
  rtcManager.getRTCTime(true, nowMillis);

  // Usage stats
  cs->uptimeMins++;

  if (!blankingManager.getCurrentBlankingStatus()) {
    cs->tubeOnTimeMins++;
  }

  // recalculate the UTC offset
  tzManager.calculateCurrentOffset(year(),month(),day(),hour(),minute(),second());
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerHourProcessing");
  #endif
  oled.setAMStatus(isAM());
}

// ************************************************************
// Called once per day
// ************************************************************
void performOncePerDayProcessing() {
  #ifdef DEBUG_ON
  debugMsg("---> OncePerDayProcessing");
  #endif
  spiffsStorage.saveStatsToSpiffs(cs);
}

void loop()
{
  AsyncElegantOTA.loop();

  // See if it is time to update the Clock
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

  // set the digit brightness
  ledcWrite(LDRPWMChannel, ldrValue);
  ledManager.setLDRValue(ldrValue);

  // -------------------------------------------------------------------------------
  
  outputDisplay();

  // output the backlight/underlight LEDs
  setLeds();

  delay(10);
}


