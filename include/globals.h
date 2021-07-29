#pragma once

#include "SpiffsStorage.h"
#include "NtpAsync.h"
#include <ESPAsyncWebServer.h>
#include "WiFi.h"

unsigned long previousMillisWiFi = 0;

spiffs_config_t* cc = &current_config;

spiffs_stats_t* cs = &current_stats;

// AsyncWebServer server(80);

// NtpAsync ntpAsync;

volatile uint32_t val1 = 0;
volatile uint32_t val2 = 0;
volatile uint32_t val3 = 0;
