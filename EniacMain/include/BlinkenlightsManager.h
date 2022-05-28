#pragma once

#include "Arduino.h"

#define BLNKN_MODE_MIN     0
#define BLNKN_MODE_STATUS  0
#define BLNKN_MODE_CHASE   1
#define BLNKN_MODE_MAX     1
#define BLNKN_MODE_DEFAULT BLNKN_MODE_STATUS

typedef struct {
  bool bl1;
  bool bl2;
  bool bl3;
  bool bl4;
  bool bl5;
  bool bl6;
} blinkenlights_t;

class BlinkenlightsManager_ {
  private:
    BlinkenlightsManager_() = default; // Make constructor private

  public:
    static BlinkenlightsManager_ &getInstance(); // Accessor for singleton instance

    BlinkenlightsManager_(const BlinkenlightsManager_ &) = delete; // no copying
    BlinkenlightsManager_ &operator=(const BlinkenlightsManager_ &) = delete;

  public:
    void setBlinkenlightsMode(byte newMode);
    void setNextBlinkenlightsMode();
    void setBlinkenlightsExtern(blinkenlights_t *blext);
    void updateBlinkenlights();
    byte getNextBlinkenlightsMode();
    String getNextBlinkenlightsModeName();
    blinkenlights_t* getBlinkenlights();
  private:
    blinkenlights_t blinkenLights;
    blinkenlights_t *bl = &blinkenLights;
    void setBlinkenlightsChase();
    void setBlinkenlightsStatus();
};

extern BlinkenlightsManager_ &blinkenlightsManager;