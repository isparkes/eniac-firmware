#include "SlaveManagerDecatron.h"

// ************************************************************
// Initialise
// ************************************************************
void SlaveManagerDecatron_::begin() {
}

// ************************************************************
// Override the slave mode via front panel switch
// ************************************************************
void SlaveManagerDecatron_::setSlaveEnabled(bool newSlaveStatus) {
  _slaveEnabled = newSlaveStatus;
}

// ************************************************************
// Slave I2C attempts to date
// ************************************************************
unsigned int SlaveManagerDecatron_::getTryCount() {
  return _slaveTryCount;
}

// ************************************************************
// Slave I2C attempts that failed to date
// ************************************************************
unsigned int SlaveManagerDecatron_::getFailCount() {
  return _slaveFailCount;
}

// ************************************************************
// Once per second update
// ************************************************************
void SlaveManagerDecatron_::updateOncePerSecond() {
  if (_slaveEnabled) {
    sendUpdateToSlaveI2C();
  }
}

// ************************************************************
// Once per minute update - time is sent every second so
// no additional action needed here
// ************************************************************
void SlaveManagerDecatron_::updateOncePerMinute() {
}

// ************************************************************
// Send time and control data to the Decatron slave over I2C
// ************************************************************
void SlaveManagerDecatron_::sendUpdateToSlaveI2C() {
  byte control = 0;
  if (blankingManager.getCurrentBlankingStatus()) {
    control |= DECATRON_CTRL_BLANKED;
  }
  control |= (cc->pMode & 0x0F) << DECATRON_CTRL_MODE_SHIFT;

  _slaveTryCount++;

  Wire.beginTransmission(DECATRON_SLAVE_I2C_ADDRESS);
  Wire.write((uint8_t)hour());
  Wire.write((uint8_t)minute());
  Wire.write((uint8_t)second());
  Wire.write((uint8_t)control);
  byte error = Wire.endTransmission();

  if (error == 0) {
    debugMsgSlv("Sent Decatron slave update");
  } else {
    debugMsgSlv("Failed Decatron slave update: " + String(error));
    _slaveFailCount++;
  }
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
SlaveManagerDecatron_ &SlaveManagerDecatron_::getInstance() {
  static SlaveManagerDecatron_ instance;
  return instance;
}

SlaveManagerDecatron_ &slaveManagerDecatron = slaveManagerDecatron.getInstance();
