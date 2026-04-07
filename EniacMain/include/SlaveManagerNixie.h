#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "Globals.h"
#include "LDRManager.h"
#include "BlankingManager.h"

// --------------------------------- Protocol ------------------------------------
// Serial communication once per second (UART2, 115200 baud, TX on GPIO0)
//   Byte 0: Start byte (0xAA)
//   Byte 1: Mode (0=100ths, 1=date, 2=secs, 3=off)
//   Byte 2: Dimming percent (0-100, 0 = blanked)
//   Byte 3: Current second (0-59)
//   Byte 4: Current day of month (1-31)
//   Byte 5: Current month (1-12)
// -------------------------------------------------------------------------------

#define NIXIE_SERIAL_TX_PIN       0     // GPIO0
#define NIXIE_SERIAL_BAUD         115200
#define NIXIE_SERIAL_HEADER       0xAA

#define MAX_SLAVE_MODE_FAIL_COUNT 20

// -------------------------------------------------------------------------------
#define SLAVE_NIX_MODE_MIN                  0
#define SLAVE_NIX_MODE_100THS               0
#define SLAVE_NIX_MODE_DATE                 1
#define SLAVE_NIX_MODE_SECS                 2
#define SLAVE_NIX_MODE_OFF                  3
#define SLAVE_NIX_MODE_MAX                  3
#define SLAVE_NIX_MODE_DEFAULT              0

class SlaveManagerNixie_ {
  private:
    SlaveManagerNixie_() = default; // Make constructor private

  public:
    static SlaveManagerNixie_ &getInstance(); // Accessor for singleton instance

    SlaveManagerNixie_(const SlaveManagerNixie_ &) = delete; // no copying
    SlaveManagerNixie_ &operator=(const SlaveManagerNixie_ &) = delete;

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
    void setBlankingStatus(bool newStatus);
    void setDimmingStatus(bool newStatus);
  private:
    bool _slaveEnabled = true;
    bool _blanked;
    bool _dimmed = false;
    unsigned int _slaveModeFailCount;
    unsigned int _slaveModeTryCount;
    byte previousMode = 255;
    byte getNextSlaveMode();
    void sendUpdateToSlaveSerial();
    void blankSlaveSerial();
};

extern SlaveManagerNixie_ &slaveManagerNixie;