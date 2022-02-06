#include "EncoderManager.h"
// The interrupt is much too sensitive - we will revert to polling
// #include <FunctionalInterrupt.h>

void EncoderManager_::setup() {
  pinMode(ENC_BTN, INPUT_PULLUP);

  ESP32Encoder::useInternalWeakPullResistors=UP;
  _encoder.attachHalfQuad(ENC_APin, ENC_BPin);
  	
  // clear the encoder's raw count and set the tracked count to zero
  _encoder.clearCount();

  // The interrupt is much too sensitive - we will revert to polling
  // attachInterrupt(ENC_BTN, std::bind(&EncoderManager_::ENC_BTN_ISR,this), FALLING);
}

int EncoderManager_::getCount() {
  debugMsg("Encoder count: " + String((int)_encoder.getCount()));
  return _encoder.getCount()/2;
}

bool EncoderManager_::getButtonState() {
  return digitalRead(ENC_BTN);
}

bool EncoderManager_::isAttached() {
  return _encoder.isAttached();
}

void EncoderManager_::clearCount() {
  _encoder.clearCount();
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void EncoderManager_::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("[ENC]: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void EncoderManager_::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// void IRAM_ATTR EncoderManager_::ENC_BTN_ISR()
// {
//   unsigned long nowMillis = millis();
//   if ((nowMillis - _lastSwitchIntr) < 300) {
//     return;
//   }

//   // Don't react to bounces on release
//   if (digitalRead(ENC_BTN) == HIGH) {
//     return;
//   }

//   _lastSwitchIntr = nowMillis;
//   configTimeout = OLED_ON_TIME;
//   if (configTimeout == 0) {
//     configMode = true;
//     configStep++;
//   } else {
//     if (configStep < 3) {
//       configStep++;
//     }
//   }
// }

// ************************************************************
// set the update interval
// ************************************************************
void EncoderManager_::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}

EncoderManager_ &EncoderManager_::getInstance() {
  static EncoderManager_ instance;
  return instance;
}

EncoderManager_ &encoderManager = encoderManager.getInstance();