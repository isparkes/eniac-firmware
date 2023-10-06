#pragma once

#include <ESPAsyncWebServer.h>
#include "WiFi.h"
#include "Defs.h"
#include "esp_wps.h"

#ifdef FEATURE_BACKLIGHTS
#include "LEDManager.h"
#endif

#include "SpiffsStorage.h"          // Access to config objects

#ifdef FEATURE_BLINKENLIGHTS
#include "BlinkenlightsManager.h"   // Access to blinkenlights
#endif

#include "OutputManager.h"          // Defintions for number arrays
#include "StorageTypes.h"           // Config and Stats objects

// Meanings of switches
#define SW_NONE                 0
#define SW_COUNTDOWN_INHIBIT    1
#define SW_SLAVE_INHIBIT        2
#define SW_MIN_DIM              3
#define SW_DIM_LEDS             4

// ************************************************************
// Global shared components and objects
// ************************************************************
extern esp_wps_config_t wps_config;

extern unsigned long nowMillis;
extern unsigned long lastMillis;
extern unsigned long previousMillisWiFi;
extern int lastSecond;
extern boolean triggeredThisSec;
extern int secsDeltaAbs;
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

extern byte numberArray[DIGIT_COUNT];
extern byte currNumberArray[DIGIT_COUNT];
extern byte displayType[DIGIT_COUNT];
extern int fadeState;
extern byte scrollCounter[DIGIT_COUNT];

extern unsigned int oledTimeout;
extern unsigned int configTimeout;
extern unsigned int flashTimeout;

#ifdef DIGIT_DIAGNOSTICS
extern int digitValue;
#endif

extern bool doAutoReconnect;

extern byte switch1Meaning;
extern byte switch2Meaning;

extern String uniqHostname;

extern AsyncWebServer server;

extern String lastWiFiScan;

extern bool switchEventWaiting;

#ifdef COG_CRANK_OUTPUT
extern byte cogCrankSecsLeft;
#endif

// ------------------ Global functions -----------------

void updateNowMillis();
