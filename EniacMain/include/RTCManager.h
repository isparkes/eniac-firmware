#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "utilities.h"
#include <TimeLib.h>
#include "DebugManager.h"

// ----------------------- Defines -----------------------

#define RtcManager_I2C_ADDRESS 0x68

#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6
#define SUN 7

// ----------------------------------------------------------------------------------------------------
// ------------------------------------------- RTC Component ------------------------------------------
// ----------------------------------------------------------------------------------------------------

class RtcManager_ {
  public:
    static RtcManager_ &getInstance(); // Accessor for singleton instance
    RtcManager_(const RtcManager_ &) = delete; // no copying
    RtcManager_ &operator=(const RtcManager_ &) = delete;

    void begin();
    void startClock(void);
    void stopClock(void);
    uint8_t isRunning();

    bool testRTCTimeProvider();
    time_t getRTCTimeAsTimeT();    
    bool getRTCValid();

    bool setTimeFromUTCSource(time_t currentTime);
    unsigned long getLastRTCSetTime();
private:
    RtcManager_() = default; // Make constructor private

    uint8_t decToBcd(uint8_t val);
    uint8_t bcdToDec(uint8_t val);
    bool setTimeRTCHardware();
    bool getTimeRTCHardware();

    bool _useRTC = false;
    bool _onceHadAnRTC = false;
    uint8_t _second;
    uint8_t _minute;
    uint8_t _hour; 
    uint8_t _dayOfWeek;// day of week, 1 = Monday
    uint8_t _dayOfMonth;
    uint8_t _month;
    uint16_t _year;
};

extern RtcManager_ &rtcManager;
