#pragma once

#include "Defs.h"
#include "Arduino.h"
#include "TZManager.h"
#include "DebugManager.h"
#include "utilities.h"

// -------------------------------------------------------------------------------

#define GPS_READING_VALIDITY_SECS       240

// -------------------------------------------------------------------------------

class GPSManager_ {
  private:
    GPSManager_() = default; // Make constructor private

    GPSManager_(const GPSManager_ &) = delete; // no copying
    GPSManager_ &operator=(const GPSManager_ &) = delete;

  public:
    static GPSManager_ &getInstance(); // Accessor for singleton instance

  public:
    void setUp();
    void parseNMEAMsg(char c);
    unsigned long getLastGPSReadTime();
    bool getGPSTimeValid();
    bool getGPSSyncStarted();
    time_t getLastGPSTime();
    String getLastGPSTimeRaw();
  private:
    char _msgBuffer[37] = {0};
    byte _bufferOffset = 0;

    time_t _lastGPSTime = 0;
    String _lastGPSTimeRaw = "";
    unsigned long _lastGPSReadTime = 0;
    unsigned long _lastGPSSyncTime = 0;
    bool _gpsTimeValid = false;

    String parseGPZDAMsg(String messageToParse);
    bool parseGPZDAMsgToUTCTime(String messageToParse);
};

extern GPSManager_ &gpsManager;

