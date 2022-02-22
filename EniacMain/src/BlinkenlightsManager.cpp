#include "BlinkenlightsManager.h"

#include <Arduino.h>

#include "globals.h"

#include "BlankingManager.h"
#include "GPSManager.h"
#include "NTPManager.h"
#include "TimeLib.h"

byte BlinkenlightsManager_::getNextBlinkenlightsMode(byte currentMode) {
  byte newMode = currentMode++;
  if (newMode > BLNKN_MODE_MAX) {
    newMode = BLNKN_MODE_MIN;
  }
  return newMode;
}

void BlinkenlightsManager_::setBlinkenlightsMode(byte newMode) {
  cc->blinkenLightsMode = newMode;
  
}

void BlinkenlightsManager_::setBlinkenlightsStatus() {
  bl->bl1 = blankingManager.getCurrentBlankingStatus();
  bl->bl2 = blankingManager.getCurrentPIRStatus();
  
  if (gpsManager.getGPSTimeValid()) {
    bl->bl3 = true;
  } else if (gpsManager.getGPSSyncStarted()) {
    bl->bl3 = (second() % 2 == 0);
  }
  bl->bl4 = ntpManager.ntpTimeValid();
  
  bl->bl5 = WiFi.isConnected();
  bl->bl6 = blankingManager.getCurrentPIRInstalled();

}

void BlinkenlightsManager_::setBlinkenlightsChase() {
  switch (second() % 6) {
    case 0:
      bl->bl1 = true;
      bl->bl6 = false;
      break;
    case 1:
      bl->bl2 = true;
      bl->bl1 = false;
      break;
    case 2:
      bl->bl3 = true;
      bl->bl2 = false;
      break;
    case 3:
      bl->bl4 = true;
      bl->bl3 = false;
      break;
    case 4:
      bl->bl5 = true;
      bl->bl4 = false;
      break;
    case 5:
      bl->bl6 = true;
      bl->bl5 = false;
      break;
  }
}

void BlinkenlightsManager_::setBlinkenlightsExtern(blinkelights_t *blext) {
  bl->bl1 = blext->bl1;
  bl->bl2 = blext->bl2;
  bl->bl3 = blext->bl3;
  bl->bl4 = blext->bl4;
  bl->bl5 = blext->bl5;
  bl->bl6 = blext->bl6;
}

void BlinkenlightsManager_::updateBlinkenlights() {
  switch (cc->blinkenLightsMode) {
    case BLNKN_MODE_STATUS:
      setBlinkenlightsStatus();
      break;
    case BLNKN_MODE_CHASE:
      setBlinkenlightsChase();
      break;
  }
}

blinkelights_t* BlinkenlightsManager_::getBlinkenlights() {
  return bl;
}

BlinkenlightsManager_ &BlinkenlightsManager_::getInstance() {
  static BlinkenlightsManager_ instance;
  return instance;
}

BlinkenlightsManager_ &blinkenlightsManager = blinkenlightsManager.getInstance();