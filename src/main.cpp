#include "defs.h"
#include "globals.h"
#include "utilities.h"
#include "WiFi.h"
#include <ESPmDNS.h>
#include "esp_wps.h"
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include "OLED.h"
#include <AsyncElegantOTA.h>
#include "clock_timers.h"
#include <ESP32Encoder.h>
#include <NeoPixelBus.h>
#include <LDRManager.h>

/*
  Change the definition of the WPS mode
  from WPS_TYPE_PBC to WPS_TYPE_PIN in
  the case that you are using pin type
  WPS
*/
#define ESP_WPS_MODE WPS_TYPE_PBC
#define ESP_MANUFACTURER "ESPRESSIF"
#define ESP_MODEL_NUMBER "ESP32"
#define ESP_MODEL_NAME "ESPRESSIF IOT"
#define ESP_DEVICE_NAME "ESP STATION"

static esp_wps_config_t config;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

#define colorSaturation 128

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

const int PWMFreq = 1000; /* 1 KHz */
const int LDRPWMChannel = 0;
const int PWMResolution = 12;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

void wpsInitConfig()
{
  config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

String wpspin2string(uint8_t a[])
{
  char wps_pin[9];
  for (int i = 0; i < 8; i++)
  {
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}

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
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    debugMsg("WPS Timedout, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    debugMsg("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
    break;
  default:
    break;
  }
}

void setPixels() {
    strip.SetPixelColor(0, red);
    strip.SetPixelColor(1, green);
    strip.SetPixelColor(2, red);
    strip.SetPixelColor(3, green);

    strip.SetPixelColor(4, blue);

    strip.SetPixelColor(5, red);
    strip.SetPixelColor(6, green);
    strip.SetPixelColor(7, red);
    strip.SetPixelColor(8, green);

    strip.SetPixelColor(9, blue);

    strip.SetPixelColor(10, red);
    strip.SetPixelColor(11, green);
    strip.SetPixelColor(12, red);
    strip.SetPixelColor(13, green);
    strip.Show();
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  // -------------------------------------------------------------------------

  debugMsg("Start up neopixels");
  strip.Begin();
  setPixels();

  // -------------------------------------------------------------------------

  debugMsg("Start up Timers" );
  startTimers();

  // -------------------------------------------------------------------------
  
  debugMsg("Starting OLED");
  oled.setUp();
  oled.clearDisplay();

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
      esp_wifi_wps_enable(&config);

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
    debugMsg("SPIFFS storage: read config failed");
    cc->tzs = TIME_ZONE_STRING_DEFAULT;
    cc->ntpPool = NTP_POOL_DEFAULT;
    cc->ntpUpdateInterval = NTP_UPDATE_INTERVAL_DEFAULT;
    spiffsStorage.saveConfigToSpiffs(cc);
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

  pinMode(clk1, OUTPUT);
  pinMode(BLANKPin, OUTPUT);
  pinMode(data1, OUTPUT);
  pinMode(latch1, OUTPUT);
  pinMode(data2, OUTPUT);
  pinMode(latch2, OUTPUT);
  pinMode(data3, OUTPUT);
  pinMode(latch3, OUTPUT);

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
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
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
  oled.setYStatus(SPIFFS.begin(false));

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
//  ArduinoOTA.handle();
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
  oled.setXStatus(touchRead(4) > TOUCH_THRESHOLD);

  // -------------------------------------------------------------------------------

  ldrManager.getDimmingFromLDR();
  ldrValue = ldrManager.getLDRValue();

  // set the digit brightness
  ledcWrite(LDRPWMChannel, ldrValue);

  // -------------------------------------------------------------------------------
  
  setPixels();

  delay(10);
}


