#pragma once

#include <Arduino.h>
#include "DebugManager.h"
#include "defs.h"

#define TIME_ZONE_STRING_DEFAULT  "CET-1CEST,M3.5.0,M10.5.0/3"

#define TIME_SOURCE_GPS           0
#define TIME_SOURCE_NTP           1
#define TIME_SOURCE_RTC           2
#define TIME_SOURCE_INT           3
#define TIME_SOURCE_COUNT         4

#define RTC_CACHE_TIME_SEC        60

class TZManager_ {
  private:
    TZManager_() = default; // Make constructor private

  public:
    static TZManager_ &getInstance(); // Accessor for singleton instance

    TZManager_(const TZManager_ &) = delete; // no copying
    TZManager_ &operator=(const TZManager_ &) = delete;

  public:
    void setTZS(String tzs);
    String getTZS();

    // Offset handling
    int  getCurrentUTCOffset();
    void calculateCurrentOffsetFromTimeT();

    String getLocalTimeFromTimeSource(byte timesource, unsigned long now);
    unsigned long getTimeLastSetFromTimeSource(byte timesource, unsigned long now);
    void setUTCTimeFromTimeSourceHourly(unsigned long now);
    void setUTCTimeFromTimeSource(byte timesource, unsigned long now, time_t gpsTime);
    byte getPrimaryTimeSource(unsigned long now);

    // Debug
    void setDebugOutput(bool newDebug);
    void setDebugCallback(DebugCallback dbcb);
  private:
    String _tzs = TIME_ZONE_STRING_DEFAULT;
    unsigned long _UTCoffset;
    time_t _utctime[TIME_SOURCE_COUNT];
    unsigned long _lastupdatetime[TIME_SOURCE_COUNT];
    byte _primarysource;
    bool _debug = false;
    DebugCallback _dbcb;

    void setInternalTime(unsigned long now);
    void debugMsg(String message);
};

extern TZManager_ &tzManager;