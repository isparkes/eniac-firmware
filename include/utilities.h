#pragma once

#include <Arduino.h>
#include "NtpAsync.h"
#include <TimeLib.h>

#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "SPIFFS.h"
#include <Wire.h>
#include "SpiffsStorage.h"

AsyncWebServer server(80);

#define COUNT0_MAX 1000
#define COUNT0_OFF 100

volatile int count0;
volatile int count0Max = COUNT0_MAX;
volatile int count0Off = COUNT0_OFF;
volatile int count1;

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

void getSummaryDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api summary GET request");
  
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

void getConfigDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api config GET request");
  
  JsonObject& data = spiffsStorage.getConfigAsJsonObject(&current_config);
  String responseString;
  data.printTo(responseString);

  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
  response->addHeader("Server", "ESP Async Web Server");
  request->send(response);  
}

void saveConfigDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api config PUT request");
  
  spiffs_config_t* cc = &current_config;

  ntpAsync.resetDefaults();
  cc->ntpPool = ntpAsync.getNtpPool();
  cc->ntpUpdateInterval = ntpAsync.getUpdateInterval();
  cc->tzs = ntpAsync.getTZS();

  cc->webUsername = "";
  cc->webPassword = "";

  spiffsStorage.saveConfigToSpiffs(cc);

  AsyncWebServerResponse *response = request->beginResponse(200, "plain/text", "OK");
  response->addHeader("Server", "ESP Async Web Server");
  request->send(response);  
}

void getTimeserverDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api timeserver GET request");
  
  spiffs_config_t* cc = &current_config;

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ntpPool"] = cc->ntpPool;
  root["ntpUpdateInterval"] = cc->ntpUpdateInterval;
  root["tzs"] = cc->tzs;
  response->setLength();
  request->send(response);
}

void dumpArgs(AsyncWebServerRequest *request) {
  int args = request->args();
  for(int i=0;i<args;i++){
    Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }  
}

void postTimeserverDataHandler(AsyncWebServerRequest *request) {
  debugMsg("Got api timeserver POST request");
  
  dumpArgs(request);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parse(String(request->arg("body")));

  spiffs_config_t* cc = &current_config;
  if (json.success()) {
    cc->ntpPool = json["ntpPool"].as<String>();
    debugMsg("Loaded NTP pool: " + cc->ntpPool);

    cc->ntpUpdateInterval = json["ntpUpdateInterval"].as<int>();
    debugMsg("Loaded NTP update interval: " + String(cc->ntpUpdateInterval));

    cc->tzs = json["tzs"].as<String>();
    debugMsg("Loaded time zone string: " + cc->tzs);
  } else {
    debugMsg("Json parse failure: " + String(request->arg("body")));
  }

  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();
  root["ntpPool"] = cc->ntpPool;
  root["ntpUpdateInterval"] = cc->ntpUpdateInterval;
  root["tzs"] = cc->tzs;
  response->setLength();
  request->send(response);
}

void getI2CScanHandler(AsyncWebServerRequest *request) {
  debugMsg("Got I2C scan request");
  
  AsyncJsonResponse * response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");
  JsonObject& root = response->getRoot();

  byte error, address;
  int nDevices;
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      debugMsg("I2C device found at address 0x" + String(address, HEX));
      root["I2C"+String(address)] = "found";
      nDevices++;
    }
    else if (error==4) {
      debugMsg("Unknown error at address 0x" + String(address, HEX));
    }    
  }
  if (nDevices == 0) {
    debugMsg("No I2C devices found");
  }
  else {
    debugMsg("done");
  }

  response->setLength();
  request->send(response);
}

void saveStatsHandler(AsyncWebServerRequest *request) {
  debugMsg("Got save stats request");

  spiffsStorage.saveStatsToSpiffs(&current_stats);
  
  request->send(200, "text/plain", "Stats saved");
}

void resetWifiHandler(AsyncWebServerRequest *request) {
  debugMsg("Got utils RESET request");
  WiFi.disconnect();
  request->send(200, "text/plain", "WiFi was reset");
}

void setLedFlashType(byte flashType) {
  switch(flashType) {
    case 0: {
      count0Max = 1000;
      count0Off = 1;
      break;
    }
    case 1: {
      count0Max = 1000;
      count0Off = 500;
      break;
    }
  }

}