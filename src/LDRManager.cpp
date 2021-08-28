#include "LDRManager.h"

void LDRManager::setUp()
{
  pinMode(LDRPin, INPUT);
  #ifdef DEBUG_ON
  debugMsg("Config useLDR: " + String(cc->useLDR));
  debugMsg("Config sensitivityLDR: " + String(cc->sensitivityLDR));
  debugMsg("Config thresholdBright: " + String(cc->thresholdBright));
  debugMsg("Config sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));
  debugMsg("Config minDim: " + String(cc->minDim));
  #endif
}

// ************************************************************
// Gets the smoothed LDR Reading and store it
// ************************************************************
void LDRManager::getDimmingFromLDR() {
  if (cc->useLDR) {
    int rawLDR = analogRead(LDRPin);
    #ifdef DEBUG_ON
    debugMsg("-----------------");
    debugMsg("Raw LDR Value: " + String(rawLDR));
    #endif
    int rawSensorVal = rawLDR;

    double sensorDiff = rawSensorVal - sensorLDRSmoothed;
    sensorLDRSmoothed += (sensorDiff / (double) cc->sensorSmoothCountLDR);
    #ifdef DEBUG_ON
    debugMsg("Smoothed LDR Value: " + String(sensorLDRSmoothed));
    #endif

    // Scaling offset increases the base brightness
    // factor increases the sensitivity
    double offset = cc->thresholdBright;
    double factor = cc->sensitivityLDR / 200.0;

    int returnValue = (sensorLDRSmoothed - offset) * factor;
    
    #ifdef DEBUG_ON
    debugMsg("Raw _ldrValue: " + String(returnValue));
    #endif

    if (returnValue > (LDR_VALUE_MAX - cc->minDim)) {
      returnValue = LDR_VALUE_MAX - cc->minDim;
      #ifdef DEBUG_ON
      debugMsg("Clamping _ldrValue to min: " + String(returnValue));
      #endif
    }
    if (returnValue < 0) {
      returnValue = 0;
      #ifdef DEBUG_ON
      debugMsg("Clamping _ldrValue to max: " + String(returnValue));
      #endif
    }
    _ldrValue = returnValue;
  } else {
    #ifdef DEBUG_ON
    debugMsg("Not using _ldrValue setting to min: " + String(cc->minDim));
    _ldrValue = LDR_VALUE_MAX - cc->minDim;
    #endif
  }
}

// ************************************************************
// Return previously calculated value
// ************************************************************
int LDRManager::getLDRValue() {
  return _ldrValue;
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void LDRManager::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("LDR: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void LDRManager::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// ************************************************************
// set the update interval
// ************************************************************
void LDRManager::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}