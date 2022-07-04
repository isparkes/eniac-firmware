#include "CountdownManager.h"

#include <Arduino.h>
#include "DebugManager.h"
#include "TZManager.h"

void CountdownManager_::begin() {
  calculateTargetTime();
}

bool CountdownManager_::getCountdownActive() {
  return _inCoundown;
}

void CountdownManager_::calculateCountdown() {
  // calculate the seconds left
  time_t now = tzManager.getRawUTCTimeFromTimeSource(TIME_SOURCE_INT);
  unsigned long nowLong = (unsigned long) now;

  debugMsgCdm("Target raw --> " + String(_tTargetLong));
  debugMsgCdm("Now raw --> " + String(nowLong));

  _inCoundown = (_tTargetLong > nowLong);

  if (_inCoundown) {
    // show the countdown
    outputManager.setOutputMode(valueMode);
    unsigned long diff = _tTargetLong - nowLong;
    debugMsgCdm("Diff raw --> " + String(diff));

    _units = COUNTDOWN_UNITS_SECS;

    if (diff > 999999) {
      diff = diff / 60;
      _units = COUNTDOWN_UNITS_MINS;
      debugMsgCdm("Diff mins --> " + String(diff));
    }
    if (diff > 999999) {
      diff = diff / 60;
      _units = COUNTDOWN_UNITS_HRS;
      debugMsgCdm("Diff hours --> " + String(diff));
    }
    if (diff > 999999) {
      diff = diff / 24;
      _units = COUNTDOWN_UNITS_DAYS;
      debugMsgCdm("Diff days --> " + String(diff));
    }
    if (diff > 999999) {
      diff = 999999;
      debugMsgCdm("Diff max --> " + String(diff));
    }

    _remainingUntilTarget = diff;

    debugMsgCdm("Scoped diff --> " + String(_remainingUntilTarget));

    outputManager.allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    outputManager.loadNumberArrayValue(diff);
  }
}

unsigned int CountdownManager_::getRemaining() {
  return _remainingUntilTarget;
}

byte CountdownManager_::getRemainingUnits() {
  return _units;
}

void CountdownManager_::calculateTargetTime() {
  struct tm targetTime;
  if (cc->countdownTarget.length() == 10) {
    targetTime.tm_year = cc->countdownTarget.substring(0,4).toInt() - 1900;
    targetTime.tm_mon = cc->countdownTarget.substring(5,7).toInt() - 1; 
    targetTime.tm_mday = cc->countdownTarget.substring(8,10).toInt(); 
    targetTime.tm_hour = 0; 
    targetTime.tm_min = 0;
    targetTime.tm_sec = 0;
    targetTime.tm_isdst = tzManager.getCurrentUTCIsDST();
  }
  time_t tTargetTime = mktime(&targetTime) + tzManager.getCurrentUTCOffset();
  _tTargetLong = (unsigned long) tTargetTime;

  debugMsgCdm("target date U--> " + tzManager.gmtimeToReadableString(tTargetTime));
}

CountdownManager_ &CountdownManager_::getInstance() {
  static CountdownManager_ instance;
  return instance;
}

CountdownManager_ &countdownManager = countdownManager.getInstance();