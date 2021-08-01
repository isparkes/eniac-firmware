#pragma once

#include "SpiffsStorage.h"
#include "NtpAsync.h"
#include <ESPAsyncWebServer.h>
#include "WiFi.h"
#include "defs.h"

extern byte numberArray[DIGIT_COUNT];
extern AsyncWebServer server;

extern NtpAsync ntpAsync;

extern SPIFFS_CLOCK spiffsStorage;

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