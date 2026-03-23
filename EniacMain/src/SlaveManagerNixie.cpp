#include "SlaveManagerNixie.h"

// ************************************************************
// Get the numeric value of the next slave mode
// ************************************************************
void SlaveManagerNixie_::begin() {
}

// ************************************************************
// Detect if there is a slave connected, called at startup
// ************************************************************
void SlaveManagerNixie_::testSlave() {
  // we only want to try once on the startup test
  _slaveEnabled = true;
  sendUpdateToSlaveI2C();
  
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
    blankSlaveI2C();
  }
  _slaveEnabled = newSlaveStatus;
}

// ************************************************************
// return current mode
// ************************************************************
bool SlaveManagerNixie_::getSlaveMode() {
  return _slaveEnabled && (cc->slaveMode != SLAVE_MODE_OFF);
}

// ************************************************************
// Slave I2C attempts to date
// ************************************************************
unsigned int SlaveManagerNixie_::getTryCount() {
  return _slaveModeTryCount;
}

// ************************************************************
// Slave I2C attempts that failed to date
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
      (cc->slaveMode == SLAVE_MODE_100THS) || 
      (cc->slaveMode == SLAVE_MODE_SECS)) {
    sendUpdateToSlaveI2C();
  }
}

// ************************************************************
// Once per minute update for the modes that need it
// ************************************************************
void SlaveManagerNixie_::updateOncePerMinute() {
  if (cc->slaveMode == SLAVE_MODE_DATE) {
    sendUpdateToSlaveI2C();
  }
}

// ************************************************************
// Called once per second with update info
// ************************************************************
void SlaveManagerNixie_::sendUpdateToSlaveI2C() {
  // deal with blanking the display
  if (previousMode != cc->slaveMode) {
    if (cc->slaveMode == SLAVE_MODE_OFF) {
      blankSlaveI2C();
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

    Wire.beginTransmission(SLAVE_MODULE_I2C_ADDRESS);
    Wire.write((uint8_t)cc->slaveMode);
    Wire.write((uint8_t)dimmingPct);
    Wire.write((uint8_t)second());
    Wire.write((uint8_t)day());
    Wire.write((uint8_t)month());
    byte error = Wire.endTransmission();

    if (error == 0) {
      debugMsgSlv("Sent slave update");
    } else {
      debugMsgSlv("Failed sending slave update: " + String(error));
      _slaveModeFailCount++;
    }
  }
}

// ************************************************************
// Called once per second with update info
// ************************************************************
void SlaveManagerNixie_::blankSlaveI2C() {
  _slaveModeTryCount++;

  Wire.beginTransmission(SLAVE_MODULE_I2C_ADDRESS);
  Wire.write((uint8_t)SLAVE_MODE_OFF);
  Wire.write((uint8_t)0);
  Wire.write((uint8_t)second());
  Wire.write((uint8_t)day());
  Wire.write((uint8_t)month());
  byte error = Wire.endTransmission();

  if (error == 0) {
    debugMsgSlv("Sent slave update");
    _slaveModeFailCount = 0;
  } else {
    debugMsgSlv("Failed sending slave update: " + String(error));
    _slaveModeFailCount++;
  }
}

// ************************************************************
// Get the name of the next slave mode: for OLED menu
// ************************************************************
String SlaveManagerNixie_::getNextSlaveModeName() {
  switch (getNextSlaveMode()) {
  case SLAVE_MODE_100THS:
    return "100ths";
    break;
  case SLAVE_MODE_DATE:
    return "Date";
    break;
  case SLAVE_MODE_SECS:
    return "Secs";
    break;
  case SLAVE_MODE_OFF:
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
  if (nextMode > SLAVE_MODE_MAX) {
    nextMode = SLAVE_MODE_MIN;
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