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
  ledcWrite(LDRPWMChannel, _ldrValueTube);
}

// ************************************************************
// Do the work for the fast moving variables
// ************************************************************
void LDRManager_::updateOncePerLoop() {
  processLDRValue();

  // Set the PWM value based on the new values
  ledcWrite(LDRPWMChannel, _ldrValueTube);
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
  _minDimTube = LDR_VALUE_MAX - (cc->minTubeDim * LDR_VALUE_MAX / 100);
  _maxDimTube = 0;
  _setDimTube = LDR_VALUE_MAX - (cc->setTubeDim * LDR_VALUE_MAX / 100);

  _minDimBL = LDR_VALUE_MAX - (cc->minBLDim * LDR_VALUE_MAX / 100);
  _maxDimBL = 0;
  _setDimBL = LDR_VALUE_MAX - (cc->setBLDim * LDR_VALUE_MAX / 100);

  // Scaling offset increases the base brightness
  // factor increases the sensitivity
  _offset = cc->thresholdBright;
  _factor = cc->sensitivityLDR / 200.0;
}

// ************************************************************
// Calculates the smoothed LDR Reading for the tubes and backlights
// ************************************************************
void LDRManager_::processLDRValue() {
  int calculatedLDRValTube = 0;
  int calculatedLDRValBL = 0;
  _rawLDRValue = analogRead(LDRPin);

  #ifdef LDR_EXTENDED_DEBUG
  debugMsgLdr("Using raw LDR reading: " + String(rawLDR));
  #endif

  if (_setMinDim) {
    calculatedLDRValTube = _minDimTube;
    calculatedLDRValBL   = _minDimBL;
  } else if (_setMaxDim) {
    calculatedLDRValTube = _maxDimTube;
    calculatedLDRValBL   = _maxDimBL;
  } else {
    calculatedLDRValTube = _setDimTube;
    calculatedLDRValBL   = _setDimBL;
  }

  if (cc->useLDRTube) {
    calculatedLDRValTube = ((double)_rawLDRValue - _offset) * _factor;
  }

  if (cc->useLDRBL) {
    calculatedLDRValBL = ((double)_rawLDRValue - _offset) * _factor;
  }

  // Tube calculation with ACP
  if (_setMaxDimACP) {
    calculatedLDRValTube = _maxDimTube;
  }

  double sensorDiff = (double)calculatedLDRValTube - _sensorLDRSmoothedTube;
  _sensorLDRSmoothedTube += (sensorDiff / (double) cc->sensorSmoothCountLDR);
  _ldrValueTube = (int) _sensorLDRSmoothedTube;

  sensorDiff = (double)calculatedLDRValBL - _sensorLDRSmoothedBL;
  _sensorLDRSmoothedBL += (sensorDiff / (double) cc->sensorSmoothCountLDR);
  _ldrValueBL = (int) _sensorLDRSmoothedBL;

  // calculate the bound tube value and set the
  // min/max values based on the tube (non-ACP) value  
  if (_ldrValueTube >= _minDimTube) {
    _ldrValueTube = _minDimTube;
    _isMinDim = true;
    _isMaxDim = false;
  } else if (_ldrValueTube <= _maxDimTube) {
    _ldrValueTube = _maxDimTube;
    _isMinDim = false;
    _isMaxDim = true;
  } else {
    _isMinDim = false;
    _isMaxDim = false;
  }

  // calculate the bound BL value and set the
  if (_ldrValueBL >= _minDimBL) {
    _ldrValueBL = _minDimBL;
  } else if (_ldrValueBL <= _maxDimBL) {
    _ldrValueBL = _maxDimBL;
  }

  #ifdef LDR_EXTENDED_DEBUG
  if (_isMinDim) debugMsgLdr("MIN LDR");
  if (_isMaxDim) debugMsgLdr("MAX LDR");
  debugMsgLdr("Sensordiff: " + String(sensorDiff));
  debugMsgLdr("sensorLDRSmoothed: " + String(sensorLDRSmoothed));
  debugMsgLdr("Smoothed LDR reading: " + String(localLDRValue));
  #endif
}

// ************************************************************
// Return previously calculated RAW value, range 0 - 4095
// ************************************************************
int LDRManager_::getRawLDRValue() {
  return _rawLDRValue;
}

// ************************************************************
// Return previously calculated RAW value, range 0 - 4095
// ************************************************************
int LDRManager_::getLDRValueTube() {
  return _ldrValueTube;
}

// ************************************************************
// Return previously calculated value, range 0 - 100
// ************************************************************
float LDRManager_::getLDRValueTubePct() {
  return (LDR_VALUE_MAX - _ldrValueTube) / (float) LDR_VALUE_MAX * 100.0;
}

// ************************************************************
// Return previously calculated RAW value, range 0 - 4095
// ************************************************************
int LDRManager_::getLDRValueBL() {
  return _ldrValueBL;
}

// ************************************************************
// Return previously calculated value, range 0 - 100
// ************************************************************
float LDRManager_::getLDRValueBLPct() {
  return (LDR_VALUE_MAX - _ldrValueBL) / (float) LDR_VALUE_MAX * 100.0;
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