#pragma once

#include "defs.h"
#include "Arduino.h"
#include "TZManager.h"

// -------------------------------------------------------------------------------

#define GPS_READING_VALIDITY_SECS       240

// -------------------------------------------------------------------------------

typedef void (*DebugCallback) (String);

class GPSManager {
  public:
    static GPSManager &getInstance(); // Accessor for singleton instance
    void parseNMEAMsg(char c, unsigned long nowMillis);
    void setDebugOutput(bool newDebug);
    unsigned long getLastGPSReadTime();
    bool getGPSTimeValid(unsigned long nowMillis);
    bool getGPSSyncStarted(unsigned long nowMillis);
    String getLastGPSTime();
    String getLastGPSTimeRaw();

    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  private:
    // Singleton constructor, no copying
    GPSManager() = default;
    GPSManager(const GPSManager &) = delete; 
    GPSManager &operator=(const GPSManager &) = delete;

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

extern GPSManager &gpsManager;

