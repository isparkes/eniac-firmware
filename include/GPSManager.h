#pragma once

#include "defs.h"
#include "Arduino.h"
#include "TZManager.h"
#include "DebugManager.h"

// -------------------------------------------------------------------------------

#define GPS_READING_VALIDITY_SECS       240

// -------------------------------------------------------------------------------

class GPSManager_ {
  public:
    static GPSManager_ &getInstance(); // Accessor for singleton instance
    void parseNMEAMsg(char c, unsigned long nowMillis);
    unsigned long getLastGPSReadTime();
    bool getGPSTimeValid(unsigned long nowMillis);
    bool getGPSSyncStarted(unsigned long nowMillis);
    String getLastGPSTime();
    String getLastGPSTimeRaw();

    // Turn off or on logging
    void setDebugOutput(bool newDebug);
    
    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  private:
    // Singleton constructor, no copying
    GPSManager_() = default;
    GPSManager_(const GPSManager_ &) = delete; 
    GPSManager_ &operator=(const GPSManager_ &) = delete;

    char _msgBuffer[37];
    byte _bufferOffset = 0;

    String _lastGPSTime = "";
    String _lastGPSTimeRaw = "";
    unsigned long _lastGPSReadTime = 0;
    unsigned long _lastGPSSyncTime = 0;
    bool _gpsTimeValid = false;

    DebugCallback _dbcb;
    bool _debug = false;

    void debugMsg(String message);                        // print a debug message to the callback
    String parseGPZDAMsg(String messageToParse);
    String parseGPZDAMsgToLocaltime(String messageToParse);
};

extern GPSManager_ &gpsManager;

