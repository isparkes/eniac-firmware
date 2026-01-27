#include "CountdownManager.h"

#include <Arduino.h>
#include "DebugManager.h"
#include "TZManager.h"

void CountdownManager_::begin() {
  calculateTargetTime();
}

// ************************************************************
// See if we are to display the countdown value or not
// ************************************************************
bool CountdownManager_::getCountdownActive() {
  return _inCountdown && !_suppressCountdown;
}

// ************************************************************
// See if we are to display the countdown value or not - does
// NOT take into account the inhibit flag.
// ************************************************************
bool CountdownManager_::getCountdownActiveInternal() {
  return _inCountdown;
}

// ************************************************************
// Set the "getCountdownActive" to false so that we show the
// normal display
// ************************************************************
void CountdownManager_::setCountdownInhibit(bool newInhibitValue) {
  #ifdef CDM_EXTENDED_DEBUG
  if (_suppressCountdown != newInhibitValue) {
    debugMsgCdm("Setting countdown inhibit via switch to: " + String(newInhibitValue ? "true" : "false"));
  }
  #endif
  _suppressCountdown = newInhibitValue;
}

// ************************************************************
// Get state of the inhibit flag
// ************************************************************
bool CountdownManager_::getCountdownInhibit() {
  return _suppressCountdown;
}

// ************************************************************
// calculate the seconds left
// ************************************************************
void CountdownManager_::calculateCountdown() {
  time_t now = tzManager.getRawUTCTimeFromTimeSource(TIME_SOURCE_INT);
  unsigned long nowLong = (unsigned long) now;

  #ifdef CDM_EXTENDED_DEBUG
  debugMsgCdm("Target raw --> " + String(_tTargetLong));
  debugMsgCdm("Now raw --> " + String(nowLong));
  #endif

  _inCountdown = (_tTargetLong > nowLong);

  if (_inCountdown) {
    // show the countdown
    unsigned long diff = _tTargetLong - nowLong;
    #ifdef CDM_EXTENDED_DEBUG
    debugMsgCdm("Diff raw --> " + String(diff));
    #endif

    _units = COUNTDOWN_UNITS_SECS;

    if (diff > 999999) {
      diff = diff / 60;
      _units = COUNTDOWN_UNITS_MINS;
      #ifdef CDM_EXTENDED_DEBUG
      debugMsgCdm("Diff mins --> " + String(diff));
      #endif
    }
    if (diff > 999999) {
      diff = diff / 60;
      _units = COUNTDOWN_UNITS_HRS;
      #ifdef CDM_EXTENDED_DEBUG
      debugMsgCdm("Diff hours --> " + String(diff));
      #endif
    }
    if (diff > 999999) {
      diff = diff / 24;
      _units = COUNTDOWN_UNITS_DAYS;
      #ifdef CDM_EXTENDED_DEBUG
      debugMsgCdm("Diff days --> " + String(diff));
      #endif
    }
    if (diff > 999999) {
      diff = 999999;
      #ifdef CDM_EXTENDED_DEBUG
      debugMsgCdm("Diff max --> " + String(diff));
      #endif
    }

    _remainingUntilTarget = diff;

    #ifdef CDM_EXTENDED_DEBUG
    debugMsgCdm("Scoped diff --> " + String(_remainingUntilTarget) + ", scope: " + String(_units));
    #endif
  }
}

// ************************************************************
// Return the SCOPED number of units until we reach countdown
// The scope can be days, hours, minutes or seconds
// ************************************************************
unsigned int CountdownManager_::getRemaining() {
  return _remainingUntilTarget;
}

// ************************************************************
// Return theunits for the scope (days, hours, minutes or seconds)
// ************************************************************
byte CountdownManager_::getRemainingUnits() {
  return _units;
}

// ************************************************************
// Calculate the target time - called once at startup
// ************************************************************
void CountdownManager_::calculateTargetTime() {
  #ifdef COUNTDOWN
  struct tm targetTime;
  if (cc->countdownTarget.length() == 10) {
    targetTime.tm_year = cc->countdownTarget.substring(0,4).toInt() - 1900;
    targetTime.tm_mon = cc->countdownTarget.substring(5,7).toInt() - 1; 
    targetTime.tm_mday = cc->countdownTarget.substring(8,10).toInt(); 
    targetTime.tm_hour = 0; 
    targetTime.tm_min = 0;
    targetTime.tm_sec = 0;
    targetTime.tm_isdst = -1;
  }
  time_t tTargetTime = mktime(&targetTime);
  _tTargetLong = (unsigned long) tTargetTime;

  debugMsgCdm("target date U--> " + tzManager.gmtimeToReadableString(tTargetTime));
  #endif
}

CountdownManager_ &CountdownManager_::getInstance() {
  static CountdownManager_ instance;
  return instance;
}

CountdownManager_ &countdownManager = countdownManager.getInstance();