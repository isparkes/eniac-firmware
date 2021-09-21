#pragma once

#include "SpiffsStorage.h"
#include "NtpAsync.h"
#include <ESPAsyncWebServer.h>
#include "WiFi.h"
#include "defs.h"
#include "esp_wps.h"
#include <ESP32Encoder.h>
#include "ESP_DS1307.h"

extern byte numberArray[DIGIT_COUNT];
extern AsyncWebServer server;

extern NtpAsync ntpAsync;

extern esp_wps_config_t wps_config;

extern SPIFFS_CLOCK spiffsStorage;

extern ESP32Encoder encoder;

extern DS1307 rtclock;

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

extern volatile uint32_t nextVal1;
extern volatile uint32_t nextVal2;
extern volatile uint32_t nextVal3;

extern volatile uint8_t switchTime;

extern volatile uint16_t impressions;
extern volatile uint16_t outputs1;
extern volatile uint16_t outputs2;
extern volatile uint16_t outputs3;
extern volatile uint16_t switches1;
extern volatile uint16_t switches2;
extern volatile uint16_t switches3;

extern int blinkState;
extern float fadeStepsInternal;

extern byte numberArray[DIGIT_COUNT];
extern byte currNumberArray[DIGIT_COUNT];
extern byte displayType[DIGIT_COUNT];
extern int fadeState;
extern byte scrollCounter[DIGIT_COUNT];

extern String ssid;
extern String password;
extern bool credentialsReceived;

extern int ldrValue;

extern bool blanked;
extern bool blankTubes;
extern bool blankLEDs;
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

extern bool useRTC;

extern String lastGPSTime;
extern String lastGPSTimeRaw;
extern unsigned long lastGPSReadTime;
extern bool gpsTimeValid;

extern String lastRTCTime;
extern unsigned long lastRTCReadTime;