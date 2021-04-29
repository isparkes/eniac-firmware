unsigned long previousMillisWiFi = 0;
long INTERVAL_WIFI = 6000;
long INTERVAL_WPS  = 60000;

long intervalWiFi = 6000;

#include "WiFi.h"
#include <ESPmDNS.h>
#include "esp_wps.h"
#include <ArduinoOTA.h>
#include "utilities.h"
#include <esp_task_wdt.h>
#include "OLED.h"

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

#define LED_PIN 2

hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;

// Led Timer
void IRAM_ATTR onTimer0() {
   portENTER_CRITICAL_ISR(&timerMux0);
   count0++;
   if (count0 > count0Max) {
     count0 = 0;
     digitalWrite(LED_PIN, HIGH);
   } else if (count0 == count0Off) {
     digitalWrite(LED_PIN, LOW);
   }
   portEXIT_CRITICAL_ISR(&timerMux0);

}

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timerMux1);
   count1++;
   portEXIT_CRITICAL_ISR(&timerMux1);
}

#define WDT_TIMEOUT 3

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
    debugMsg("Connected to :" + String(WiFi.SSID()) + " Got IP: " + WiFi.localIP().toString());
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

void setupOTA()
{
  // Port defaults to 3232
  ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        debugMsg("Start updating " + type);
      })
      .onEnd([]() {
        debugMsg("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          debugMsg("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          debugMsg("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          debugMsg("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          debugMsg("Receive Failed");
        else if (error == OTA_END_ERROR)
          debugMsg("End Failed");
      });

  debugMsg("Start up OTA: " + ArduinoOTA.getHostname());
  ArduinoOTA.begin();
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  debugMsg("Start up Timers" );
  
  // Configure LED output
  pinMode(LED_PIN, OUTPUT);

  setLedFlashType(1);

  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer0, true);
  timerAlarmWrite(timer0, 1000, true);
  timerAlarmEnable(timer0);

  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 33333, true);
  timerAlarmEnable(timer1);
  setLedFlashType(1);

  debugMsg("Starting OLED");
  oled.setUp();
  oled.clearDisplay();

  debugMsg("");
  debugMsg("Starting WiFi");
  oled.showScrollingMessage("Starting WiFi");

  WiFi.onEvent(WiFiEvent);

  WiFi.begin();

  debugMsg("");
  debugMsg("Connessione all'ultimo AP");
  oled.showScrollingMessage("Connect to AP");

  intervalWiFi = millis() + INTERVAL_WIFI;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (previousMillisWiFi < intervalWiFi)
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

  // Try WPS
  debugMsg("");
  debugMsg("Connect using WPS");
  oled.showScrollingMessage("Connect using WPS");
  intervalWiFi = millis() + INTERVAL_WPS;
  while (WiFi.status() != WL_CONNECTED)
  {
      wpsInitConfig();
      esp_wifi_wps_enable(&config);

    if (previousMillisWiFi < intervalWiFi)
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

  debugMsg("");
  debugMsg("Connesso a: " + WiFi.SSID());
  debugMsg("IP Address: " + WiFi.localIP().toString());
  debugMsg("MAC Address: " + WiFi.macAddress());
  debugMsg("Host name: " + String(WiFi.getHostname()));

  // Connected, show only the IP
  oled.clearDisplay();
  oled.showScrollingMessage("IP: " + WiFi.localIP().toString());
  
  debugMsg("Start up SPIFFS");

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    debugMsg("An Error has occurred while mounting SPIFFS");
    return;
  }

  debugMsg("Start up WebServer" );

  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");
  // server.on("/", HTTP_GET, mainHandler);
  // server.on("/style.css", HTTP_GET, cssHandler);
  server.on("/api/getSummary", HTTP_GET, getSummaryDataHandler);
  server.on("/api/getTimeserver", HTTP_GET, getTimeserverDataHandler);
  server.on("/api/postTimeserver", HTTP_POST, postTimeserverDataHandler);
  
  server.on("/api/putConfig", HTTP_GET, saveConfigDataHandler);

  server.on("/utils/resetWifi", HTTP_GET, resetWifiHandler);
  server.on("/utils/scanI2C", HTTP_GET, getI2CScanHandler);
  server.on("/utils/saveStats", HTTP_GET, saveStatsHandler);
/*  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
/    request->send(200, "text/plain", "Hello World");
/  }); */

  server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "text/plain", "The content you are looking for was not found.");
  });

  server.begin();

  debugMsg("Start up mDNS");

  if(!MDNS.begin("myesp32")) {
      debugMsg("Error starting mDNS");
      return;
  }

  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "prop1", "test");
  MDNS.addServiceTxt("http", "tcp", "prop2", "test2");

  debugMsg("Start up OTA");

  setupOTA();

  debugMsg("Start up NTP" );

  DebugCallback dbcb = debugMsg;
  ntpAsync.setDebugCallback(dbcb);
  ntpAsync.setDebugOutput(true);

  ntpAsync.setTZS(TIME_ZONE_STRING_DEFAULT);
  ntpAsync.setNtpPool(NTP_POOL_DEFAULT);
  ntpAsync.setUpdateInterval(NTP_UPDATE_INTERVAL_DEFAULT);


  NewTimeCallback ntcb = newTimeUpdateReceived;

  // set up the NTP component and wire up the "got time update" callback
  ntpAsync.setNewTimeCallback(ntcb);

  // kick off NTP updates
  unsigned long nowMillis = millis();
  ntpAsync.getTimeFromNTP(nowMillis);

  // Default pins SDA 21, SCL 22
  debugMsg("Start up I2C...");
  Wire.begin();

  debugMsg("Startup SPIFFS storage");
  spiffsStorage.setDebugCallback(dbcb);
  spiffsStorage.setDebugOutput(true);
  spiffsStorage.getStatsFromSpiffs(&current_stats);

  // debugMsg("Current uptime: " + String(current_stats.uptimeMins));

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

  char time_c[11];
  sprintf(time_c, "%02d:%02d:%02d", hour(), minute(), second());
  oled.setWiFiStatus(connected);
  oled.setBlankStatus(false);
  oled.setNTPStatus(ntpAsync.ntpTimeValid(nowMillis));
  oled.setTimeString(String(time_c));
  
    esp_task_wdt_reset();
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  debugMsg("---> OncePerMinuteProcessing");

  debugMsg("nu: " + String(ntpAsync.getNextUpdate(nowMillis)));

  // Usage stats
  current_stats.uptimeMins++;
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
  spiffsStorage.saveStatsToSpiffs(&current_stats);
}

void loop()
{
  ArduinoOTA.handle();

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
  
  delay(100);
}


