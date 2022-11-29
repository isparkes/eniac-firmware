#include "LDRManager.h"

// ************************************************************
// Set up the component
// ************************************************************
void LDRManager_::setUp() {
  pinMode(LDRPin, INPUT);
  #ifdef LDR_EXTENDED_DEBUG_ON
  debugMsgLdr("Config useLDR: " + String(cc->useLDR));
  debugMsgLdr("Config sensitivityLDR: " + String(cc->sensitivityLDR));
  debugMsgLdr("Config thresholdBright: " + String(cc->thresholdBright));
  debugMsgLdr("Config sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));
  debugMsgLdr("Config minDim%: " + String(cc->minDim));
  debugMsgLdr("Config setDim%: " + String(cc->setDim));
  #endif
}

// ************************************************************
// Start the PWM - broken out so that we can do the startup
// Sequence
// ************************************************************
void LDRManager_::setUpPWM() {
  debugMsgLdr("Start up dimming PWM");
  const int PWMFreq = 500; /* Hz */
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
    _ldrValue = LDR_VALUE_MAX - (cc->setDim * LDR_VALUE_MAX / 100);
  }

  ledcWrite(LDRPWMChannel, _ldrValue);
}

// ************************************************************
// Return previously calculated value, range 0 - 4095
// ************************************************************
int LDRManager_::getLDRValue() {
  return _ldrValue;
}

// ************************************************************
// Return previously calculated value, range 0 - 100
// ************************************************************
float LDRManager_::getLDRValuePct() {
  return (LDR_VALUE_MAX - ldrValue) / (float) LDR_VALUE_MAX * 100.0;
}

// ************************************************************
// Set the brightest LDR value
// ************************************************************
void LDRManager_::setLDRValueToMax() {
  _locked = true;
  ledcWrite(LDRPWMChannel, 0);
}

// ************************************************************
// Set the dimmest LDR value
// ************************************************************
void LDRManager_::setLDRValueToMin() {
  _locked = true;
  int effectiveMinDim = LDR_VALUE_MAX - (cc->minDim * LDR_VALUE_MAX / 100);
  _isMinDim = true;
  ledcWrite(LDRPWMChannel, effectiveMinDim);
}

// ************************************************************
// Reset an imposed LDR (max/min) value
// ************************************************************
void LDRManager_::resetFixedLDRValue() {
  _locked = false;
  ledcWrite(LDRPWMChannel, _ldrValue);
}

// ************************************************************
// Return brightest LDR value
// ************************************************************
bool LDRManager_::getIsFixedLDRValue() {
  return _locked;
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