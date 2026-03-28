#include "BlinkenlightsManager.h"

#include <Arduino.h>

#include "Globals.h"

#include "BlankingManager.h"
#include "GPSManager.h"
#include "NTPManager.h"
#include "TimeLib.h"

byte BlinkenlightsManager_::getNextBlinkenlightsMode() {
  byte newMode = cc->blinkenLightsMode + 1;
  if (newMode > BLNKN_MODE_MAX) {
    newMode = BLNKN_MODE_MIN;
  }
  return newMode;
}

String BlinkenlightsManager_::getNextBlinkenlightsModeName() {
  byte nextMode = getNextBlinkenlightsMode();
  switch(nextMode) {
    case BLNKN_MODE_STATUS: {
      return "Status";
      break;
    }
    case BLNKN_MODE_CHASE: {
      return "Chase";
      break;
    }
    default: {
      return "Unknown";
    }
  }
}

void BlinkenlightsManager_::setNextBlinkenlightsMode() {
  byte newMode = getNextBlinkenlightsMode();
  debugMsgBlm("Set BL Mode: " + String(newMode));
  cc->blinkenLightsMode = newMode;
}

void BlinkenlightsManager_::setBlinkenlightsMode(byte newMode) {
  if (newMode > BLNKN_MODE_MAX) {
    newMode = BLNKN_MODE_MAX;
  }
  if (newMode < BLNKN_MODE_MIN) {
    newMode = BLNKN_MODE_MIN;
  }
  debugMsgBlm("Set BL Mode: " + String(newMode));
  cc->blinkenLightsMode = newMode;
}

void BlinkenlightsManager_::setBlinkenlightsStatus() {
  bl->bl1 = getBlinkenlightStatusValue(BLNKN_STATUS_BLANKING);
  bl->bl2 = getBlinkenlightStatusValue(BLNKN_STATUS_PIR);
  bl->bl3 = getBlinkenlightStatusValue(BLNKN_STATUS_GPS);
  bl->bl4 = getBlinkenlightStatusValue(BLNKN_STATUS_NTP);
  bl->bl5 = getBlinkenlightStatusValue(BLNKN_STATUS_WIFI);
  bl->bl6 = getBlinkenlightStatusValue(BLNKN_STATUS_PIR_INST);
}

bool BlinkenlightsManager_::getBlinkenlightStatusValue(int blValue) {
  switch (blValue)
  {
    case BLNKN_STATUS_BLANKING:
      return blankingManager.getCurrentBlankingIndicator();
    case BLNKN_STATUS_PIR:
      return blankingManager.getCurrentPIRStatus();
    case BLNKN_STATUS_GPS:
      {
        if (gpsManager.getGPSTimeValid()) {
          return true;
        } else if (gpsManager.getGPSSyncStarted()) {
          return (second() % 2 == 0);
        }
      }
    case BLNKN_STATUS_NTP:
      return ntpManager.ntpTimeValid();
    case BLNKN_STATUS_WIFI:
      return WiFi.isConnected();
    case BLNKN_STATUS_PIR_INST:
      return blankingManager.getCurrentPIRInstalled();
    case BLNKN_STATUS_UP_DOWN:
      return !upOrDown;
    default:
      return false;
  }
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

void BlinkenlightsManager_::setBlanked(bool blanked) {
  _blanked = blanked;
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
  if (_blanked) {
    bl->bl1 = bl->bl2 = bl->bl3 = bl->bl4 = bl->bl5 = bl->bl6 = false;
  }
}

blinkenlights_t* BlinkenlightsManager_::getBlinkenlights() {
  return bl;
}

BlinkenlightsManager_ &BlinkenlightsManager_::getInstance() {
  static BlinkenlightsManager_ instance;
  return instance;
}

BlinkenlightsManager_ &blinkenlightsManager = blinkenlightsManager.getInstance();