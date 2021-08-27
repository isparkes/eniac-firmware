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
#include "clock_timers.h"
#include <ESP32Encoder.h>
#include <LDRManager.h>
#include <LEDManager.h>

const int PWMFreq = 1000; /* 1 KHz */
const int LDRPWMChannel = 0;
const int PWMResolution = 12;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    debugMsg("Station Mode Started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    debugMsg("Connected to :" + WiFi.SSID() + ", password: " + WiFi.psk());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    debugMsg("Disconnected from station, attempting reconnection");
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    debugMsg("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
    esp_wifi_wps_disable();
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    debugMsg("WPS Failed, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    debugMsg("WPS Timedout, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    debugMsg("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  // -------------------------------------------------------------------------

  debugMsg("Start up Timers" );
  startTimers();

  // -------------------------------------------------------------------------
  
  debugMsg("Starting OLED");
  oled.setUp();
  oled.clearDisplay();

  // -------------------------------------------------------------------------

  debugMsg("Start up neopixels");
  ledManager.setUp();
  ledManager.setLDRRange(LDR_VALUE_MAX);

  // -------------------------------------------------------------------------
  
  debugMsg("");
  debugMsg("Starting WiFi");
  oled.showScrollingMessage("Starting WiFi");

  WiFi.onEvent(WiFiEvent);

  String mac = String(WiFi.macAddress());
  mac.replace(":","");
  String uniqHostname = "ESP32-"+mac.substring(6);

  debugMsg("Unique hostname: " + uniqHostname);

  WiFi.setHostname(uniqHostname.c_str());
  WiFi.begin();

  debugMsg("");
  debugMsg("Trying to reconnect to last known AP");
  oled.showScrollingMessage("Connect to last AP");

  unsigned long maxMillisWiFiWait = millis() + INTERVAL_WIFI;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (previousMillisWiFi < maxMillisWiFiWait)
    {
      previousMillisWiFi = millis();
      Serial.print(".");

      delay(500);
    }
    else {
        debugMsg("");
        debugMsg("Failed to connect");
        debugMsg("");
        break;
    }
  }

  // -------------------------------------------------------------------------
  
  // Try WPS
  if (WiFi.status() != WL_CONNECTED) {  
    debugMsg("");
    debugMsg("Connect using WPS");
    oled.showScrollingMessage("Connect using WPS");
    maxMillisWiFiWait = millis() + INTERVAL_WPS;
    while (WiFi.status() != WL_CONNECTED)
    {
      wpsInitConfig();
      esp_wifi_wps_enable(&wps_config);

      if (previousMillisWiFi < maxMillisWiFiWait)
      {
        previousMillisWiFi = millis();
        Serial.print(".");

        esp_wifi_wps_start(500);
        delay(500);
      } else {
        debugMsg("");
        debugMsg("Failed to connect");
        debugMsg("");
        break;
      }
    }
  }

  // -------------------------------------------------------------------------
  
  // Captive portal
  if (WiFi.status() != WL_CONNECTED) {  
    debugMsg("");
    debugMsg("Portal mode");
    oled.showScrollingMessage("Portal mode");

    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_MODE_APSTA);
    delay(100);
    // WiFi.softAPsetHostname(uniqHostname.c_str());
    delay(100);
    debugMsg("Setting soft-AP configuration ... ");
    WiFi.softAP(uniqHostname.c_str());
    delay(100);
    debugMsg("Soft-AP IP address = ");
    debugMsg(WiFi.softAPIP().toString());
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
        Serial.print("o");
        wifiBeginWithCredentials();
      } else {
        Serial.print(".");
      }
      delay(500);
    } else {
      debugMsg("");
      debugMsg("Failed to connect");
      debugMsg("");
      break;
    }
  }

  // -------------------------------------------------------------------------
  
  debugMsg("");
  debugMsg("Connesso a: " + WiFi.SSID());
  debugMsg("IP Address: " + WiFi.localIP().toString());
  debugMsg("MAC Address: " + WiFi.macAddress());
  debugMsg("Host name: " + String(WiFi.getHostname()));

  // Connected, show only the IP
  oled.clearDisplay();
  oled.showScrollingMessage("IP: " + WiFi.localIP().toString());
  oled.showScrollingMessage(String(WiFi.getHostname()) + ".local");
  
  // -------------------------------------------------------------------------
  
  debugMsg("Start up SPIFFS");

  // define the debug callback
  DebugCallback dbcb = debugMsg;

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    debugMsg("An Error has occurred while mounting SPIFFS");
    return;
  }

  debugMsg("Startup SPIFFS storage");
  spiffsStorage.setDebugCallback(dbcb);
  spiffsStorage.setDebugOutput(true);
  bool statsLoaded = spiffsStorage.getStatsFromSpiffs(cs);

  if (!statsLoaded) {
    debugMsg("SPIFFS storage: read stats failed");
    spiffsStorage.saveStatsToSpiffs(cs);
  }

  bool configloaded = spiffsStorage.getConfigFromSpiffs(cc);

  if (configloaded) {
    debugMsg("Got TZS: " + cc->tzs);
    ntpAsync.setTZS(cc->tzs);
    ntpAsync.setNtpPool(cc->ntpPool);
    ntpAsync.setUpdateInterval(cc->ntpUpdateInterval);
  } else {
    debugMsg("SPIFFS storage: read config failed - do factory reset");
    resetOptions();
  }

  // -------------------------------------------------------------------------
  
  debugMsg("Start up WebServer" );

  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");

  server.on("/api/getSummary", HTTP_GET, getSummaryDataHandler);
  
  server.on("/api/getTimeserver", HTTP_GET, getTimeserverDataHandler);
  server.on("/api/postTimeserver", HTTP_POST, postTimeserverDataHandler);
  
  server.on("/api/getConfig", HTTP_GET, getConfigDataHandler);
  server.on("/api/postConfig", HTTP_POST, postConfigDataHandler);

  server.on("/api/postWiFiCredentials", HTTP_POST, postWiFiDataHandler);
  server.on("/utils/resetWifi", HTTP_GET, resetWifiHandler);
  server.on("/utils/scanI2C", HTTP_GET, getI2CScanHandler);
  server.on("/utils/saveStats", HTTP_GET, saveStatsHandler);
  server.on("/utils/ntpupdate", HTTP_GET, [] (AsyncWebServerRequest *request) {
    ntpAsync.resetNextUpdate();
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
  server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "text/plain", "The content you are looking for was not found.");
  });

  // -------------------------------------------------------------------------
  
  debugMsg("Start up OTA");
  AsyncElegantOTA.begin(&server, "admin", "update");

  server.begin();

  // -------------------------------------------------------------------------
  
  debugMsg("Start up mDNS on http://" + String(WiFi.getHostname()) + ".local");

  // The MDNS host name does not seem to work at the moment - it is being set by OTA
  if(!MDNS.begin(uniqHostname.c_str())) {
      debugMsg("Error starting mDNS");
      return;
  }

  MDNS.addService("http", "tcp", 80);

  debugMsg("Start up NTP" );

  ntpAsync.setDebugCallback(dbcb);
  ntpAsync.setDebugOutput(true);

  NewTimeCallback ntcb = newTimeUpdateReceived;
  ntpAsync.setNewTimeCallback(ntcb);

  // -------------------------------------------------------------------------
  
  // Default pins SDA 21, SCL 22
  debugMsg("Start up I2C...");
  Wire.begin();

  // -------------------------------------------------------------------------
  
  // kick off NTP updates
  nowMillis = millis();
  ntpAsync.getTimeFromNTP(nowMillis);

  // debugMsg("Current uptime: " + String(cs->uptimeMins));

  // -------------------------------------------------------------------------

  debugMsg("Start up encoder");

  ESP32Encoder encoder;
	ESP32Encoder::useInternalWeakPullResistors=UP;

	// use pin 19 and 18 for the first encoder
	encoder.attachHalfQuad(encoderA, encoderB);
		
	// clear the encoder's raw count and set the tracked count to zero
	encoder.clearCount();

  // -------------------------------------------------------------------------
  
  debugMsg("Start up GPIOs");
  pinMode(LED_PIN, OUTPUT);

  pinMode(CLKPin, OUTPUT);
  pinMode(BLANKPin, OUTPUT);
  pinMode(DATA1Pin, OUTPUT);
  pinMode(LATCH1Pin, OUTPUT);
  pinMode(DATA2Pin, OUTPUT);
  pinMode(LATCH2Pin, OUTPUT);
  pinMode(DATA3Pin, OUTPUT);
  pinMode(LATCH3Pin, OUTPUT);

  ledcSetup(LDRPWMChannel, PWMFreq, PWMResolution);
  ledcAttachPin(BLANKPin, LDRPWMChannel);
  setLedFlashType(1);
  ledcWrite(LDRPWMChannel, MAX_DUTY_CYCLE/2);

  // -------------------------------------------------------------------------
  
  debugMsg("Start up LDR...");
  // Not managing sensorSmoothCountLDR yet
  cc->sensorSmoothCountLDR = SENSOR_SMOOTH_READINGS_DEFAULT;
  ldrManager.setDebugOutput(true);
  ldrManager.setDebugCallback(dbcb);
  ldrManager.setUp();
  ldrManager.setDebugOutput(false);

  // -------------------------------------------------------------------------
  
  debugMsg("Start up WDT...");
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
  
  bool led1State;
  bool led2State;

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


// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  lastMillis = nowMillis;

  if (ntpAsync.getNextUpdate(nowMillis) < 0) {
    ntpAsync.getTimeFromNTP(nowMillis);
  }

  bool connected = (WiFi.status() == WL_CONNECTED);
  if (connected) {
    setLedFlashType(0);
  } else {
    setLedFlashType(1);
  }

  // send time update to OLED and set the other status flags 
  char time_c[11];
  sprintf(time_c, "%02d:%02d:%02d", hour(), minute(), second());
  oled.setWiFiStatus(connected);
  oled.setBlankStatus(false);
  oled.setNTPStatus(ntpAsync.ntpTimeValid(nowMillis));
  oled.setTimeString(String(time_c));

  // ************************************************************
  // Break the time into displayable digits
  // ************************************************************
  numberArray[5] = second() % 10;
  numberArray[4] = second() / 10;
  numberArray[3] = minute() % 10;
  numberArray[2] = minute() / 10;
  if (cc->hourMode) {
    numberArray[1] = hourFormat12() % 10;
    numberArray[0] = hourFormat12() / 10;
  } else {
    numberArray[1] = hour() % 10;
    numberArray[0] = hour() / 10;
  }

  // send time display to the drivers
  bool led1 = (second() % 2 == 0);
  bool led2 = (second() % 2 == 1);
  val1 = decodeBCD(hour(), led1, led2);
  val2 = decodeBCD(minute(), led1, led2);
  val3 = decodeBCD(second(), led1, led2);

  // debugMsg("LDR Reading: " + String(ldrValue));

  esp_task_wdt_reset();
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  debugMsg("---> OncePerMinuteProcessing");

  debugMsg("nu: " + String(ntpAsync.getNextUpdate(nowMillis)));

  // Usage stats
  cs->uptimeMins++;

  if (!blanked) {
    cs->tubeOnTimeMins++;
  }
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  debugMsg("---> OncePerHourProcessing");
  oled.setAMStatus(isAM());
}

// ************************************************************
// Called once per day
// ************************************************************
void performOncePerDayProcessing() {
  debugMsg("---> OncePerDayProcessing");
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

  // Touch sensor
  #define TOUCH_THRESHOLD 20
  oled.setXStatus(touchRead(btn1) > TOUCH_THRESHOLD);
  oled.setYStatus(touchRead(btn2) > TOUCH_THRESHOLD);

  // -------------------------------------------------------------------------------

  ldrManager.getDimmingFromLDR();
  ldrValue = ldrManager.getLDRValue();

  // set the digit brightness
  ledcWrite(LDRPWMChannel, ldrValue);
  ledManager.setLDRValue(ldrValue);

  // -------------------------------------------------------------------------------
  
  // output the backlight/underlight LEDs
  setLeds();

  delay(10);
}


