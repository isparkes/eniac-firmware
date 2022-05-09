#pragma once

#include <Arduino.h>
#include "DebugManager.h"
#include "Defs.h"
#include "utilities.h"
#include "DebugManager.h"

#define TIME_ZONE_STRING_DEFAULT  "CET-1CEST,M3.5.0,M10.5.0/3"

#define TIME_SOURCE_GPS           0
#define TIME_SOURCE_NTP           1
#define TIME_SOURCE_RTC           2
#define TIME_SOURCE_INT           3
#define TIME_SOURCE_COUNT         4

#define RTC_CACHE_TIME_SEC        120

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
    int  getCurrentUTCIsDST();
    void calculateCurrentOffsetFromTimeT();

    String getLocalTimeFromTimeSource(byte timesource);
    unsigned long getTimeLastSetFromTimeSource(byte timesource);
    void setUTCTimeFromTimeSourceHourly();
    void setUTCTimeFromTimeSource(byte timesource, unsigned long readTime, time_t utcTime);
    byte getPrimaryTimeSource();
    time_t getRawUTCTimeFromTimeSource(byte timesource);

    String localtimeToReadableString(time_t timeToConvert);
    String gmtimeToReadableString(time_t timeToConvert);
    tm getRTCTimeAsLocalTimeTM();
    time_t convertLocalTimeTMToUTC(tm tmFrom);
  private:
    String _tzs = TIME_ZONE_STRING_DEFAULT;
    unsigned long _UTCoffset;
    bool _localtimeisDST;
    time_t _utctime[TIME_SOURCE_COUNT];
    unsigned long _lastupdatetime[TIME_SOURCE_COUNT];
    byte _primarysource = TIME_SOURCE_RTC;

    void setInternalTimeFromRTC();
};

extern TZManager_ &tzManager;