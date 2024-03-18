#pragma once

#include <ESPAsyncWebServer.h>
#include "WiFi.h"
#include "Defs.h"
#include "esp_wps.h"

#include "SpiffsStorage.h"          // Access to config objects

#ifdef FEATURE_BLINKENLIGHTS
#include "BlinkenlightsManager.h"   // Access to blinkenlights
#endif

#include "StorageTypes.h"           // Config and Stats objects

// Meanings of switches
#define SW_NONE                 0
#define SW_SLAVE_INHIBIT        1
#define SW_MIN_DIM              2
#define SW_DIM_LEDS             3
#define SW_COUNTDOWN_INHIBIT    4
#define SW1_DEFAULT             SW_DIM_LEDS
#define SW2_DEFAULT             SW_SLAVE_INHIBIT

// ************************************************************
// Global shared components and objects
// ************************************************************
extern esp_wps_config_t wps_config;

extern unsigned long nowMillis;
extern unsigned long lastSecondStartMillis;
extern unsigned long previousMillisWiFi;
extern int lastSecond;
extern boolean triggeredThisSec;
extern int secsDeltaAbs;
extern int secsDelta;
extern bool upOrDown;  

// ************************************************************
// Shared config objects
// ************************************************************
extern spiffs_config_t* cc;
extern spiffs_stats_t* cs;

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

extern portMUX_TYPE encoderMux;

#ifdef DIGIT_DIAGNOSTICS
extern int digitValue;
#endif

extern bool doAutoReconnect;

extern String uniqHostname;

extern AsyncWebServer server;

extern String lastWiFiScan;

extern bool switchEventWaiting;

#ifdef COG_CRANK_OUTPUT
extern byte cogCrankSecsLeft;
#endif

// ------------------ Global functions -----------------

void updateNowMillis();
