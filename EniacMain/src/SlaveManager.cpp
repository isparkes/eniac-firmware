#include "SlaveManager.h"

// ************************************************************
// Get the numeric value of the next slave mode
// ************************************************************
void SlaveManager_::begin() {
}

// ************************************************************
// Detect if there is a slave connected, called at startup
// ************************************************************
void SlaveManager_::testSlave() {
  // we only want to try once on the startup test
  _slaveEnabled = true;
  sendUpdateToSlaveI2C();
  if (_slaveModeFailCount == 0)
    debugMsgSlv("Slave detected")
  else
    debugMsgSlv("Slave NOT detected");
}

// ************************************************************
// Override the slave mode via front panel switch
// ************************************************************
void SlaveManager_::setSlaveEnabled(bool newSlaveStatus) {
  if ((_slaveEnabled != newSlaveStatus) && (!_slaveEnabled)) {
    blankSlaveI2C();
  }
  _slaveEnabled = newSlaveStatus;
}

// ************************************************************
// return current mode
// ************************************************************
bool SlaveManager_::getSlaveMode() {
  return _slaveEnabled && (cc->slaveMode != SLAVE_MODE_OFF);
}

// ************************************************************
// Slave I2C attempts to date
// ************************************************************
unsigned int SlaveManager_::getTryCount() {
  return _slaveModeTryCount;
}

// ************************************************************
// Slave I2C attempts that failed to date
// ************************************************************
unsigned int SlaveManager_::getFailCount() {
  return _slaveModeFailCount;
}

// ************************************************************
// Once per second update for the modes that need it
// ************************************************************
void SlaveManager_::updateOncePerSecond() {
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
void SlaveManager_::updateOncePerMinute() {
  if (cc->slaveMode == SLAVE_MODE_DATE) {
    sendUpdateToSlaveI2C();
  }
}

// ************************************************************
// Called once per second with update info
// ************************************************************
void SlaveManager_::sendUpdateToSlaveI2C() {
  // deal with blanking the display
  if (previousMode != cc->slaveMode) {
    if (cc->slaveMode == SLAVE_MODE_OFF) {
      blankSlaveI2C();
    }
    previousMode = cc->slaveMode;
  }

  if (getSlaveMode()) {
    byte dimmingPct = 0;
    if (!blankingManager.getCurrentBlankTubes())
      dimmingPct = (byte) ldrManager.getLDRValuePct();

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
void SlaveManager_::blankSlaveI2C() {
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
String SlaveManager_::getNextSlaveModeName() {
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
void SlaveManager_::setNextSlaveMode() {
  cc->slaveMode = getNextSlaveMode();
}

// ************************************************************
// Set the slave mode explictly
// ************************************************************
void SlaveManager_::setSlaveMode(byte newMode) {
  cc->slaveMode = newMode;
}

// ************************************************************
// Get the numeric value of the next slave mode
// ************************************************************
byte SlaveManager_::getNextSlaveMode() {
  byte nextMode = cc->slaveMode + 1;
  if (nextMode > SLAVE_MODE_MAX) {
    nextMode = SLAVE_MODE_MIN;
  }
  return nextMode;
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
SlaveManager_ &SlaveManager_::getInstance() {
  static SlaveManager_ instance;
  return instance;
}

SlaveManager_ &slaveManager = slaveManager.getInstance();