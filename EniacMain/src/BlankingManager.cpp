#include "BlankingManager.h"
#include "LDRManager.h"

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

  // Determine the effective action per output
  if (_blanked) {
    _actionTubes    = static_cast<BlankingAction>(cc->blankModeTubes);
    _actionLEDs     = static_cast<BlankingAction>(cc->blankModeLEDs);
    _actionSepNeon  = static_cast<BlankingAction>(cc->blankModeSepNeon);
    _actionSlave    = static_cast<BlankingAction>(cc->blankModeSlave);
    _actionSepTower = static_cast<BlankingAction>(cc->blankModeSepTower);
  } else {
    _actionTubes    = BLANKING_ACTION_NORMAL;
    _actionLEDs     = BLANKING_ACTION_NORMAL;
    _actionSepNeon  = BLANKING_ACTION_NORMAL;
    _actionSlave    = BLANKING_ACTION_NORMAL;
    _actionSepTower = BLANKING_ACTION_NORMAL;
  }

  if (_blankLEDoverride) {
    _actionLEDs = BLANKING_ACTION_BLANK;
  }

  // Trigger events on change
  if (_actionTubes != _prevActionTubes) {
    triggerTubeActionChange(_actionTubes);
    _prevActionTubes = _actionTubes;
  }

  if (_actionLEDs != _prevActionLEDs) {
    triggerLEDActionChange(_actionLEDs);
    _prevActionLEDs = _actionLEDs;
  }

  if (_actionSepNeon != _prevActionSepNeon) {
    triggerSepNeonActionChange(_actionSepNeon);
    _prevActionSepNeon = _actionSepNeon;
  }

  if (_actionSlave != _prevActionSlave) {
    triggerSlaveActionChange(_actionSlave);
    _prevActionSlave = _actionSlave;
  }

  if (_actionSepTower != _prevActionSepTower) {
    triggerSepTowerActionChange(_actionSepTower);
    _prevActionSepTower = _actionSepTower;
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
// Return the effective slave action (for decatron slave)
// ************************************************************
BlankingAction BlankingManager_::getSlaveAction() {
  return _actionSlave;
}

// ************************************************************
// Send blanking/dim triggers to tubes and nixie slave
// ************************************************************
void BlankingManager_::triggerTubeActionChange(BlankingAction newAction) {
  outputManager.setBlankingStatusTubes(newAction == BLANKING_ACTION_BLANK);
  ldrManager.setBlankingDim(newAction == BLANKING_ACTION_DIM);
  slaveManagerNixie.setBlankingStatus(newAction == BLANKING_ACTION_BLANK);
  slaveManagerNixie.setDimmingStatus(newAction == BLANKING_ACTION_DIM);
}

// ************************************************************
// Send blanking/dim triggers to NeoPixel backlights
// ************************************************************
void BlankingManager_::triggerLEDActionChange(BlankingAction newAction) {
  ledManager.setLEDBlanking(newAction == BLANKING_ACTION_BLANK);
  ledManager.setLEDDimmingStatus(newAction == BLANKING_ACTION_DIM);
}

// ************************************************************
// Send blanking triggers to separator neons (on/off only: DIM=BLANK)
// ************************************************************
void BlankingManager_::triggerSepNeonActionChange(BlankingAction newAction) {
  outputManager.setBlankingStatusTowers(newAction != BLANKING_ACTION_NORMAL);
}

// ************************************************************
// Send blanking triggers for slave action changes (nixie slave handled
// in triggerTubeActionChange; this is a no-op as decatron reads directly)
// ************************************************************
void BlankingManager_::triggerSlaveActionChange(BlankingAction newAction) {
  // Decatron slave reads getSlaveAction() directly each second
  // Nixie slave is driven alongside tube action (triggerTubeActionChange)
}

// ************************************************************
// Send blanking/dim triggers to separator tower NeoPixels
// ************************************************************
void BlankingManager_::triggerSepTowerActionChange(BlankingAction newAction) {
  ledManager.setTowerBlanking(newAction == BLANKING_ACTION_BLANK);
  ledManager.setTowerDimmingStatus(newAction == BLANKING_ACTION_DIM);
}

BlankingManager_ &BlankingManager_::getInstance() {
  static BlankingManager_ instance;
  return instance;
}

BlankingManager_ &blankingManager = blankingManager.getInstance();