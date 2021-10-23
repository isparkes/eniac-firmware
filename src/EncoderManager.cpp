#include "EncoderManager.h"
#include <Arduino.h>
#include "defs.h"

void EncoderManager_::setup() {
  pinMode(ENC_BTN, INPUT_PULLUP);

  ESP32Encoder::useInternalWeakPullResistors=UP;
  _encoder.attachHalfQuad(ENC_APin, ENC_BPin);
  	
  // clear the encoder's raw count and set the tracked count to zero
  _encoder.clearCount();
}

int EncoderManager_::getCount() {
  return _encoder.getCount()/2 % 6;
}

EncoderManager_ &EncoderManager_::getInstance() {
  static EncoderManager_ instance;
  return instance;
}

EncoderManager_ &encoderManager = encoderManager.getInstance();