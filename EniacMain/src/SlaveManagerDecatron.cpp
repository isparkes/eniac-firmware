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
}

// ************************************************************
// Once per minute update for the modes that need it
// ************************************************************
void SlaveManagerDecatron_::updateOncePerMinute() {
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
SlaveManagerDecatron_ &SlaveManagerDecatron_::getInstance() {
  static SlaveManagerDecatron_ instance;
  return instance;
}

SlaveManagerDecatron_ &slaveManagerDecatron = slaveManagerDecatron.getInstance();