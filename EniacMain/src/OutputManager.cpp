#include "OutputManager.h"

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArrayTime() {
  numberArray[S1]  = second() % 10;
  numberArray[S10] = second() / 10;
  numberArray[M1]  = minute() % 10;
  numberArray[M10] = minute() / 10;
  if (cc->hourMode) {
    numberArray[H1]  = hourFormat12() % 10;
    numberArray[H10] = hourFormat12() / 10;
  } else {
    numberArray[H1]  = hour() % 10;
    numberArray[H10] = hour() / 10;
  }
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArrayBurn(byte value) {
  allBlanked();
  loadNumberArraySameValue(value % 10);
  displayType[value / 10] = NORMAL;
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArrayDate() {
  switch (cc->dateFormat) {
    case DATE_FORMAT_YYMMDD:
      numberArray[S1]  = day() % 10;
      numberArray[S10] = day() / 10;
      numberArray[M1]  = month() % 10;
      numberArray[M10] = month() / 10;
      numberArray[H1]  = (year() - 2000) % 10;
      numberArray[H10] = (year() - 2000) / 10;
      break;
    case DATE_FORMAT_MMDDYY:
      numberArray[S1]  = (year() - 2000) % 10;
      numberArray[S10] = (year() - 2000) / 10;
      numberArray[M1]  = day() % 10;
      numberArray[M10] = day() / 10;
      numberArray[H1]  = month() % 10;
      numberArray[H10] = month() / 10;
      break;
    case DATE_FORMAT_DDMMYY:
      numberArray[S1]  = (year() - 2000) % 10;
      numberArray[S10] = (year() - 2000) / 10;
      numberArray[M1]  = month() % 10;
      numberArray[M10] = month() / 10;
      numberArray[H1]  = day() % 10;
      numberArray[H10] = day() / 10;
      break;
  }
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArraySameValue(byte value) {
  byte val = value % 10;
  numberArray[S1]  = val;
  numberArray[S10] = val;
  numberArray[M1]  = val;
  numberArray[M10] = val;
  numberArray[H1]  = val;
  numberArray[H10] = val;
}

// ************************************************************
// Display preset
// ************************************************************
void OutputManager_::allNormal(bool leadingBlank) {

  if (leadingBlank)
    applyBlanking();
  else 
    displayType[H10] = NORMAL;

  displayType[H1]  = NORMAL;
  displayType[M10] = NORMAL;
  displayType[M1]  = NORMAL;
  displayType[S10] = NORMAL;
  displayType[S1]  = NORMAL;
}

// ************************************************************
// Display preset
// ************************************************************
void OutputManager_::allBlanked() {
  displayType[S1]  = BLANKED;
  displayType[S10] = BLANKED;
  displayType[M1]  = BLANKED;
  displayType[M10] = BLANKED;
  displayType[H1]  = BLANKED;
  displayType[H10] = BLANKED;
}

// ************************************************************
// Do a single complete display, including any fading and
// dimming requested. Prepares the display variables for
// the interrupt driven display output.
// This is the heart of the display processing!
// ************************************************************
void OutputManager_::outputDisplay() {
  processStunts();
  processSeparators();

  blinkenlights_t *bl = blinkenlightsManager.getBlinkenlights();
  byte tmpDispType;
  byte tmpDispTypeArray[DIGIT_COUNT];
  byte tmpNumberArray[DIGIT_COUNT];

  for ( int i = DIGIT_COUNT - 1 ; i >= 0  ; i -- ) {
    // Blanking
    if (blankingManager.getCurrentBlankTubes()) {
      tmpDispType = BLANKED;
    } else {
      tmpDispType = displayType[i];
    }

    // Digit blinking
    if (tmpDispType == BLINK) {
      if (upOrDown) {
      } else {
        tmpDispType = BLANKED;
      }
    }
    tmpDispTypeArray[i] = tmpDispType;

    switch(_outputMode) {
      case timeMode: {
        // Trigger scolling and fading - scolling takes precendence
        // _suppressEffects stops any effects for ACP
        if (numberArray[i] != currNumberArray[i]) {
          // Do scrollback when we are going to 0
          if ((numberArray[i] == 0) && cc->scrollback && (scrollCounter[i] == 0)) {
            scrollCounter[i] = (currNumberArray[i]+1) * cc->scrollSteps;
          } else if ((fadeState == 0)  && cc->fade) {
            // if we are not going to 0, set up the fade steps
            fadeState = cc->fadeSteps;
          } else if (fadeState == 0) {
            currNumberArray[i] = numberArray[i];
          }
        }

        if (scrollCounter[i] > 0) {
          scrollCounter[i] = scrollCounter[i] - 1;
          currNumberArray[i] = scrollCounter[i]/cc->scrollSteps;
          tmpNumberArray[i] = currNumberArray[i];
        } else {
          tmpNumberArray[i] = numberArray[i];
        }
        break;
      }
      default: {
        // No effects (fade/scrolling) during ACP or Slots
        currNumberArray[i] = numberArray[i];
        tmpNumberArray[i] = numberArray[i];
        break;
      }
    }
  }

  uint8_t tmpSwitchTime = 0;
  if (fadeState == 1) {
    fadeState = 0;
    for (byte j = 0 ; j < DIGIT_COUNT ; j++) {
      if (scrollCounter[j] == 0) {
        currNumberArray[j] = numberArray[j];
      }
    }
  } else if (fadeState > 0) {
    fadeState--;
    tmpSwitchTime = PHASE_MAX - (PHASE_MAX * fadeState / cc->fadeSteps);
  }

  uint32_t tmpnextVal1 = decodeFromNumberArray(
                                currNumberArray[H10], 
                                currNumberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl->bl1,
                                bl->bl2,
                                _led1State,
                                _led2State);
  uint32_t tmpnextVal2 = decodeFromNumberArray(
                                currNumberArray[M10], 
                                currNumberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl->bl3,
                                bl->bl4,
                                _led3State,
                                _led4State);
  uint32_t tmpnextVal3 = decodeFromNumberArray(
                                currNumberArray[S10], 
                                currNumberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl->bl5,
                                bl->bl6,
                                _indLed1,
                                _indLed2);

  uint32_t tmpval1 = tmpnextVal1;
  uint32_t tmpval2 = tmpnextVal2;
  uint32_t tmpval3 = tmpnextVal3;

  if (tmpSwitchTime > 0) {
    // Only need to calculate the switch values if we are going to switch
    tmpval1 = decodeFromNumberArray(
                                  tmpNumberArray[H10], 
                                  tmpNumberArray[H1],
                                  tmpDispTypeArray[H10] == BLANKED,
                                  tmpDispTypeArray[H1] == BLANKED,
                                  bl->bl1,
                                  bl->bl2,
                                  _led1State,
                                  _led2State);
    tmpval2 = decodeFromNumberArray(
                                  tmpNumberArray[M10], 
                                  tmpNumberArray[M1],
                                  tmpDispTypeArray[M10] == BLANKED,
                                  tmpDispTypeArray[M1] == BLANKED,
                                  bl->bl3,
                                  bl->bl4,
                                  _led3State,
                                  _led4State);
    tmpval3 = decodeFromNumberArray(
                                  tmpNumberArray[S10], 
                                  tmpNumberArray[S1],
                                  tmpDispTypeArray[S10] == BLANKED,
                                  tmpDispTypeArray[S1] == BLANKED,
                                  bl->bl5,
                                  bl->bl6,
                                  _indLed1,
                                  _indLed2);
  }

  // move the values over, respect the MUTEX on the interrupt
  portENTER_CRITICAL_ISR(&timerMux1);
  val1 = tmpval1;
  val2 = tmpval2;
  val3 = tmpval3;
  nextVal1 = tmpnextVal1;
  nextVal2 = tmpnextVal2;
  nextVal3 = tmpnextVal3;
  switchTime = tmpSwitchTime;
  portEXIT_CRITICAL_ISR(&timerMux1);
}

// ************************************************************
// Turn a display pair into a uint24 ready for output
// ************************************************************
uint32_t OutputManager_::decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool blankUnits, bool bl1, bool bl2, bool led1, bool led2) {
  uint32_t decoded = 0;
  if (!blankTens) decoded = DECODE_DIGIT[valueToDecodeTens];
  if (!blankUnits) decoded = decoded | DECODE_DIGIT[valueToDecodeUnits] << 10;
  if (led1) decoded |= DECODE_LED[0];
  if (led2) decoded |= DECODE_LED[1];
  if (bl1)  decoded |= DECODE_BLINKENIGHTS[0];
  if (bl2)  decoded |= DECODE_BLINKENIGHTS[1];
  return decoded;
}

// ************************************************************
// Apply leading zero blanking
// ************************************************************
void OutputManager_::applyBlanking() {

  // We only want to blank the hours tens digit
  if (cc->blankLeading && numberArray[H10] == 0) {
    displayType[H10] = BLANKED;
  }
  else {
    displayType[H10] = NORMAL;
  }
}

// ************************************************************
// Trigger slots/ACP processing
// ************************************************************
void OutputManager_::triggerStunts() {
  if (_acpOffset == 0) {
    if (second() == ACP_TRIGGER_SECOND) {
      if ((cc->acpMode == ACP_MODE_1M) ||
          ((cc->acpMode == ACP_MODE_10M) && (minute() % 10 == 9)) || 
          ((cc->acpMode == ACP_MODE_1H) && (minute() == 9))) {
        if (cc->useLDR) {
          if (cc->suppressACP) {
            if (!ldrManager.isMinLDRValue()) {
              // If we have suppress ACP set, only trigger when not at min brightness
              _acpOffset = 1;
            }
          } else {
            _acpOffset = 1;
          }
        } else {
          _acpOffset = 1;
        }
      }
    }

    if (_acpOffset != 0) {
      debugMsgOtm("Triggering ACP");
      _outputMode = acpMode;
    }
  }

  if (cc->slotsMode > SLOTS_MODE_NONE) {
    // Initialise the slots transition values and start it
    if (second() == SLOTS_TRIGGER_SECOND) {
      debugMsgOtm("Triggering Slots mode: " + String(cc->slotsMode));

      _outputMode = slotsMode;
      setCurrentTransition();
      activeTransition->start(nowMillis);
    }
  }
}

// ************************************************************
// Choose the transition we are using
// ************************************************************
void OutputManager_::setCurrentTransition() {
  if (cc->slotsMode > SLOTS_MODE_MIN) {
    // Which slots transition are we using?
    switch (cc->slotsMode) {
      case SLOTS_MODE_NONE: {
          activeTransition = &transitionDummy;
          break;
        }
      case SLOTS_MODE_WIPE: {
          activeTransition = &transitionWipe;
          break;
        }
      case SLOTS_MODE_BANG: {
          activeTransition = &transitionBang;
          break;
        }
      default: {
          activeTransition = &transitionDummy;
        }
    }
  }
}

// ************************************************************
// Slots/ACP loop processing
// ************************************************************
void OutputManager_::processStunts() {
  switch (_outputMode) {
    case acpMode: {
      // One armed bandit handling
      if (_acpOffset > 0) {
        if (_acpTick >= ACP_TICKS_PER_DIGIT) {
          _acpTick = 0;
          _acpOffset++;

          debugMsgOtm("ACP: " + String(_acpOffset-2));
          loadNumberArraySameValue(_acpOffset-2);
          if (_acpOffset == 12) {
            _acpOffset = 0;
            debugMsgOtm("ACP End");
            _outputMode = timeMode;
          }
        } else {
          _acpTick++;
        }
      }
      break;
    }
    case slotsMode: {
      if (activeTransition->isMessageOnDisplay(nowMillis)) {
        // Continue slots transition
        bool msgDisplaying = activeTransition->runEffect(nowMillis, cc->blankLeading);
        if (msgDisplaying) {
          activeTransition->updateRegularDisplaySeconds(second());
        }
      } else {
        // We were in slots but now we're not, so that means the last call ended them
        debugMsgOtm("Ending slots");
        _outputMode = timeMode;
      }
      break;        
    }
    default:
      break;
  }
}

// ************************************************************
// The number of the next ACP mode
// ************************************************************
byte OutputManager_::getNextACPMode(byte modeNumber) {
  byte nextNodeNumber = modeNumber+1;
  if (nextNodeNumber > ACP_MODE_MAX) {
    nextNodeNumber = ACP_MODE_MIN;
  }

  return nextNodeNumber;
}

// ************************************************************
// The name of the next ACP mode
// ************************************************************
String OutputManager_::getNextACPModeName(byte modeNumber) {
  byte nextNodeNumber = modeNumber+1;
  if (nextNodeNumber > ACP_MODE_MAX) {
    nextNodeNumber = ACP_MODE_MIN;
  }

  switch (nextNodeNumber) {
    case ACP_MODE_NONE: {
      return "Off";
      break;
    }
    case ACP_MODE_1M: {
      return "1 Min";
      break;
    }
    case ACP_MODE_10M: {
      return "10 Min";
      break;
    }
    case ACP_MODE_1H: {
      return "1 Hour";
      break;
    }
  } 
  return "Unknown";
}

// ************************************************************
// The number of the next Slots mode
// ************************************************************
byte OutputManager_::getNextSlotsMode(byte modeNumber) {
  byte nextNodeNumber = modeNumber+1;
  if (nextNodeNumber > SLOTS_MODE_MAX) {
    nextNodeNumber = SLOTS_MODE_MIN;
  }

  return nextNodeNumber;
}

// ************************************************************
// The name of the next ACP mode
// ************************************************************
String OutputManager_::getNextSlotsModeName(byte modeNumber) {
  byte nextNodeNumber = modeNumber+1;
  if (nextNodeNumber > SLOTS_MODE_MAX) {
    nextNodeNumber = SLOTS_MODE_MIN;
  }

  switch (nextNodeNumber) {
    case SLOTS_MODE_NONE: {
      return "Off";
      break;
    }
    case SLOTS_MODE_WIPE: {
      return "Wipe";
      break;
    }
    case SLOTS_MODE_BANG: {
      return "Bang";
      break;
    }
  } 
  return "Unknown";
}

// ************************************************************
// Set the sepator neons and indicator LEDs
// ************************************************************
void OutputManager_::processSeparators() {
  // --------------------------------------- separators --------------------------------------
  
  switch (cc->sepMode) {
    case SEP_RAILROAD:
      {
        _led1State = _led3State = upOrDown;
        _led2State = _led4State = !upOrDown;
        break;
      }
    case SEP_RAILROAD_X:
      {
        _led1State = _led4State = upOrDown;
        _led2State = _led3State = !upOrDown;
        break;
      }
    case SEP_BLINK_SLOW:
      {
        _led1State = _led3State = upOrDown;
        _led2State = _led4State = upOrDown;
        break;
      }
    case SEP_BLINK_FAST:
      {
        if (secsDeltaAbs < 500) {
        _led1State = _led3State = true;
        _led2State = _led4State = true;
        } else {
        _led1State = _led3State = false;
        _led2State = _led4State = false;
        }
        break;
      }
    case SEP_BLINK_DBL:
      {
        if ((secsDeltaAbs < 100) || ((secsDeltaAbs > 200) && (secsDeltaAbs < 300))) {
        _led1State = _led3State = true;
        _led2State = _led4State = true;
        } else {
        _led1State = _led3State = false;
        _led2State = _led4State = false;
        }
        break;
      }
    case SEP_ON:
      {
        _led1State = _led3State = true;
        _led2State = _led4State = true;
        break;
      }
    case SEP_OFF:
      {
        _led1State = _led3State = false;
        _led2State = _led4State = false;
        break;
      }
    case SEP_AM_PM:
      {
        _led1State = _led3State = isAM();
        _led2State = _led4State = isPM();
        break;
      }
  }

  // will probably think of something better sooner or later  
  #ifdef COG_CRANK_OUTPUT
  _indLed1 = (cogCrankSecsLeft > 0);
  #else
  _indLed1 = slaveManager.getSlaveMode();
  #endif
  _indLed2 = upOrDown;
}

// ************************************************************
// Get the mode we are in
// ************************************************************
outputModes OutputManager_::getOutputMode() {
  return _outputMode;
}

// ************************************************************
// Set the mode we are in
// ************************************************************
void OutputManager_::setOutputMode(outputModes newMode) {
  _outputMode = newMode;
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
OutputManager_ &OutputManager_::getInstance() {
  static OutputManager_ instance;
  return instance;
}

OutputManager_ &outputManager = outputManager.getInstance();