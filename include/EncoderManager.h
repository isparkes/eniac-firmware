#pragma once

#include <Arduino.h>
#include <ESP32Encoder.h>
#include "DebugManager.h"
#include <Arduino.h>
#include "defs.h"
#include "globals.h"

class EncoderManager_ {
  private:
    EncoderManager_() = default; // Make constructor private

  public:
    static EncoderManager_ &getInstance(); // Accessor for singleton instance

    EncoderManager_(const EncoderManager_ &) = delete; // no copying
    EncoderManager_ &operator=(const EncoderManager_ &) = delete;

  public:
    void setup();
    int getCount();
    bool getButtonState();
    bool isAttached();
    void clearCount();
    void setDebugOutput(bool newDebug);

    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  private:
    ESP32Encoder _encoder;
    DebugCallback _dbcb;
    bool _debug = false;
    unsigned long _lastSwitchIntr = 0;

    void debugMsg(String message);                        // print a debug message to the callback
//    void IRAM_ATTR ENC_BTN_ISR();
};

extern EncoderManager_ &encoderManager;