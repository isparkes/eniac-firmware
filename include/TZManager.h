#pragma once

#include <Arduino.h>
#include "DebugManager.h"

// TZ manager deals with the application of the Time Zone and DST to UTC

class TZManager_ {
  private:
    TZManager_() = default; // Make constructor private

  public:
    static TZManager_ &getInstance(); // Accessor for singleton instance

    TZManager_(const TZManager_ &) = delete; // no copying
    TZManager_ &operator=(const TZManager_ &) = delete;

  public:
    void setDebugOutput(bool newDebug);
    void begin();
    void calculateCurrentOffset(int year, int mon, int day, int hour, int min, int sec);
    int  getCurrentUTCOffset();
    void setUTCTimeFromNTP(time_t ntpTime);
    void setUTCTimeFromGPS(time_t gpsTime);

    void setDebugCallback(DebugCallback dbcb);
  private:
    unsigned long _UTCoffset;
    time_t _ntptime;
    time_t _gpstime;
    bool _debug = false;
    DebugCallback _dbcb;

    void debugMsg(String message);
    void calculateCurrentOffsetFromTimeT(time_t now);
};

extern TZManager_ &tzManager;