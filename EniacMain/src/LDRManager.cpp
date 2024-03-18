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
// Do the work for the fast moving variables
// ************************************************************
void LDRManager_::updateOncePerLoop() {
  getTubeDimmingFromLDR();
}

// ************************************************************
// Do the work for the slow moving variables
// ************************************************************
void LDRManager_::updateOncePerSecond() {
  recalculateVariables();
}

// ************************************************************
// Recalculate the per-config or slow moving variables
// ************************************************************
void LDRManager_::recalculateVariables() {
  _minDimLDR = LDR_VALUE_MAX - (cc->minTubeDim * LDR_VALUE_MAX / 100);
  _maxDimLDR = 0;
  _setDimLDR = LDR_VALUE_MAX - (cc->setTubeDim * LDR_VALUE_MAX / 100);

  // Scaling offset increases the base brightness
  // factor increases the sensitivity
  _offset = cc->thresholdBright;
  _factor = cc->sensitivityLDR / 200.0;
}

// ************************************************************
// Gets the smoothed LDR Reading for the tubes
// ************************************************************
void LDRManager_::getTubeDimmingFromLDR() {
  int calculatedLDRVal = 0; 
  if (_setMinDim) {
    calculatedLDRVal = _minDimLDR;
  } else if (_setMaxDim) {
    calculatedLDRVal = _maxDimLDR;
  } else if (cc->useLDRTube) {
    int rawLDR = analogRead(LDRPin);
    calculatedLDRVal = ((double)rawLDR - _offset) * _factor;
    #ifdef LDR_EXTENDED_DEBUG
    debugMsgLdr("Using raw LDR reading: " + String(rawLDR));
    debugMsgLdr("Adjusted LDR reading: " + String(_ldrValue));
    #endif
  } else {
    calculatedLDRVal = _setDimLDR;
  }

  // Non-ACP calculation
  double sensorDiff = (double)calculatedLDRVal - _sensorLDRSmoothed;
  _sensorLDRSmoothed += (sensorDiff / (double) cc->sensorSmoothCountLDR);
  _ldrValue = (int) _sensorLDRSmoothed;

  // ACP calculation
  if (_setMaxDimACP) {
    calculatedLDRVal = _maxDimLDR;
  }
  double sensorDiffACP = (double)calculatedLDRVal - _sensorLDRSmoothedACP;
  _sensorLDRSmoothedACP += (sensorDiffACP / (double) cc->sensorSmoothCountLDR);
  _ldrValueACP = (int) _sensorLDRSmoothedACP;

  // calculate the min/max values based on the tube (non-ACP) value  
  if (_ldrValue >= _minDimLDR) {
    _ldrValue = _minDimLDR;
    _isMinDim = true;
    _isMaxDim = false;
  } else if (_ldrValue <= _maxDimLDR) {
    _ldrValue = _maxDimLDR;
    _isMinDim = false;
    _isMaxDim = true;
  } else {
    _isMinDim = false;
    _isMaxDim = false;
  }

  #ifdef LDR_EXTENDED_DEBUG
  if (_isMinDim) debugMsgLdr("MIN LDR");
  if (_isMaxDim) debugMsgLdr("MAX LDR");
  debugMsgLdr("Sensordiff: " + String(sensorDiff));
  debugMsgLdr("sensorLDRSmoothed: " + String(sensorLDRSmoothed));
  debugMsgLdr("Smoothed LDR reading: " + String(localLDRValue));
  #endif

  ledcWrite(LDRPWMChannel, _ldrValueACP);
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
// Return previously calculated value, range 0 - 4095
// ************************************************************
int LDRManager_::getLDRValueACP() {
  return _ldrValueACP;
}

// ************************************************************
// Set the brightest LDR value. Next regular update will
// reset any previous min dim state. 
// ************************************************************
void LDRManager_::setLDRValueToMax(bool newState) {
  _setMaxDim = newState;
}

// ************************************************************
// Set the brightest LDR value - does not reset anything - used
// for ACP
// ************************************************************
void LDRManager_::setLDRValueToMaxACP(bool newState) {
  _setMaxDimACP = newState;
}

// ************************************************************
// Set the dimmest LDR value.. Next regular update will
// reset any previous max dim state.
// ************************************************************
void LDRManager_::setLDRValueToMin(bool newState) {
  _setMinDim = newState;
}

// ************************************************************
// Return if we are in an imposed dimming value - either min,
// max or set value
// ************************************************************
bool LDRManager_::getIsFixedLDRValue() {
  return _setMinDim || _setMaxDim || !cc->useLDRTube;
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