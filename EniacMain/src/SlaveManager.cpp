#include "SlaveManager.h"

void SlaveManager_::startSlaveI2C() {
  _slaveModeFailCount = 0;
  _slaveModeStatus = true;
}

void SlaveManager_::stopSlaveI2C() {
  _slaveModeStatus = false;
}

bool SlaveManager_::getSlaveMode() {
  return _slaveModeStatus;
}

void SlaveManager_::sendUpdateToSlaveI2C() {
  if(_slaveModeStatus) {      
    #ifdef DEBUG_ON
    debugMsg("Slave update: mode: " + String(cc->slaveMode) + "," + String(second()) + "," + String(month()) + "," + String(day()));
    #endif

    Wire.beginTransmission(SLAVE_MODULE_I2C_ADDRESS);
    switch (cc->slaveMode) {
      case SLAVE_MODE_100THS: {
        Wire.write((uint8_t)0x1a);
        break;
      }
      case SLAVE_MODE_DATE: {
        Wire.write((uint8_t)0x1b);
        break;
      }
      case SLAVE_MODE_SECS: {
        Wire.write((uint8_t)0x1c);
        break;
      }
    }
    Wire.write((uint8_t)second());
    Wire.write((uint8_t)day());
    Wire.write((uint8_t)month());

    byte error = Wire.endTransmission();

    #ifdef DEBUG_ON
    if (error == 0) {
      debugMsg("Sent slave update");
    } else {
      debugMsg("Failed sending slave update: " + String(error));
      _slaveModeFailCount++;
      if(_slaveModeFailCount > SLAVE_MODE_MAX_RETRIES) {
        #ifdef DEBUG_ON
        debugMsg("Failed to do slave update after " + String(SLAVE_MODE_MAX_RETRIES) + " retries, giving up.");
        #endif
        _slaveModeStatus = false;
      }
    }
    #endif
  }
}

// ************************************************************
// Output a logging message to the debug output
// ************************************************************
void SlaveManager_::debugMsg(String message) {
  debugManager.debugMsg("[SLV]: " + message);
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
SlaveManager_ &SlaveManager_::getInstance() {
  static SlaveManager_ instance;
  return instance;
}

SlaveManager_ &slaveManager = slaveManager.getInstance();