#pragma once

#include "defs.h"
#include "Arduino.h"
#include "globals.h"

#define MD_TIMEOUT_MIN                 60    // 1 minute in seconds
#define MD_TIMEOUT_MAX                 3600  // 1 hour in seconds
#define MD_TIMEOUT_DEFAULT             300   // 5 minutes in seconds

#define MD_BLANK_MIN                   0
#define MD_OVERRIDE_BLANK              0     // Motion detection overrides blanking period
#define MD_RESPECT_BLANK               1     // Motion detection will not trigger during blanking period
#define MD_DISABLE                     2     // Motion detection disabled
#define MD_BLANK_MAX                   2
#define MD_BLANK_DEFAULT               0

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