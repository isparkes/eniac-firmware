#pragma once

#include "Defs.h"
#include "Arduino.h"
#include "Globals.h"
#include "DebugManager.h"
#include "TimeLib.h"
#include "OutputManager.h"
#include "SlaveManagerNixie.h"
#include "LEDManager.h"

// -------------------------------------------------------------------------------
enum MotionDetectionMode {
    MD_OVERRIDE_BLANK = 0,  // Motion detection overrides blanking period
    MD_RESPECT_BLANK = 1,   // Motion detection will not trigger during blanking period
    MD_DISABLE = 2          // Motion detection disabled
};

enum DayBlankingMode {
    DAY_BLANKING_NEVER = 0,              // Don't blank ever (default)
    DAY_BLANKING_WEEKEND = 1,            // Blank during the weekend
    DAY_BLANKING_WEEKDAY = 2,            // Blank during weekdays
    DAY_BLANKING_ALWAYS = 3,             // Always blank
    DAY_BLANKING_HOURS = 4,              // Blank between start and end hour every day
    DAY_BLANKING_WEEKEND_OR_HOURS = 5,   // Blank between start and end hour during the week AND all day on the weekend
    DAY_BLANKING_WEEKDAY_OR_HOURS = 6,   // Blank between start and end hour during the weekends AND all day on weekdays
    DAY_BLANKING_WEEKEND_AND_HOURS = 7,  // Blank between start and end hour during the weekend
    DAY_BLANKING_WEEKDAY_AND_HOURS = 8   // Blank between start and end hour during weekdays
};

enum BlankingMode {
    BLANK_MODE_TUBES = 0,       // Use blanking for tubes only
    BLANK_MODE_LEDS = 1,        // Use blanking for LEDs only
    BLANK_MODE_TUBES_LEDS = 2,  // Use blanking for tubes and LEDs
    BLANK_MODE_ALL = 3          // Use blanking for tubes, LEDs, and towers
};

// -------------------------------------------------------------------------------
#define MD_TIMEOUT_MIN       60    // 1 minute in seconds
#define MD_TIMEOUT_MAX       3600  // 1 hour in seconds
#define MD_TIMEOUT_DEFAULT   300   // 5 minutes in seconds

#define PIR_TIMEOUT_MIN       60    // 1 minute in seconds
#define PIR_TIMEOUT_MAX       3600  // 1 hour in seconds
#define PIR_TIMEOUT_DEFAULT   300   // 5 minutes in seconds

class BlankingManager_ {
  public:
    static BlankingManager_ &getInstance(); // Accessor for singleton instance

    BlankingManager_(const BlankingManager_ &) = delete; // No copying
    BlankingManager_ &operator=(const BlankingManager_ &) = delete;

  private:
    BlankingManager_() = default; // Private constructor

  public:
    void begin();
    void updateBlankingStatus();
    bool getCurrentPIRStatus();
    bool getCurrentPIRInstalled();
    bool getCurrentBlankingStatus();
    bool getCurrentBlankingIndicator();
    void setCurrentLEDBlankingOverride(bool newLEDOverrideStatus);
    String getNextBlankingModeName();
    DayBlankingMode getNextBlankingMode();
    bool getCurrentModeWantsHours();
    int getBlankAge();
    String getBlankingReason();

  private:
    unsigned long _mdTimeout = PIR_TIMEOUT_DEFAULT;
    unsigned long _pirLastSeen = 0;
    bool _pirInstalled = false;
    bool _pirvalue = false;
    bool _blanked;
    bool _pirBlanked;
    bool _timeBasedBlanked;
    unsigned int _pirBlankingPct;

    bool _blankLEDoverride = false;

    bool _blankTubes = false;
    bool _blankLEDs = false;
    bool _blankTowers = false;

    bool _PrevBlankTubes = false;
    bool _PrevBlankLEDs = false;
    bool _PrevBlankTowers = false;

    bool checkPIR();
    bool checkTimeBasedBlanking(byte currentWeekday, byte currentHour);
    bool getHoursBlanked(byte currentHour);

    void triggerTubeBlankChange(bool newStatus);
    void triggerLEDBlankChange(bool newStatus);
    void triggerTowerBlankChange(bool newStatus);
};

extern BlankingManager_ &blankingManager;