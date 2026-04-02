#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "Globals.h"
#include "BlankingManager.h"

// --------------------------------- Protocol ------------------------------------
// Serial communication once per second (UART2, 115200 baud, TX on GPIO0)
//   Byte 0: Start byte (0xAA)
//   Byte 1: Hours   (0-23)
//   Byte 2: Minutes (0-59)
//   Byte 3: Seconds (0-59)
//   Byte 4: Control
//     Bit 0:   Blanked (1 = display is blanked)
//     Bits 1-4: Primary display mode (cc->pMode)
// -------------------------------------------------------------------------------

#define DECATRON_SERIAL_TX_PIN        0     // GPIO0 (D0)
#define DECATRON_SERIAL_BAUD          115200

// Control byte bit masks
#define DECATRON_CTRL_BLANKED         0x01  // Bit 0: display is blanked
#define DECATRON_CTRL_MODE_SHIFT      1     // Bits 1-4: primary display mode

// -------------------------------------------------------------------------------
#define SLAVE_DECA_MODE_MIN                  0
#define SLAVE_DECA_MODE_MINS_SECS            0
#define SLAVE_DECA_MODE_HOURS_MINS           1
#define SLAVE_DECA_MODE_SPINNER              2
#define SLAVE_DECA_MODE_OFF                  3
#define SLAVE_DECA_MODE_MAX                  3
#define SLAVE_DECA_MODE_DEFAULT              1

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
    unsigned int getTryCount();
    unsigned int getFailCount();
  private:
    bool _slaveEnabled = true;
    unsigned int _slaveTryCount = 0;
    unsigned int _slaveFailCount = 0;
    void sendUpdateToSlaveSerial();
};

extern SlaveManagerDecatron_ &slaveManagerDecatron;
