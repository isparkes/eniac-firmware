#include "LDRManager.h"

void LDRManager_::setUp()
{
  pinMode(LDRPin, INPUT);
  #ifdef DEBUG_ON
  debugMsg("Config useLDR: " + String(cc->useLDR));
  debugMsg("Config sensitivityLDR: " + String(cc->sensitivityLDR));
  debugMsg("Config thresholdBright: " + String(cc->thresholdBright));
  debugMsg("Config sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));
  debugMsg("Config minDim %: " + String(cc->minDim));
  #endif

  #ifdef DEBUG_ON
  debugMsg("Start up dimming PWM");
  #endif
  const int PWMFreq = 1000; /* 1 KHz */
  const int PWMResolution = 12;
  const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

  ledcSetup(LDRPWMChannel, PWMFreq, PWMResolution);
  ledcAttachPin(BLANKPin, LDRPWMChannel);
  ledcWrite(LDRPWMChannel, MAX_DUTY_CYCLE);
}

// ************************************************************
// Gets the smoothed LDR Reading and store it
// ************************************************************
void LDRManager_::getDimmingFromLDR() {
  if (_locked) return;
  
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

    int effectiveMinDim = LDR_VALUE_MAX - (cc->minDim * LDR_VALUE_MAX / 100);

    if (returnValue >= effectiveMinDim) {
      returnValue = effectiveMinDim;
      _isMinDim = true;
      #ifdef DEBUG_ON
      debugMsg("Clamping _ldrValue to min: " + String(returnValue));
      #endif
    } else {
      _isMinDim = false;
    }
    if (returnValue < 0) {
      returnValue = 0;
      #ifdef DEBUG_ON
      debugMsg("Clamping _ldrValue to max: " + String(returnValue));
      #endif
    }
    _ldrValue = returnValue;
  } else {
    _ldrValue = LDR_VALUE_MAX - (cc->minDim * LDR_VALUE_MAX / 100);
    #ifdef DEBUG_ON
    debugMsg("Not using _ldrValue setting to min: " + String(cc->minDim));
    #endif
  }

  ledcWrite(LDRPWMChannel, _ldrValue);
}

// ************************************************************
// Return previously calculated value
// ************************************************************
int LDRManager_::getLDRValue() {
  return _ldrValue;
}

// ************************************************************
// Return brightest LDR value
// ************************************************************
void LDRManager_::setLDRValueToMax() {
  _locked = true;
  ledcWrite(LDRPWMChannel, 0);
}

// ************************************************************
// Return brightest LDR value
// ************************************************************
void LDRManager_::resetMaxLDRValue() {
  _locked = false;
  ledcWrite(LDRPWMChannel, _ldrValue);
}

// ************************************************************
// Return brightest LDR value
// ************************************************************
bool LDRManager_::isMinLDRValue() {
  return _isMinDim;
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void LDRManager_::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("[LDR]: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void LDRManager_::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// ************************************************************
// set the update interval
// ************************************************************
void LDRManager_::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}

LDRManager_ &LDRManager_::getInstance() {
  static LDRManager_ instance;
  return instance;
}

LDRManager_ &ldrManager = ldrManager.getInstance();