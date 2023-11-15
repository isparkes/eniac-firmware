#include "LDRManager.h"

// ************************************************************
// Set up the component
// ************************************************************
void LDRManager_::setUp() {
  pinMode(LDRPin, INPUT);
  #ifdef LDR_EXTENDED_DEBUG
  debugMsgLdr("Config useLDR: " + String(cc->useLDR));
  debugMsgLdr("Config sensitivityLDR: " + String(cc->sensitivityLDR));
  debugMsgLdr("Config thresholdBright: " + String(cc->thresholdBright));
  debugMsgLdr("Config sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));
  debugMsgLdr("Config minDim%: " + String(cc->minDim));
  debugMsgLdr("Config setDim%: " + String(cc->setDim));
  #endif

  recalculateVariables();
  setUpPWM();
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
  ledcWrite(LDRPWMChannel, _ldrValue);
}

// ************************************************************
// Recalculate the per-config or slow moving variables
// ************************************************************
void LDRManager_::recalculateVariables() {
  _minDimLDR = LDR_VALUE_MAX - (cc->minDim * LDR_VALUE_MAX / 100);
  _maxDimLDR = 0;
  _setDimLDR = LDR_VALUE_MAX - (cc->setDim * LDR_VALUE_MAX / 100);

  // Scaling offset increases the base brightness
  // factor increases the sensitivity
  _offset = cc->thresholdBright;
  _factor = cc->sensitivityLDR / 200.0;
}

// ************************************************************
// Gets the smoothed LDR Reading and store it
// ************************************************************
void LDRManager_::getDimmingFromLDR() {
  _isMaxDim = _isMinDim = false;
  if (_setMinDim) {
    _ldrValue = _minDimLDR;
    _isMinDim = true;
  } else if (_setMaxDim || _setMaxDimACP) {
    _ldrValue = _maxDimLDR;
    _isMaxDim = true;
  } else if (cc->useLDR) {
    int rawLDR = analogRead(LDRPin);
    int rawSensorVal = rawLDR;

    double sensorDiff = rawSensorVal - sensorLDRSmoothed;
    sensorLDRSmoothed += (sensorDiff / (double) cc->sensorSmoothCountLDR);

    _ldrValue = (sensorLDRSmoothed - _offset) * _factor;
  } else {
    _ldrValue = LDR_VALUE_MAX - (cc->setDim * LDR_VALUE_MAX / 100);
  }

  if (_ldrValue >= _minDimLDR) {
    _ldrValue = _minDimLDR;
    _isMinDim = true;
  }
  if (_ldrValue <= 0) {
    _ldrValue = 0;
    _isMaxDim = true;
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
  return (LDR_VALUE_MAX - _ldrValue) / (float) LDR_VALUE_MAX * 100.0;
}

// ************************************************************
// Set the brightest LDR value - resets any previous min dim
// state. 
// ************************************************************
void LDRManager_::setLDRValueToMax(bool newState) {
  _setMaxDim = newState;
  if (newState) {
    // if we're in Max Dim, we can't be in Min 
    _setMinDim = false;
  }
}

// ************************************************************
// Set the brightest LDR value - does not reset anything - used
// for ACP
// ************************************************************
void LDRManager_::setLDRValueToMaxACP(bool newState) {
  _setMaxDimACP = newState;
}

// ************************************************************
// Set the dimmest LDR value
// ************************************************************
void LDRManager_::setLDRValueToMin(bool newState) {
  _setMinDim = newState;
  if (newState) {
    // if we're in Min Dim, we can't be in Max 
    _setMaxDim = false;
  }
}

// ************************************************************
// Return if we are in an imposed dimming value - either min,
// max or set value
// ************************************************************
bool LDRManager_::getIsFixedLDRValue() {
  return _setMinDim || _setMaxDim || !cc->useLDR;
}

// ************************************************************
// Return if the LDR has been set to min or if it is naturally
// set min dim
// ************************************************************
bool LDRManager_::isMinDim() {
  return _isMinDim;
}

// ************************************************************
// Return if the LDR has been set to max
// ************************************************************
bool LDRManager_::isMaxDim() {
  return _isMaxDim;
}

LDRManager_ &LDRManager_::getInstance() {
  static LDRManager_ instance;
  return instance;
}

LDRManager_ &ldrManager = ldrManager.getInstance();