#pragma once

#include "defs.h"
#include "Arduino.h"
#include "globals.h"

// -------------------------------------------------------------------------------
#define MD_TIMEOUT_MIN                 60    // 1 minute in seconds
#define MD_TIMEOUT_MAX                 3600  // 1 hour in seconds
#define MD_TIMEOUT_DEFAULT             300   // 5 minutes in seconds

// -------------------------------------------------------------------------------
#define MD_BLANK_MIN                   0
#define MD_OVERRIDE_BLANK              0     // Motion detection overrides blanking period
#define MD_RESPECT_BLANK               1     // Motion detection will not trigger during blanking period
#define MD_DISABLE                     2     // Motion detection disabled
#define MD_BLANK_MAX                   2
#define MD_BLANK_DEFAULT               0

// -------------------------------------------------------------------------------
#define DAY_BLANKING_MIN                0
#define DAY_BLANKING_NEVER              0  // Don't blank ever (default)
#define DAY_BLANKING_WEEKEND            1  // Blank during the weekend
#define DAY_BLANKING_WEEKDAY            2  // Blank during weekdays
#define DAY_BLANKING_ALWAYS             3  // Always blank
#define DAY_BLANKING_HOURS              4  // Blank between start and end hour every day
#define DAY_BLANKING_WEEKEND_OR_HOURS   5  // Blank between start and end hour during the week AND all day on the weekend
#define DAY_BLANKING_WEEKDAY_OR_HOURS   6  // Blank between start and end hour during the weekends AND all day on week days
#define DAY_BLANKING_WEEKEND_AND_HOURS  7  // Blank between start and end hour during the weekend
#define DAY_BLANKING_WEEKDAY_AND_HOURS  8  // Blank between start and end hour during week days
#define DAY_BLANKING_MAX                8
#define DAY_BLANKING_DEFAULT            0

// -------------------------------------------------------------------------------
#define BLANK_MODE_MIN                  0
#define BLANK_MODE_TUBES                0  // Use blanking for tubes only 
#define BLANK_MODE_LEDS                 1  // Use blanking for LEDs only
#define BLANK_MODE_TUBES_LEDS           2  // Use blanking for tubes and LEDs
#define BLANK_MODE_ALL                  3  // Use blanking for tubes, LEDs and towers
#define BLANK_MODE_MAX                  
#define BLANK_MODE_DEFAULT              2

// -------------------------------------------------------------------------------
#define PIR_TIMEOUT_MIN                 60    // 1 minute in seconds
#define PIR_TIMEOUT_MAX                 3600  // 1 hour in seconds
#define PIR_TIMEOUT_DEFAULT             300   // 5 minutes in seconds

// #define USE_PIR_PULLUP_DEFAULT          true

// -------------------------------------------------------------------------------

class BlankingManager_ {
  private:
    BlankingManager_() = default; // Make constructor private

    unsigned long _mdTimeout = PIR_TIMEOUT_DEFAULT;
    unsigned long _pirLastSeen = 0;
    bool _pirInstalled = false;
    bool _pirvalue = false;
    bool _blanked;
    bool _blankTubes = false;
    bool _blankLEDs = false;
    bool _blankTowers = false;
    bool _pirBlanked;
    bool _timeBasedBlanked;

    bool checkPIR();
    bool checkTimeBasedBlanking(byte currentWeekday, byte currentHour);
    bool getHoursBlanked(byte currentHou);

  public:
    static BlankingManager_ &getInstance(); // Accessor for singleton instance

    BlankingManager_(const BlankingManager_ &) = delete; // no copying
    BlankingManager_ &operator=(const BlankingManager_ &) = delete;

  public:
    void begin();
    bool getBlankingStatus(byte currentWeekday, byte currentHour);
    bool getCurrentBlankTubes();
    bool getCurrentBlankLEDs();
    bool getCurrentBlankTowers();
    bool getCurrentPIRStatus();
    bool getCurrentPIRInstalled();
    bool getCurrentBlankingStatus();
    int  getBlankAge();
    String getBlankingReason();
};

extern BlankingManager_ &blankingManager;