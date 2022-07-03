#pragma once

#include <Arduino.h>
#include "Globals.h"

// --------------------------------- Protocol ------------------------------------
// Digits Update: Same data packet regardless of mode!
//   Byte 1 - Mode 
//      #define I2C_SET_SLAVE_DATA     0x1A
//      #define I2C_SET_SLAVE_DATE     0x1B
//      #define I2C_SET_SLAVE_SECS     0x1C
//   Byte 2 - Current Second 00 - 59 
//   Byte 3 - Current DoM 00 - 31
//   Byte 4 - Current Mon 01 - 12
//   Byte 5 - Dimming value Percent 0 - 100, 0 = blanked
//
// -------------------------------------------------------------------------------

#define SLAVE_MODULE_I2C_ADDRESS  105

#define MAX_SLAVE_MODE_FAIL_COUNT 20

// -------------------------------------------------------------------------------
#define SLAVE_MODE_MIN                  0
#define SLAVE_MODE_100THS               0
#define SLAVE_MODE_DATE                 1
#define SLAVE_MODE_SECS                 2
#define SLAVE_MODE_OFF                  3
#define SLAVE_MODE_MAX                  3
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
    void testSlave();
    bool getSlaveMode();
    String getNextSlaveModeName();
    void setNextSlaveMode();
    void setSlaveMode(byte newMode);
    void setSlaveEnabled(bool newSlaveStatus);
    void updateOncePerSecond();
    void updateOncePerMinute();
    unsigned int getTryCount();
    unsigned int getFailCount();
  private:
    bool _slaveEnabled;
    unsigned int _slaveModeFailCount;
    unsigned int _slaveModeTryCount;
    byte previousMode = 255;
    byte getNextSlaveMode();
    void sendUpdateToSlaveI2C();
    void blankSlaveI2C();
};

extern SlaveManager_ &slaveManager;