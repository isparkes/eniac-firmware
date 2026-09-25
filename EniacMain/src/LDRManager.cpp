#include "LDRManager.h"

// ************************************************************
// Set up the component
// ************************************************************
void LDRManager_::setUp() {
  pinMode(LDRPin, INPUT);
  #ifdef LDR_EXTENDED_DEBUG
  debugMsgLdr("Config useLDRTube: " + String(cc->useLDRTube));
  debugMsgLdr("Config useLDRBL: " + String(cc->useLDRBL));
  debugMsgLdr("Config sensitivityLDR: " + String(cc->sensitivityLDR));
  debugMsgLdr("Config thresholdBright: " + String(cc->thresholdBright));
  debugMsgLdr("Config sensorSmoothCountLDR: " + String(cc->sensorSmoothCountLDR));
  debugMsgLdr("Config minDimTube%: " + String(cc->minTubeDim));
  debugMsgLdr("Config setDimTube%: " + String(cc->setTubeDim));
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
  ledcWrite(LDRPWMChannel, _pwmValueTube);
}

// ************************************************************
// Do the work for the slow moving variables
// ************************************************************
void LDRManager_::updateOncePerSecond() {
  recalculateVariables();
  #ifdef LDR_EXTENDED_DEBUG
  debugMsgLed("_ldrValueTube: " + String(_ldrValueTube));
  debugMsgLed("_minDimTube: " + String(_minDimTube));
  debugMsgLed("mindim: " + String(_isMinDim));
  #endif
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
  debugMsgLdr("Using raw LDR reading: " + String(_rawLDRValue));
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

    if (cc->useLDRTube) {
      calculatedLDRValTube = ((double)_rawLDRValue - _offset) * _factor;
    }

    if (cc->useLDRBL) {
      calculatedLDRValBL = ((double)_rawLDRValue - _offset) * _factor;
    }
  }

  // Tube calculation with ACP
  if (_setMaxDimACP) {
    calculatedLDRValTube = _maxDimTube;
  }

  // sensorSmoothCountLDR comes from config: 0 would give 0/0 = NaN,
  // which never recovers and locks the brightness
  double smoothCount = (cc->sensorSmoothCountLDR > 0) ? (double) cc->sensorSmoothCountLDR : 1.0;

  double sensorDiff = (double)calculatedLDRValTube - _sensorLDRSmoothedTube;
  _sensorLDRSmoothedTube += (sensorDiff / smoothCount);
  _ldrValueTube = (int) _sensorLDRSmoothedTube;

  // Blanking dim/off fades - tube only, does not affect BL
  updateBlankingFade(_ldrValueTube);

  // Blanking dim: fade tube towards min dim (ACP overrides)
  if (!_setMaxDimACP) {
    _ldrValueTube += (int)((_minDimTube - _ldrValueTube) * _dimFade);
  }

  sensorDiff = (double)calculatedLDRValBL - _sensorLDRSmoothedBL;
  _sensorLDRSmoothedBL += (sensorDiff / smoothCount);
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

  // Blanking off: fade tube output towards fully off
  _pwmValueTube = _ldrValueTube + (int)((LDR_VALUE_MAX - _ldrValueTube) * _offFade);

  // calculate the bound BL value
  if (_ldrValueBL >= _minDimBL) {
    _ldrValueBL = _minDimBL;
  } else if (_ldrValueBL <= _maxDimBL) {
    _ldrValueBL = _maxDimBL;
  }

  #ifdef LDR_EXTENDED_DEBUG
  if (_isMinDim) debugMsgLdr("MIN LDR");
  if (_isMaxDim) debugMsgLdr("MAX LDR");
  debugMsgLdr("Sensordiff: " + String(sensorDiff));
  debugMsgLdr("_sensorLDRSmoothedTube: " + String(_sensorLDRSmoothedTube));
  debugMsgLdr("Smoothed LDR reading: " + String(_sensorLDRSmoothedTube));
  #endif
}

// ************************************************************
// Move the blanking fades towards their targets at
// BLANKING_FADE_RATE, independent of the loop speed
// ************************************************************
void LDRManager_::updateBlankingFade(int baseTube) {
  unsigned long now = millis();
  unsigned long elapsed = now - _lastFadeMillis;
  _lastFadeMillis = now;

  // PWM units we may move in this step
  float stepUnits = (float)elapsed * BLANKING_FADE_RATE * LDR_VALUE_MAX / 100000.0;

  _dimFade = stepFade(_dimFade, _blankingDim, abs(_minDimTube - baseTube), stepUnits);

  int dimmedTube = baseTube + (int)((_minDimTube - baseTube) * _dimFade);
  _offFade = stepFade(_offFade, _blankingOff, LDR_VALUE_MAX - dimmedTube, stepUnits);
}

// ************************************************************
// Step a fade fraction towards its target (1.0 if target is
// true, else 0.0). Span is the PWM distance the full fade
// covers, so the brightness changes at a constant rate.
// ************************************************************
float LDRManager_::stepFade(float fade, bool target, int span, float stepUnits) {
  float step = (span > 0) ? stepUnits / span : 1.0;
  if (target) {
    fade += step;
    if (fade > 1.0) fade = 1.0;
  } else {
    fade -= step;
    if (fade < 0.0) fade = 0.0;
  }
  return fade;
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
// Set the dimmest LDR value. Next regular update will
// reset any previous max dim state.
// ************************************************************
void LDRManager_::setLDRValueToMin(bool newState) {
  _setMinDim = newState;
}

// ************************************************************
// Returns if we have an imposed mn dim
// ************************************************************
bool LDRManager_::getLDRValueSetToMin() {
  return _setMinDim;
}

// ************************************************************
// Set tube dim from blanking period (does not affect BL)
// ************************************************************
void LDRManager_::setBlankingDim(bool newState) {
  _blankingDim = newState;
}

// ************************************************************
// Set tube fade to off from blanking period (does not affect BL)
// ************************************************************
void LDRManager_::setBlankingOff(bool newState) {
  _blankingOff = newState;
}

// ************************************************************
// True once the tubes have fully faded out for blanking
// ************************************************************
bool LDRManager_::isBlankingFadeComplete() {
  return _offFade >= 1.0;
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