#pragma once

#include "Arduino.h"

#define BLNKN_MODE_MIN     0
#define BLNKN_MODE_STATUS  0
#define BLNKN_MODE_CHASE   1
#define BLNKN_MODE_MAX     1
#define BLNKN_MODE_DEFAULT BLNKN_MODE_STATUS

#define BLNKN_STATUS_MIN      0
#define BLNKN_STATUS_BLANKING 0
#define BLNKN_STATUS_PIR      1
#define BLNKN_STATUS_GPS      2
#define BLNKN_STATUS_NTP      3
#define BLNKN_STATUS_WIFI     4
#define BLNKN_STATUS_PIR_INST 5
#define BLNKN_STATUS_UP_DOWN  6
#define BLNKN_STATUS_1PPS     7
#define BLNKN_STATUS_MAX      7

typedef struct {
  bool bl1;
  bool bl2;
  bool bl3;
  bool bl4;
  bool bl5;
  bool bl6;
  bool in1;
  bool in2;
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
    void updateBlinkenlights();
    byte getNextBlinkenlightsMode();
    String getNextBlinkenlightsModeName();
    blinkenlights_t* getBlinkenlights();
  private:
    blinkenlights_t blinkenLights;
    blinkenlights_t *bl = &blinkenLights;
    void setBlinkenlightsChase();
    void setBlinkenlightsStatus();
    bool getBlinkenlightStatusValue(int blValue);
};

extern BlinkenlightsManager_ &blinkenlightsManager;