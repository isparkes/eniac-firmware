#include "SlaveManagerDecatron.h"

// ************************************************************
// Get the numeric value of the next slave mode
// ************************************************************
void SlaveManagerDecatron_::begin() {
}

// ************************************************************
// Override the slave mode via front panel switch
// ************************************************************
void SlaveManagerDecatron_::setSlaveEnabled(bool newSlaveStatus) {
  if ((_slaveEnabled != newSlaveStatus) && (!newSlaveStatus)) {
  }
  _slaveEnabled = newSlaveStatus;
}

// ************************************************************
// Once per second update for the modes that need it
// ************************************************************
void SlaveManagerDecatron_::updateOncePerSecond() {
  if (!blankingManager.getCurrentBlankingStatus() && _slaveEnabled) {
    triggerOnePulsePerSecShort();
    debugMsgSlv("Short");
  }
}

// ************************************************************
// Once per minute update for the modes that need it
// ************************************************************
void SlaveManagerDecatron_::updateOncePerMinute() {
  if (!blankingManager.getCurrentBlankingStatus() && _slaveEnabled) {
    triggerOnePulsePerSecLong();
    debugMsgSlv("Long");
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