#pragma once

#include <Arduino.h>
#include "Globals.h"
#include "BlankingManager.h"

// --------------------------------- Protocol ------------------------------------
// 1 10mS pulse per second
// 1 100mS pule per minute
// >1s no pulse = blank
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
class SlaveManagerDecatron_ {
  private:
    SlaveManagerDecatron_() = default; // Make constructor private

  public:
    static SlaveManagerDecatron_ &getInstance(); // Accessor for singleton instance

    SlaveManagerDecatron_(const SlaveManagerDecatron_ &) = delete; // no copying
    SlaveManagerDecatron_ &operator=(const SlaveManagerDecatron_ &) = delete;

  public:
    void begin();
    void updateOncePerSecond();
    void updateOncePerMinute();
    void setSlaveEnabled(bool newSlaveStatus);
  private:
    bool _slaveEnabled = true;
};

extern SlaveManagerDecatron_ &slaveManagerDecatron;