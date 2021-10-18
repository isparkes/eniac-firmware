#pragma once

#include <ESPAsyncWebServer.h>
#include "WiFi.h"
#include "defs.h"
#include "esp_wps.h"

#include "SpiffsStorage.h"          // Access to config objects
#include "BlinkenlightsManager.h"   // Access to blinkenlights

// ************************************************************
// Global shared components and objects
// ************************************************************
extern AsyncWebServer server;
extern esp_wps_config_t wps_config;

extern unsigned long nowMillis;
extern unsigned long lastMillis;
extern unsigned long previousMillisWiFi;
extern int lastSecond;
extern boolean triggeredThisSec;

// ************************************************************
// Shared config objects
// ************************************************************
extern spiffs_config_t* cc;
extern spiffs_stats_t* cs;

extern blinkelights_t* bl;

// ************************************************************
// Display values
// ************************************************************
extern volatile uint32_t val1;
extern volatile uint32_t val2;
extern volatile uint32_t val3;

extern volatile uint32_t nextVal1;
extern volatile uint32_t nextVal2;
extern volatile uint32_t nextVal3;

extern volatile uint8_t switchTime;

extern volatile uint16_t impressions;
extern portMUX_TYPE timerMux1;

extern byte numberArray[DIGIT_COUNT];
extern byte currNumberArray[DIGIT_COUNT];
extern byte displayType[DIGIT_COUNT];
extern int fadeState;
extern byte scrollCounter[DIGIT_COUNT];

// ************************************************************
// Variables for clock management
// ************************************************************
extern int blinkState;
extern float fadeStepsInternal;

extern int ldrValue;
extern unsigned int oledTime;

// ToDo move into outputManager
extern bool led1State;
extern bool led2State;
extern bool indLed1;
extern bool indLed2;

// ToDo move into NetworkManager
extern String ssid;
extern String password;
extern bool credentialsReceived;
