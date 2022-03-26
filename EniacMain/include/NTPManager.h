#pragma once

#include <memory>
#include <Arduino.h>
#include <AsyncUDP.h>
#include <WiFi.h>
#include <DNSServer.h>          //https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
#include "defs.h"               // for DEBUG setting
#include "DebugManager.h"
#include "globals.h"

// ------------------------ Types ------------------------

typedef void (*NewTimeCallback) ();

// ------------------------ NTP ------------------------

#define NTP_POOL_DEFAULT "pool.ntp.org"
#define NTP_PACKET_SIZE 48
#define NTP_UPDATE_INTERVAL_DEFAULT 7261
#define NTP_UPDATE_INTERVAL_MIN 60
#define NTP_UPDATE_INTERVAL_MAX 86400
#define NTP_RETRY_INTERVAL_MS 30000

class NtpManager_
{
  private:
    NtpManager_() = default; // Make constructor private

  public:
    static NtpManager_ &getInstance(); // Accessor for singleton instance

    NtpManager_(const NtpManager_ &) = delete; // no copying
    NtpManager_ &operator=(const NtpManager_ &) = delete;

  public:
    void setDebugOutput(bool newDebug);
    void getTimeFromNTP();
    bool getIsConnected();
    void resetDefaults();
    
    void setUpdateInterval(int updateInterval);
    int getUpdateInterval();
    
    String getNtpPool();
    void setNtpPool(String pool);
    
    unsigned long getLastUpdate();
    signed int getNextUpdate();
    void resetNextUpdate();
    
    time_t getLastTimeTFromServer();
    
    bool ntpTimeValid();
    
    // callbacks
    void setNewTimeCallback(NewTimeCallback ntcb);
  private:
    String _ntpPool = NTP_POOL_DEFAULT;                   // The pool name we are using
    unsigned long _ntpStarted = 0;                        // the millis the request was sent at
    time_t _ntpTime;                                      // The time we retrieved
    unsigned long _lastUpdateFromServer = 0;              // The last millis() we got an update at
    int _ntpUpdateInterval = NTP_UPDATE_INTERVAL_DEFAULT; // The interval between updates in SECONDS
    AsyncUDP _udp;
    NewTimeCallback _ntcb;

    // print a debug message to the callback
    void debugMsg(String message);
};

extern NtpManager_ &ntpManager;