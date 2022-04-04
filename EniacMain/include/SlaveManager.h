#pragma once

#include <Arduino.h>
#include "globals.h"

#define SLAVE_MODULE_I2C_ADDRESS 105

// Used for timing out on the slave
#define SLAVE_MODE_MAX_RETRIES 20

// -------------------------------------------------------------------------------
#define SLAVE_MODE_MIN                  0
#define SLAVE_MODE_100THS               0
#define SLAVE_MODE_DATE                 1
#define SLAVE_MODE_SECS                 2
#define SLAVE_MODE_MAX                  2
#define SLAVE_MODE_DEFAULT              0

class SlaveManager_ {
  private:
    SlaveManager_() = default; // Make constructor private

  public:
    static SlaveManager_ &getInstance(); // Accessor for singleton instance

    SlaveManager_(const SlaveManager_ &) = delete; // no copying
    SlaveManager_ &operator=(const SlaveManager_ &) = delete;

  public:
    void begin();

    void sendUpdateToSlaveI2C();
    void startSlaveI2C();
    void stopSlaveI2C();
    bool getSlaveMode();
  private:
    bool _slaveModeStatus;
    byte _slaveModeFailCount;
};

extern SlaveManager_ &slaveManager;