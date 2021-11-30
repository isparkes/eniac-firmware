#pragma once

#include "Arduino.h"

#define MODE_STATUS 0
#define MODE_CHASE  1

typedef struct {
  bool bl1;
  bool bl2;
  bool bl3;
  bool bl4;
  bool bl5;
  bool bl6;
} blinkelights_t;

class BlinkenlightsManager_ {
  private:
    BlinkenlightsManager_() = default; // Make constructor private

  public:
    static BlinkenlightsManager_ &getInstance(); // Accessor for singleton instance

    BlinkenlightsManager_(const BlinkenlightsManager_ &) = delete; // no copying
    BlinkenlightsManager_ &operator=(const BlinkenlightsManager_ &) = delete;

  public:
    void begin();
    void setBlinkenlightsMode(uint8_t mode);
    void setBlinkenlightsStatus();
    void setBlinkenlightsChase();
    void setBlinkenlightsExtern(blinkelights_t *blext);
    void updateBlinkenlights();
    blinkelights_t* getBlinkenlights();
  private:
    blinkelights_t blinkenLights;
    blinkelights_t *bl = &blinkenLights;
    uint8_t mode;
};

extern BlinkenlightsManager_ &blinkenlightsManager;