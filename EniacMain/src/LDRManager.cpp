#include "LDRManager.h"

// ************************************************************
// Set up the component
// ************************************************************
void LDRManager_::setUp() {
  pinMode(LDRPin, INPUT);
  debugMsgLdr("Config useLDR: " + String(cc->useLDR));
  debugMsgLdr("Config sensitivityLDR: " + String(cc->sensitivityLDR));
  debugMsgLdr("Config thresholdBright: " + String(cc->thresholdBright));
  debugMsgLdr("Config sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));
  debugMsgLdr("Config minDim %: " + String(cc->minDim));
}

// ************************************************************
// Start the PWM - broken out so that we can do the startup
// Sequence
// ************************************************************
void LDRManager_::setUpPWM() {
  debugMsgLdr("Start up dimming PWM");
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
    int rawSensorVal = rawLDR;

    double sensorDiff = rawSensorVal - sensorLDRSmoothed;
    sensorLDRSmoothed += (sensorDiff / (double) cc->sensorSmoothCountLDR);

    // Scaling offset increases the base brightness
    // factor increases the sensitivity
    double offset = cc->thresholdBright;
    double factor = cc->sensitivityLDR / 200.0;

    int returnValue = (sensorLDRSmoothed - offset) * factor;
    int effectiveMinDim = LDR_VALUE_MAX - (cc->minDim * LDR_VALUE_MAX / 100);

    if (returnValue >= effectiveMinDim) {
      returnValue = effectiveMinDim;
      _isMinDim = true;
    } else {
      _isMinDim = false;
    }
    if (returnValue < 0) {
      returnValue = 0;
    }
    _ldrValue = returnValue;
  } else {
    _ldrValue = LDR_VALUE_MAX - (cc->minDim * LDR_VALUE_MAX / 100);
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

LDRManager_ &LDRManager_::getInstance() {
  static LDRManager_ instance;
  return instance;
}

LDRManager_ &ldrManager = ldrManager.getInstance();