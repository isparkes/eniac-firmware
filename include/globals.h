#pragma once

#include "SpiffsStorage.h"
#include "NtpAsync.h"
#include <ESPAsyncWebServer.h>
#include "WiFi.h"
#include "defs.h"
#include "esp_wps.h"
#include <ESP32Encoder.h>

extern byte numberArray[DIGIT_COUNT];
extern AsyncWebServer server;

extern NtpAsync ntpAsync;

extern esp_wps_config_t wps_config;

extern SPIFFS_CLOCK spiffsStorage;

extern ESP32Encoder encoder;

extern unsigned long nowMillis;
extern unsigned long lastMillis;
extern unsigned long previousMillisWiFi;
extern int lastSecond;
extern boolean triggeredThisSec;

extern spiffs_config_t* cc;
extern spiffs_stats_t* cs;

extern volatile uint32_t val1;
extern volatile uint32_t val2;
extern volatile uint32_t val3;

extern String ssid;
extern String password;
extern bool credentialsReceived;

extern int ldrValue;

extern bool blanked;
extern unsigned int oledTime;

// ToDo move into outputManager
extern bool led1State;
extern bool led2State;
extern bool indLed1;
extern bool indLed2;

extern bool bl1;
extern bool bl2;
extern bool bl3;
extern bool bl4;
extern bool bl5;
extern bool bl6;
