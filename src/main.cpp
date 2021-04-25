/*
  Example Code To Get ESP32 To Connect To A Router Using WPS
  ===========================================================
  This example code provides both Push Button method and Pin
  based WPS entry to get your ESP connected to your WiFi router.

  Hardware Requirements
  ========================
  ESP32 and a Router having atleast one WPS functionality

  This code is under Public Domain License.

  Author:
  Pranav Cherukupalli 
*/

//---------------------------------- VARIABILI PER IL CONTROLLO DELLA CONNESSIONE ATTIVA ---------------------------------
unsigned long previousMillisWiFi = 0; // Memorizza l'ultimo volta che l'evento è stato aggiornato
long intervalWiFi = 6000;             // Variabile per l'intervallo di tempo tra un tentativo di connessione e l'altro.

#include "WiFi.h"
#include <ESPmDNS.h>
#include "esp_wps.h"
#include <ArduinoOTA.h>
#include "utilities.h"

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

volatile int count;
int totalInterrupts;

#define LED_PIN 2

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Code with critica section
void IRAM_ATTR onTimer0() {
   portENTER_CRITICAL_ISR(&timerMux);
   count++;
   portEXIT_CRITICAL_ISR(&timerMux);
}

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
    Serial.println("Station Mode Started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("Connected to :" + String(WiFi.SSID()));
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from station, attempting reconnection");
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
    esp_wifi_wps_disable();
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    Serial.println("WPS Failed, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    Serial.println("WPS Timedout, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    Serial.println("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
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
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  Serial.println("Start up OTA: " + ArduinoOTA.getHostname());
  ArduinoOTA.begin();
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println("Starting WiFi");

  WiFi.begin();

  Serial.print("Connessione all'ultimo AP");

  while (WiFi.status() != WL_CONNECTED)
  {
    if (previousMillisWiFi < intervalWiFi)
    {
      previousMillisWiFi = millis();
      Serial.print(".");

      delay(500);
    }
    else
      break;
  }

  if (WiFi.status() != WL_CONNECTED)
  {

    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);

    Serial.println("Starting WPS");

    wpsInitConfig();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
  }
  else
  {
    Serial.println();
    Serial.print("Connesso a: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }

  debugMsg("Start up mDNS" );

  if(!MDNS.begin("esp32")) {
      Serial.println("Error starting mDNS");
      return;
  }
  MDNS.addService("http","tcp",80);

  setupOTA();

  debugMsg("Start up SPIFFS" );

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  debugMsg("Start up WebServer" );

  // server.serveStatic("/", SPIFFS, "/web/");
  server.on("/", HTTP_GET, mainHandler);
  server.on("/style.css", HTTP_GET, cssHandler);
  
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });

  server.begin();

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

  debugMsg("Start up Timer" );
  
  // Configure LED output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer0, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);
}


// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  if (ntpAsync.getNextUpdate(nowMillis) < 0) {
    ntpAsync.getTimeFromNTP(nowMillis);
  }  
  debugMsg("count: " + String(count));
  digitalWrite(LED_PIN, second() % 2 == 0);
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  debugMsg("---> OncePerMinuteProcessing");

  debugMsg("nu: " + String(ntpAsync.getNextUpdate(nowMillis)));
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  debugMsg("---> OncePerHourProcessing");
}

// ************************************************************
// Called once per day
// ************************************************************
void performOncePerDayProcessing() {
  debugMsg("---> OncePerDayProcessing");
  // spiffs.saveStatsToSpiffs(&current_stats);
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

  // -------------------------------------------------------------------------------
  
  delay(100);
}


