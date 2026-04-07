#include "SlaveManagerNixie.h"

// ************************************************************
// Get the numeric value of the next slave mode
// ************************************************************
void SlaveManagerNixie_::begin() {
  Serial2.begin(NIXIE_SERIAL_BAUD, SERIAL_8N1, -1, NIXIE_SERIAL_TX_PIN);
}

// ************************************************************
// Detect if there is a slave connected, called at startup
// ************************************************************
void SlaveManagerNixie_::testSlave() {
  // we only want to try once on the startup test
  _slaveEnabled = true;
  sendUpdateToSlaveSerial();

  #ifdef DEBUG
  if (_slaveModeFailCount == 0)
    debugMsgSlv("Slave detected")
  else
    debugMsgSlv("Slave NOT detected");
  #endif
}

// ************************************************************
// Override the slave mode via front panel switch
// ************************************************************
void SlaveManagerNixie_::setSlaveEnabled(bool newSlaveStatus) {
  if ((_slaveEnabled != newSlaveStatus) && (!newSlaveStatus)) {
    blankSlaveSerial();
  }
  _slaveEnabled = newSlaveStatus;
}

// ************************************************************
// return current mode
// ************************************************************
bool SlaveManagerNixie_::getSlaveMode() {
  return _slaveEnabled && (cc->slaveMode != SLAVE_NIX_MODE_OFF);
}

// ************************************************************
// Slave serial attempts to date
// ************************************************************
unsigned int SlaveManagerNixie_::getTryCount() {
  return _slaveModeTryCount;
}

// ************************************************************
// Slave serial attempts that failed to date
// ************************************************************
unsigned int SlaveManagerNixie_::getFailCount() {
  return _slaveModeFailCount;
}

// ************************************************************
// Once per second update for the modes that need it
// ************************************************************
void SlaveManagerNixie_::updateOncePerSecond() {
  // If we change to a per minute update, update the display anyway
  if ((previousMode != cc->slaveMode) ||
      (cc->slaveMode == SLAVE_NIX_MODE_100THS) ||
      (cc->slaveMode == SLAVE_NIX_MODE_SECS)) {
    sendUpdateToSlaveSerial();
  }
}

// ************************************************************
// Once per minute update for the modes that need it
// ************************************************************
void SlaveManagerNixie_::updateOncePerMinute() {
  if (cc->slaveMode == SLAVE_NIX_MODE_DATE) {
    sendUpdateToSlaveSerial();
  }
}

// ************************************************************
// Send current state to the Nixie slave over serial
// ************************************************************
void SlaveManagerNixie_::sendUpdateToSlaveSerial() {
  // deal with blanking the display
  if (previousMode != cc->slaveMode) {
    if (cc->slaveMode == SLAVE_NIX_MODE_OFF) {
      blankSlaveSerial();
    }
    previousMode = cc->slaveMode;
  }

  if (getSlaveMode()) {
    byte dimmingPct = 0;
    if (!_blanked) {
      if (_dimmed)
        dimmingPct = (byte) cc->minTubeDim;
      else
        dimmingPct = (byte) ldrManager.getLDRValueTubePct();
    }

    _slaveModeTryCount++;

    Serial2.write((uint8_t)NIXIE_SERIAL_HEADER);
    Serial2.write((uint8_t)cc->slaveMode);
    Serial2.write((uint8_t)dimmingPct);
    Serial2.write((uint8_t)second());
    Serial2.write((uint8_t)day());
    Serial2.write((uint8_t)month());
    Serial2.flush();

    debugMsgSlv("Sent slave update");
  }
}

// ************************************************************
// Blank the Nixie slave display
// ************************************************************
void SlaveManagerNixie_::blankSlaveSerial() {
  _slaveModeTryCount++;

  Serial2.write((uint8_t)NIXIE_SERIAL_HEADER);
  Serial2.write((uint8_t)SLAVE_NIX_MODE_OFF);
  Serial2.write((uint8_t)0);
  Serial2.write((uint8_t)second());
  Serial2.write((uint8_t)day());
  Serial2.write((uint8_t)month());
  Serial2.flush();

  debugMsgSlv("Sent slave blank");
  _slaveModeFailCount = 0;
}

// ************************************************************
// Get the name of the next slave mode: for OLED menu
// ************************************************************
String SlaveManagerNixie_::getNextSlaveModeName() {
  switch (getNextSlaveMode()) {
  case SLAVE_NIX_MODE_100THS:
    return "100ths";
    break;
  case SLAVE_NIX_MODE_DATE:
    return "Date";
    break;
  case SLAVE_NIX_MODE_SECS:
    return "Secs";
    break;
  case SLAVE_NIX_MODE_OFF:
    return "Off";
    break;
  default:
    return "Unknown";
  }
}

// ************************************************************
// Set the slave mode by using the next one: OLED
// ************************************************************
void SlaveManagerNixie_::setNextSlaveMode() {
  cc->slaveMode = getNextSlaveMode();
}

// ************************************************************
// Set the slave mode explictly
// ************************************************************
void SlaveManagerNixie_::setSlaveMode(byte newMode) {
  cc->slaveMode = newMode;
}

// ************************************************************
// Get the numeric value of the next slave mode
// ************************************************************
byte SlaveManagerNixie_::getNextSlaveMode() {
  byte nextMode = cc->slaveMode + 1;
  if (nextMode > SLAVE_NIX_MODE_MAX) {
    nextMode = SLAVE_NIX_MODE_MIN;
  }
  return nextMode;
}

// ************************************************************
// Set the tube blanking status
// ************************************************************
void SlaveManagerNixie_::setBlankingStatus(bool newStatus) {
  _blanked = newStatus;
}

// ************************************************************
// Set the dim-during-blanking status
// ************************************************************
void SlaveManagerNixie_::setDimmingStatus(bool newStatus) {
  _dimmed = newStatus;
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
SlaveManagerNixie_ &SlaveManagerNixie_::getInstance() {
  static SlaveManagerNixie_ instance;
  return instance;
}

SlaveManagerNixie_ &slaveManagerNixie = slaveManagerNixie.getInstance();