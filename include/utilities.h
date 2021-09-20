#ifndef utilities_h
#define utilities_h

#include <Arduino.h>
#include "defs.h"
#include "globals.h"
#include "NtpAsync.h"
#include <TimeLib.h>

#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <Wire.h>
#include "SpiffsStorage.h"
#include "LDRManager.h"

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

void debugMsg(String message);
void debugMsgCont(String message);
void newTimeUpdateReceived();

uint32_t decodeBCD(byte valueToDecode, bool bl1, bool bl2, bool led1, bool led2);

void getCredentialsHandler(AsyncWebServerRequest *request);
bool gotCredentials();
void wifiBeginWithCredentials();

void getSummaryDataHandler(AsyncWebServerRequest *request);
void getDiagsDataHandler(AsyncWebServerRequest *request);

void getTimeserverDataHandler(AsyncWebServerRequest *request);
void postTimeserverDataHandler(AsyncWebServerRequest *request);

void getConfigDataHandler(AsyncWebServerRequest *request);
void postConfigDataHandler(AsyncWebServerRequest *request);

void postWiFiDataHandler(AsyncWebServerRequest *request);
void resetWifiHandler(AsyncWebServerRequest *request);

void getI2CScanHandler(AsyncWebServerRequest *request);
void saveStatsHandler(AsyncWebServerRequest *request);

void restartHandler(AsyncWebServerRequest *request);

void resetWifi();
void resetOptions();
void resetAll();

void testRTCTimeProvider();
String getRTCTime(boolean setInternalTime);
void setRTCTime();

void calculateCurrentOffset(int year, int mon, int day, int hour, int min, int sec);

void loadNumberArrayTime();
void outputDisplay();

#endif