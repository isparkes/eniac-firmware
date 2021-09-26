#include "BlankingManager.h"
#include <Arduino.h>

void BlankingManager_::begin() {
  pinMode(PIRPin, INPUT);
}

// ************************************************************
// Check the PIR status. If we don't have a PIR installed, we
// don't want to respect the pin value, because it would defeat
// normal day blanking. The first time the PIR takes the pin low
// we mark that we have a PIR and we should start to respect
// the sensor.
// Returns true if PIR sensor is installed and we are blanked
// --------------
// Uses the value of pirBlanking to avoid disturbing  blanking 
// period if so configured.
// ************************************************************
bool BlankingManager_::checkPIR(unsigned long nowMillis) {
  _pirvalue = (digitalRead(PIRPin) == HIGH);

  if (_pirvalue) {
    _pirLastSeen = nowMillis;
    return false;
  } else {
    // Note that we have a pir
    _pirInstalled = true;
    if (nowMillis > (_pirLastSeen + (cc->mdTimeout * 1000))) {
      return true;
    } else {
      return false;
    }
  }
}

// ************************************************************
// Check the blanking
// ************************************************************
bool BlankingManager_::checkTimeBasedBlanking(byte currentWeekday, byte currentHour) {
  switch (cc->dayBlanking) {
    case DAY_BLANKING_NEVER:
      _blanked = false;
      break;
    case DAY_BLANKING_HOURS:
      _blanked = getHoursBlanked(currentHour);
      break;
    case DAY_BLANKING_WEEKEND:
      _blanked = ((currentWeekday == 1) || (currentWeekday == 7));
      break;
    case DAY_BLANKING_WEEKEND_OR_HOURS:
      _blanked = ((currentWeekday == 1) || (currentWeekday == 7)) || getHoursBlanked(currentHour);
      break;
    case DAY_BLANKING_WEEKEND_AND_HOURS:
      _blanked = ((currentWeekday == 1) || (currentWeekday == 7)) && getHoursBlanked(currentHour);
      break;
    case DAY_BLANKING_WEEKDAY:
      _blanked = ((currentWeekday > 1) && (currentWeekday < 7));
      break;
    case DAY_BLANKING_WEEKDAY_OR_HOURS:
      _blanked = ((currentWeekday > 1) && (currentWeekday < 7)) || getHoursBlanked(currentHour);
      break;
    case DAY_BLANKING_WEEKDAY_AND_HOURS:
      _blanked = ((currentWeekday > 1) && (currentWeekday < 7)) && getHoursBlanked(currentHour);
      break;
    case DAY_BLANKING_ALWAYS:
      _blanked = true;
      break;
  }

  // default, should never reach here
  return _blanked;
}

// ************************************************************
// If we are currently blanked based on hours
// ************************************************************
bool BlankingManager_::getHoursBlanked(byte currentHour) {
  if (cc->blankHourStart > cc->blankHourEnd) {
    // blanking before midnight
    return ((currentHour >= cc->blankHourStart) || (currentHour < cc->blankHourEnd));
  } else if (cc->blankHourStart < cc->blankHourEnd) {
    // dim at or after midnight
    return ((currentHour >= cc->blankHourStart) && (currentHour < cc->blankHourEnd));
  } else {
    return currentHour == cc->blankHourStart;
  }
}

bool BlankingManager_::getBlankingStatus(unsigned long nowMillis, byte currentWeekday, byte currentHour) {
  _pirBlanked = checkPIR(nowMillis);
  _timeBasedBlanked = checkTimeBasedBlanking(currentWeekday, currentHour);

  if (cc->mdBlankMode == MD_RESPECT_BLANK) {
    // respect quiet period: use PIR when not in time based blanking
    _blanked = _timeBasedBlanked || _pirBlanked;
  } else {
    // normal blanking mode: if PIR is installed, use it
    if (_pirInstalled) {
      _blanked = _pirBlanked;
    } else {
      _blanked = _timeBasedBlanked;
    }
  }

  // decide if we are blanked or dimmed
  if (_blanked) {
    switch(cc->blankMode) {
      case BLANK_MODE_TUBES: {
        _blankTubes = true;
        _blankLEDs = false;
        break;
      }
      case BLANK_MODE_LEDS: {
        _blankTubes = false;
        _blankLEDs = true;
        break;
      }
      case BLANK_MODE_BOTH: {
        _blankTubes = true;
        _blankLEDs = true;
        break;
      }
    }
  } else {
    _blankTubes = false;
    _blankLEDs = false;
  }

  return _blanked;
}

bool BlankingManager_::getCurrentPIRInstalled() {
  return _pirInstalled;
}

bool BlankingManager_::getCurrentPIRStatus() {
  return _pirvalue;
}

bool BlankingManager_::getCurrentBlankingStatus() {
  return _blanked;
}

bool BlankingManager_::getCurrentBlankTubes() {
  return _blankTubes;
}

bool BlankingManager_::getCurrentBlankLEDs() {
  return _blankLEDs;
}

String BlankingManager_::getBlankingReason() {
  if (_blanked) {
    if (_pirBlanked) {
      return "Blanked by motion detector";
    } else {
      return "Blanked by time settings";
    }
  } else {
    return "Not blanked";
  }

}

int  BlankingManager_::getBlankAge(unsigned long nowMillis)
{
    int lastMotionDetection = (nowMillis - _pirLastSeen) / 1000.0;
    return lastMotionDetection;
}

BlankingManager_ &BlankingManager_::getInstance() {
  static BlankingManager_ instance;
  return instance;
}

BlankingManager_ &BlankingManager = BlankingManager.getInstance();