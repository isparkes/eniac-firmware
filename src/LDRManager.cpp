#include "LDRManager.h"

void LDRManager::setUp()
{
  pinMode(LDRPin, INPUT);
  debugMsg("Config useLDR: " + String(_cc->useLDR));
  debugMsg("Config sensitivityLDR: " + String(_cc->sensitivityLDR));
  debugMsg("Config thresholdBright: " + String(_cc->thresholdBright));
  debugMsg("Config sensorSmoothCountLDR: " + String(_cc->sensorSmoothCountLDR));
  debugMsg("Config minDim: " + String(_cc->minDim));
}

// ************************************************************
// Gets the smoothed LDR Reading and store it
// ************************************************************
void LDRManager::getDimmingFromLDR() {
  if (_cc->useLDR) {
    int rawLDR = analogRead(LDRPin);
    debugMsg("-----------------");
    debugMsg("Raw LDR Value: " + String(rawLDR));
    int rawSensorVal = rawLDR;

    double sensorDiff = rawSensorVal - sensorLDRSmoothed;
    sensorLDRSmoothed += (sensorDiff / (double) _cc->sensorSmoothCountLDR);
    debugMsg("Smoothed LDR Value: " + String(sensorLDRSmoothed));

    // Scaling offset increases the base brightness
    // factor increases the sensitivity
    double offset = _cc->thresholdBright;
    double factor = _cc->sensitivityLDR / 200.0;

    int returnValue = (sensorLDRSmoothed + offset) / factor;
    
    debugMsg("Raw _ldrValue: " + String(returnValue));

    if (returnValue > (MIN_DIM_MAX - _cc->minDim)) {
      returnValue = MIN_DIM_MAX - _cc->minDim;
      debugMsg("Clamping _ldrValue to min: " + String(returnValue));
    }
    if (returnValue < 0) {
      returnValue = 0;
      debugMsg("Clamping _ldrValue to max: " + String(returnValue));
    }
    _ldrValue = returnValue;
  } else {
      debugMsg("Not using _ldrValue setting to min: " + String(_cc->minDim));
    _ldrValue = MIN_DIM_MAX - _cc->minDim;
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

// ************************************************************
// Set up the manager
// ************************************************************
void LDRManager::setConfigObject(spiffs_config_t* ccPtr) {
  _cc = ccPtr;
}
