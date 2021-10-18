#include "BlinkenlightsManager.h"

#include <Arduino.h>

#include "globals.h"

#include "BlankingManager.h"
#include "GPSManager.h"
#include "NTPManager.h"
#include "TimeLib.h"

void BlinkenlightsManager_::begin() {
}

void BlinkenlightsManager_::setBlinkenlightsStatus(blinkelights_t *bl) {
  bl->bl1 = blankingManager.getCurrentBlankingStatus();
  bl->bl2 = blankingManager.getCurrentPIRStatus();
  
  if (gpsManager.getGPSTimeValid(nowMillis)) {
    bl->bl3 = true;
  } else if (gpsManager.getGPSSyncStarted(nowMillis)) {
    bl->bl3 = (second() % 2 == 0);
  }
  bl->bl4 = ntpManager.ntpTimeValid(nowMillis);
  
  bl->bl5 = WiFi.isConnected();
  bl->bl6 = blankingManager.getCurrentPIRInstalled();

}

void BlinkenlightsManager_::setBlinkenlightsChase(blinkelights_t *bl) {
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

BlinkenlightsManager_ &BlinkenlightsManager_::getInstance() {
  static BlinkenlightsManager_ instance;
  return instance;
}

BlinkenlightsManager_ &blinkenlightsManager = blinkenlightsManager.getInstance();