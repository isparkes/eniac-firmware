#include "SlaveManager.h"

// ************************************************************
// Detect if there is a slave connected, called at startup
// ************************************************************
void SlaveManager_::testSlave() {
  // we only want to try once on the startup test
  _slaveModeFailCount = SLAVE_MODE_MAX_RETRIES;
  _slaveModeStatus = true;
  sendUpdateToSlaveI2C();
  if (_slaveModeStatus)
    debugMsgSlv("Slave detected")
  else
    debugMsgSlv("Slave NOT detected");
}

// ************************************************************
// Start the slave
// ************************************************************
void SlaveManager_::startSlaveI2C() {
  _slaveModeFailCount = 0;
  _slaveModeStatus = true;
}

// ************************************************************
// Stop the slave
// ************************************************************
void SlaveManager_::stopSlaveI2C() {
  _slaveModeStatus = false;
}

// ************************************************************
// Override the slave mode via front panel switch
// ************************************************************
void SlaveManager_::setSlaveModeViaSwitch(bool newSlaveStatus) {
  if (_slaveModeOverrideStatus != newSlaveStatus) {
    if (!newSlaveStatus) {
      startSlaveI2C();
    } else {
      stopSlaveI2C();
    }
  _slaveModeOverrideStatus = newSlaveStatus;
  }
}

// ************************************************************
// return current mode
// ************************************************************
bool SlaveManager_::getSlaveMode() {
  return _slaveModeStatus;
}

// ************************************************************
// Called once per second with update info
// ************************************************************
void SlaveManager_::sendUpdateToSlaveI2C() {
  if(_slaveModeStatus) {      
    byte dimmingPct = 0;
    if (!blankingManager.getCurrentBlankTubes())
      dimmingPct = (byte) ldrManager.getLDRValuePct();

    byte effectiveMode = cc->slaveMode;

    if (_slaveModeOverrideStatus) {
      effectiveMode = SLAVE_MODE_OFF;
    }

    Wire.beginTransmission(SLAVE_MODULE_I2C_ADDRESS);
    switch (effectiveMode) {
      case SLAVE_MODE_DIMMING: {
        debugMsgSlv("Slave update: mode: 0x1a, " + String(dimmingPct) + ", 0x1a");
        Wire.write((uint8_t)0x1a);
        Wire.write((uint8_t)dimmingPct);
        Wire.write((uint8_t)0x1a);
        break;
      }
      case SLAVE_MODE_DATE: {
        debugMsgSlv("Slave update: mode: 0x1b, " + String(day()) + "," + String(month()) + ", 0x1b");
        Wire.write((uint8_t)0x1b);
        Wire.write((uint8_t)day());
        Wire.write((uint8_t)month());
        Wire.write((uint8_t)0x1b);
        break;
      }
      case SLAVE_MODE_100THS:
      case SLAVE_MODE_SECS: {
        debugMsgSlv("Slave update: mode: 0x1c, " + String(second()) + ", 0x1c");
        Wire.write((uint8_t)0x1c);
        Wire.write((uint8_t)second());
        Wire.write((uint8_t)0x1c);
        break;
      }
      case SLAVE_MODE_OFF: {
        debugMsgSlv("Slave update: mode: 0x1a, 0 (blanked), 0x1a");
        Wire.write((uint8_t)0x1a);
        Wire.write((uint8_t)0);
        Wire.write((uint8_t)0x1a);
        break;
      }
    }

    byte error = Wire.endTransmission();

    if (error == 0) {
      debugMsgSlv("Sent slave update");
      _slaveModeFailCount = 0;
    } else {
      debugMsgSlv("Failed sending slave update: " + String(error));
      _slaveModeFailCount++;
      if(_slaveModeFailCount > SLAVE_MODE_MAX_RETRIES) {
        debugMsgSlv("Failed to do slave update after " + String(SLAVE_MODE_MAX_RETRIES) + " retries, giving up.");
        _slaveModeStatus = false;
      }
    }
  }
}

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

void SlaveManager_::setNextSlaveMode() {
  cc->slaveMode = getNextSlaveMode();
}

void SlaveManager_::setSlaveMode(byte newMode) {
  if (newMode > SLAVE_MODE_MAX) {
    newMode = SLAVE_MODE_MAX;
  }
  if (newMode < SLAVE_MODE_MIN) {
    newMode = SLAVE_MODE_MIN;
  }
  cc->slaveMode = newMode;
}

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