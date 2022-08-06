#pragma once

#include "Arduino.h"

#define COUNTDOWN_UNITS_SECS      0
#define COUNTDOWN_UNITS_MINS      1
#define COUNTDOWN_UNITS_HRS       2
#define COUNTDOWN_UNITS_DAYS      3

class CountdownManager_ {
  private:
    CountdownManager_() = default; // Make constructor private

  public:
    static CountdownManager_ &getInstance(); // Accessor for singleton instance

    CountdownManager_(const CountdownManager_ &) = delete; // no copying
    CountdownManager_ &operator=(const CountdownManager_ &) = delete;

  public:
    void begin();
    bool getCountdownActive();
    bool getCountdownActiveInternal();
    unsigned int getRemaining();
    byte getRemainingUnits();
    void calculateCountdown();
    void calculateTargetTime();
    void setCountdownInhibit(bool inhibitValue);
    bool getCountdownInhibit();
  private:
    unsigned long _tTargetLong;
    unsigned long _remainingUntilTarget;
    byte _units;
    bool _inCountdown;
    bool _suppressCountdown = false;
};

extern CountdownManager_ &countdownManager;