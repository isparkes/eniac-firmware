#pragma once

#include <Arduino.h>
#include "Globals.h"

// --------------------------------- Protocol ------------------------------------
// Digits Update: Same data packet regardless of mode!
//   Byte 1 - Mode 
//      #define I2C_SET_SLAVE_DATA     0x1A
//      #define I2C_SET_SLAVE_DATE     0x1B
//      #define I2C_SET_SLAVE_SECS     0x1C
//      #define I2C_SET_SLAVE_BLANK    0x1D
//   Byte 2 - Current Second 00 - 59 
//   Byte 3 - Current DoM 00 - 31
//   Byte 4 - Current Mon 01 - 12
//
// Backlights Update: TBD
//
//
// -------------------------------------------------------------------------------

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