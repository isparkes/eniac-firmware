#include "BlankingManager.h"

// ************************************************************
// Set up
// ************************************************************
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
bool BlankingManager_::checkPIR() {
  _pirvalue = (digitalRead(PIRPin) == HIGH);

  #ifdef BLK_EXTENDED_DEBUG
  debugMsgBlk("PIR reading: " + String(_pirvalue));
  #endif
  bool pirBlanked = false;

  if (_pirvalue) {
    _pirLastSeen = nowMillis;
    _pirBlankingPct = 0;
  } else {
    // Note that we have a pir
    _pirInstalled = true;
    unsigned int lastMotionDetection = (nowMillis - _pirLastSeen) / 1000;
    if (lastMotionDetection > 0) {
      _pirBlankingPct = lastMotionDetection  * 1000 / cc->mdTimeout;
    } else {
      _pirBlankingPct = 0;
    }
    if (nowMillis > (_pirLastSeen + (cc->mdTimeout * 1000))) {
      pirBlanked = true;
    }
  }

  #ifdef BLK_EXTENDED_DEBUG
  debugMsgBlk("PIR based blanking: " + String(pirBlanked));
  #endif

  return pirBlanked;
}

// ************************************************************
// Check the blanking based on the day/time
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

  #ifdef BLK_EXTENDED_DEBUG
  debugMsgBlk("Time based blanking: " + String(_blanked));
  #endif

  // default, should never reach here
  return _blanked;
}

// ************************************************************
// Set if we are overriding the LED blanking via switch
// Only "true" has a meaning - this will blank the LEDs
// ************************************************************
void BlankingManager_::setCurrentLEDBlankingOverride(bool newLEDOverrideStatus) {
  _blankLEDoverride = newLEDOverrideStatus;
  updateBlankingStatus();
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

// ************************************************************
// Get the overall blanking status
// ************************************************************
void BlankingManager_::updateBlankingStatus() {
  _pirBlanked = checkPIR();
  _timeBasedBlanked = checkTimeBasedBlanking(weekday(), hour());

  if (cc->mdBlankMode == MD_DISABLE) {
    // don't take the MD into account at all
    _blanked = _timeBasedBlanked;
  } else if (cc->mdBlankMode == MD_RESPECT_BLANK) {
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
        _blankTowers = false;
        break;
      }
      case BLANK_MODE_LEDS: {
        _blankTubes = false;
        _blankLEDs = true;
        _blankTowers = false;
        break;
      }
      case BLANK_MODE_TUBES_LEDS: {
        _blankTubes = true;
        _blankLEDs = true;
        _blankTowers = false;
        break;
      }
      case BLANK_MODE_ALL: {
        _blankTubes = true;
        _blankLEDs = true;
        _blankTowers = true;
        break;
      }
    }
  } else {
    _blankTubes = false;
    _blankLEDs = false;
    _blankTowers = false;
  }

  if (_blankLEDoverride) {
    _blankLEDs = true;
  }

  // Trigger events
  if (_blankTubes != _PrevBlankTubes) {
    triggerTubeBlankChange(_blankTubes);
    _PrevBlankTubes = _blankTubes;
  }

  if (_blankLEDs != _PrevBlankLEDs) {
    triggerLEDBlankChange(_blankLEDs);
    _PrevBlankLEDs = _blankLEDs;
  }

  if (_blankTowers != _PrevBlankTowers) {
    triggerTowerBlankChange(_blankTowers);
    _PrevBlankTowers = _blankTowers;
  }
}

// ************************************************************
// Get if the PIR is installed
// ************************************************************
bool BlankingManager_::getCurrentPIRInstalled() {
  return _pirInstalled;
}

// ************************************************************
// Get if the PIR is active
// ************************************************************
bool BlankingManager_::getCurrentPIRStatus() {
  return _pirvalue;
}

// ************************************************************
// Overall status of if we are blanked
// ************************************************************
bool BlankingManager_::getCurrentBlankingStatus() {
  return (_timeBasedBlanked ||_pirBlanked);
}

// ************************************************************
// True if we are blaned otherwise a percentual indicator of
// how close we are to blanking, if called regularly.
// Primarily used for the status Blinkenlight.
// ************************************************************
bool BlankingManager_::getCurrentBlankingIndicator() {
  if (_timeBasedBlanked) return true;
  if (_pirBlanked) return true;

  // We are waiting for PIR blanking to finish
  return _pirBlankingPct > secsDeltaAbs;
}

// ************************************************************
// Reason we are blanked
// ************************************************************
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

// ************************************************************
// The next mode name
// ************************************************************
String BlankingManager_::getNextBlankingModeName() {
  switch(getNextBlankingMode()) {
    case DAY_BLANKING_NEVER:
      return "off";
      break;
    case DAY_BLANKING_HOURS:
      return "hours";
      break;
    case DAY_BLANKING_WEEKEND:
      return "weekend";
      break;
    case DAY_BLANKING_WEEKEND_OR_HOURS:
      return "WE or Hrs";
      break;
    case DAY_BLANKING_WEEKEND_AND_HOURS:
      return "WE & Hrs";
      break;
    case DAY_BLANKING_WEEKDAY:
      return "weekday";
      break;
    case DAY_BLANKING_WEEKDAY_OR_HOURS:
      return "WD / Hrs";
      break;
    case DAY_BLANKING_WEEKDAY_AND_HOURS:
      return "WD & Hrs";
      break;
    case DAY_BLANKING_ALWAYS:
      return "always";
      break;
    default:
      return "unknown";
      break;
  }
}

// ************************************************************
// If we should accept hours settings
// ************************************************************
bool BlankingManager_::getCurrentModeWantsHours() {
  return (cc->dayBlanking > 3);
}

// ************************************************************
// The next mode
// ************************************************************
DayBlankingMode BlankingManager_::getNextBlankingMode() {
  byte nextMode = cc->dayBlanking + 1;
  if (nextMode > DAY_BLANKING_WEEKDAY_AND_HOURS) nextMode = DAY_BLANKING_NEVER;
  return static_cast<DayBlankingMode>(nextMode);
}

// ************************************************************
// How long ago we last saw movement
// ************************************************************
int BlankingManager_::getBlankAge()
{
    int lastMotionDetection = (nowMillis - _pirLastSeen) / 1000.0;
    return lastMotionDetection;
}

// ************************************************************
// Send blanking triggers to affected components - tubes
// ************************************************************
void BlankingManager_::triggerTubeBlankChange(bool newStatus) {
  outputManager.setBlankingStatusTubes(newStatus);
  slaveManagerNixie.setBlankingStatus(newStatus);
}

// ************************************************************
// Send blanking triggers to affected components - LEDs
// ************************************************************
void BlankingManager_::triggerLEDBlankChange(bool newStatus) {
  ledManager.setLEDBlanking(newStatus);
}

// ************************************************************
// Send blanking triggers to affected components - Towers
// ************************************************************
void BlankingManager_::triggerTowerBlankChange(bool newStatus) {
  ledManager.setTowerBlanking(newStatus);
  outputManager.setBlankingStatusTowers(newStatus);
}

BlankingManager_ &BlankingManager_::getInstance() {
  static BlankingManager_ instance;
  return instance;
}

BlankingManager_ &blankingManager = blankingManager.getInstance();