#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <TimeLib.h>
#include <rom/rtc.h>

#include "Defs.h"
#include "Globals.h"

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <Wire.h>
#include "SpiffsStorage.h"
#include "LDRManager.h"
#include "TimerManager.h"
#include "Globals.h"
#include "GPSManager.h"
#include "BlankingManager.h"
#include "RTCManager.h"
#include "NTPManager.h"
#include "DebugManager.h"
#include <esp_task_wdt.h>
#include "WiFiManager.h"
#include "OutputManager.h"
#ifdef NIXIE_SLAVE
#include "SlaveManagerNixie.h"
#endif
#ifdef DECATRON_SLAVE
#include "SlaveManagerDecatron.h"
#endif

// -------------------------------------------------------------------------------

#define SYNC_HOURS 3
#define SYNC_MINS 4
#define SYNC_SECS 5
#define SYNC_DAY 2
#define SYNC_MONTH 1
#define SYNC_YEAR 0

const uint32_t DECODE_DIGIT[] = { 0x0200, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100};
const uint32_t DECODE_LED[]          = { 0x800000, 0x400000};
const uint32_t DECODE_BLINKENIGHTS[] = { 0x200000, 0x100000};

void newTimeUpdateReceived();

// Formatting routines
String timeToReadableStringFromTm(tm timeToFormat);
uint32_t decodeBCD(byte valueToDecode, bool bl1, bool bl2, bool led1, bool led2);


// Web handlers
void getCredentialsHandler(AsyncWebServerRequest *request);
void getWiFiNetworksHandler(AsyncWebServerRequest *request);
void saveWiFiCredentials(String newWiFiSSID, String newWiFiPassword);

void getSummaryDataHandler(AsyncWebServerRequest *request);
void getDiagsDataHandler(AsyncWebServerRequest *request);
void postDiagsDataHandler(AsyncWebServerRequest *request);

void getTimeserverDataHandler(AsyncWebServerRequest *request);
void postTimeserverDataHandler(AsyncWebServerRequest *request);
void getZonesListDataHandler(AsyncWebServerRequest *request);

void getConfigDataHandler(AsyncWebServerRequest *request);
void postConfigDataHandler(AsyncWebServerRequest *request);

void postWiFiCredentialsHandler(AsyncWebServerRequest *request);
void resetWifiHandler(AsyncWebServerRequest *request);

void getI2CScanHandler(AsyncWebServerRequest *request);
void getSPIFFSScanHandler(AsyncWebServerRequest *request);
void saveStatsHandler(AsyncWebServerRequest *request);

void restartHandler(AsyncWebServerRequest *request);

void postValueHandler(AsyncWebServerRequest *request);

void wifiBeginWithCredentials();
bool gotCredentials();

void resetWiFi();
void resetOptions();
void resetAll();

void enableWatchdog();
void disableWatchdog();
void feedWatchdog();

#ifdef FEATURE_TICKERS
int getBTCPrice();
#endif
