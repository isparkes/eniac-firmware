#include "SlaveManagerDecatron.h"

// ************************************************************
// Initialise
// ************************************************************
void SlaveManagerDecatron_::begin() {
  Serial2.begin(DECATRON_SERIAL_BAUD, SERIAL_8N1, -1, DECATRON_SERIAL_TX_PIN);
}

// ************************************************************
// Override the slave mode via front panel switch
// ************************************************************
void SlaveManagerDecatron_::setSlaveEnabled(bool newSlaveStatus) {
  _slaveEnabled = newSlaveStatus;
}

// ************************************************************
// Slave serial transmit attempts to date
// ************************************************************
unsigned int SlaveManagerDecatron_::getTryCount() {
  return _slaveTryCount;
}

// ************************************************************
// Slave serial transmit attempts that failed to date
// ************************************************************
unsigned int SlaveManagerDecatron_::getFailCount() {
  return _slaveFailCount;
}

// ************************************************************
// Once per second update
// ************************************************************
void SlaveManagerDecatron_::updateOncePerSecond() {
  if (_slaveEnabled) {
    sendUpdateToSlaveSerial();
  }
}

// ************************************************************
// Once per minute update - time is sent every second so
// no additional action needed here
// ************************************************************
void SlaveManagerDecatron_::updateOncePerMinute() {
}

// ************************************************************
// Send time and control data to the Decatron slave over serial
// ************************************************************
void SlaveManagerDecatron_::sendUpdateToSlaveSerial() {
  byte control = 0;
  // DIM maps to blanked for decatron (protocol has no partial brightness)
  if (blankingManager.getSlaveAction() != BLANKING_ACTION_NORMAL) {
    control |= DECATRON_CTRL_BLANKED;
  }
  control |= (cc->pMode & 0x0F) << DECATRON_CTRL_MODE_SHIFT;

  _slaveTryCount++;

  Serial2.write((uint8_t)0xAA);
  Serial2.write((uint8_t)hour());
  Serial2.write((uint8_t)minute());
  Serial2.write((uint8_t)second());
  Serial2.write((uint8_t)control);
  Serial2.flush();

  debugMsgSlvX("Sent Decatron slave update");
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
SlaveManagerDecatron_ &SlaveManagerDecatron_::getInstance() {
  static SlaveManagerDecatron_ instance;
  return instance;
}

SlaveManagerDecatron_ &slaveManagerDecatron = slaveManagerDecatron.getInstance();
