#pragma once

#include <Arduino.h>
#include "NtpAsync.h"
#include <TimeLib.h>

#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "SPIFFS.h"

AsyncWebServer server(80);

void debugMsg(String message) {
    Serial.println(message);
    Serial.flush();
}

unsigned long lastMillis = 0;
unsigned long nowMillis = millis();
int lastSecond = 0;
boolean triggeredThisSec = false;

NtpAsync ntpAsync;

// -------------------------------------------------------------------------------
#define SYNC_HOURS 3
#define SYNC_MINS 4
#define SYNC_SECS 5
#define SYNC_DAY 2
#define SYNC_MONTH 1
#define SYNC_YEAR 0

// ************************************************************
// Set the time from the value we get back from the time server
// ************************************************************
void grabInts(String s, int *dest, String sep) {
  int end = 0;
  for (int start = 0; end != -1; start = end + 1) {
    end = s.indexOf(sep, start);
    if (end > 0) {
      *dest++ = s.substring(start, end).toInt();
    } else {
      *dest++ = s.substring(start).toInt();
    }
  }
}

// ************************************************************
// Set the time from the value we get back from the time server
// ************************************************************
void setTimeFromServer(String timeString) {
  int intValues[6];
  grabInts(timeString, &intValues[0], ",");
  setTime(intValues[SYNC_HOURS], intValues[SYNC_MINS], intValues[SYNC_SECS], intValues[SYNC_DAY], intValues[SYNC_MONTH], intValues[SYNC_YEAR]);
  debugMsg("Set internal time to NTP time: " + String(year()) + ":" + String(month()) + ":" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
}

 // ************************************************************
  // Callback: When the NTP component tells us there is an update
  // go and get it
  // ************************************************************
  void newTimeUpdateReceived() {
    debugMsg("Got a new time update: " + ntpAsync.getLastTimeFromServer());
    setTimeFromServer(ntpAsync.getLastTimeFromServer());
  }

void mainHandler(AsyncWebServerRequest *request) {
	debugMsg("Got request");
	request->send(SPIFFS, "/web/index.html");
}

void cssHandler(AsyncWebServerRequest *request) {
	debugMsg("Got css request");
	request->send(SPIFFS, "/web/style.css");
}

String timeToReadableString(int y, int m, int d, int h, int mi, int s) {
  char buf1[20];
  sprintf(buf1, "%04d:%02d:%02d %02d:%02d:%02d", y, m, d, h, mi, s);
  return String(buf1);
}

String timeStringToReadableString(String timeString){
  char* ptr = strtok((char *)timeString.c_str(), ",");
  int y = atoi(ptr);
  ptr = strtok(NULL, ",");
  int m = atoi(ptr);
  ptr = strtok(NULL, ",");
  int d = atoi(ptr);
  ptr = strtok(NULL, ",");
  int h = atoi(ptr);
  ptr = strtok(NULL, ",");
  int mi = atoi(ptr);
  ptr = strtok(NULL, ",");
  int s = atoi(ptr);
  return timeToReadableString(y,m,d,h,mi,s);
}

void getConfigHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api GET request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ip"] = WiFi.localIP().toString();
  root["mac"] = WiFi.macAddress();
  root["ntp-pool"] = ntpAsync.getTZS();
  String clockUrl = "http://" + String(WiFi.getHostname()) + ".local";
  clockUrl.toLowerCase();
  root["clock-url"] = clockUrl;
  root["last-ntp-time"] = timeStringToReadableString(ntpAsync.getLastTimeFromServer());
  root["heap"] = ESP.getFreeHeap();
  root["ssid"] = WiFi.SSID();
  response->setLength();
  request->send(response);
}