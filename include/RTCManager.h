#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "defs.h"
#include "utilities.h"
#include <TimeLib.h>
#include "DebugManager.h"

// ----------------------- Defines -----------------------

#define DS1307_I2C_ADDRESS 0x68

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

class DS1307_ {
  public:
    static DS1307_ &getInstance(); // Accessor for singleton instance
    DS1307_(const DS1307_ &) = delete; // no copying
    DS1307_ &operator=(const DS1307_ &) = delete;

    void begin();
    void startClock(void);
    void stopClock(void);
    uint8_t isRunning();

    bool testRTCTimeProvider();
    time_t getRTCTimeAsTimeT();    
    bool getRTCValid();

    void setTimeFromUTCSource(time_t currentTime, bool updateRTC);
    unsigned long getLastRTCSetTime();
    
    // Turn off or on logging
    void setDebugOutput(bool newDebug);
    
    // callbacks
    void setDebugCallback(DebugCallback dbcb);
private:
    DS1307_() = default; // Make constructor private

    uint8_t decToBcd(uint8_t val);
    uint8_t bcdToDec(uint8_t val);
    void setTimeRTCHardware(void);
    void getTimeRTCHardware(void);
    void fillByHMS(uint8_t _hour, uint8_t _minute, uint8_t _second);
    void fillByYMD(uint8_t _year, uint8_t _month, uint8_t _day);
    void fillDayOfWeek(uint8_t _dow);

    bool _useRTC = false;
    uint8_t _second;
    uint8_t _minute;
    uint8_t _hour; 
    uint8_t _dayOfWeek;// day of week, 1 = Monday
    uint8_t _dayOfMonth;
    uint8_t _month;
    uint16_t _year;

    DebugCallback _dbcb;
    bool _debug = false;

    void debugMsg(String message);                        // print a debug message to the callback
};

extern DS1307_ &rtcManager;
