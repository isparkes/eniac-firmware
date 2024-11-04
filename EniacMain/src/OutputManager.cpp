#include "OutputManager.h"

// ************************************************************
// Load the display according to the value we are told to use
// ************************************************************
void OutputManager_::setArbitraryValue(unsigned int newValue) {
  _arbitraryValue = newValue;
}

// ************************************************************
// Load the display according to the value we are told to use
// ************************************************************
void OutputManager_::setArbitraryValueDisplayTime(unsigned int newValue) {
  _arbitraryValueEndTime = nowMillis + 1000 * newValue;
}

// ************************************************************
// Load the display according to the value we are told to use
// ************************************************************
void OutputManager_::loadNumberArrayPrimary() {
  byte tmpMode = cc->pMode;

  // Value mode, if set by REST Push
  if (_arbitraryValueEndTime > nowMillis) {
    tmpMode = DISPLAY_VALUE;
  }

  #ifdef DIGIT_DIAGNOSTICS
  if (cc->diagsMode == DIGIT_DIAGS_MODE_FAST) {
    setArbitraryValue(second());
    setArbitraryValueDisplayTime(10);
    tmpMode = DISPLAY_VALUE;
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_SLOW) {
    setArbitraryValue(minute());
    setArbitraryValueDisplayTime(10);
    tmpMode = DISPLAY_VALUE;
  } else if (cc->diagsMode == DIGIT_DIAGS_MODE_ENCODER) {
    #ifdef FEATURE_MENU
    // Encoder value injected by loop
    tmpMode = DISPLAY_VALUE;
    #endif
  }
  #endif

  #ifdef OTM_EXTENDED_DEBUG
  debugMsgOtm("Modes T: " + String(tmpMode) + " P: " + String(cc->pMode) + " S: " + String(cc->sMode));
  #endif

  loadNumberArrayInternal(tmpMode);
}

// ************************************************************
// Load the display according to the value we are told to use
// ************************************************************
void OutputManager_::loadNumberArraySecondary() {
  loadNumberArrayInternal(cc->sMode);
}

// ************************************************************
// Load the display according to the value we are told to use
// ************************************************************
void OutputManager_::loadNumberArrayInternal(byte tmpMode) {
  if (tmpMode == DISPLAY_COUNTDOWN) {
    if (!countdownManager.getCountdownActive()) {
      // We have finished countdown - show time instead 
      tmpMode = DISPLAY_TIME;
    }
  }

  switch (tmpMode) {
  default:
  case DISPLAY_TIME:
    loadNumberArrayTime();
    allNormal(APPLY_LEAD_0_BLANK);
    break;
  case DISPLAY_DATE:
    loadNumberArrayDate();
    allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    break;
  case DISPLAY_VALUE:
    loadNumberArrayValue();
    allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    break;
  case DISPLAY_COUNTDOWN:
    setArbitraryValue(countdownManager.getRemaining());
    setArbitraryValueDisplayTime(10);
    loadNumberArrayValue();
    allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    break;
  case DISPLAY_TICKER:
    loadNumberArrayTicker();
    allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    break;
  }
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArrayTime() {
  numberArray[S1]  = convertToDigit(second() % 10);
  numberArray[S10] = convertToDigit(second() / 10);
  numberArray[M1]  = convertToDigit(minute() % 10);
  numberArray[M10] = convertToDigit(minute() / 10);
  if (cc->hourMode) {
    numberArray[H1]  = convertToDigit(hourFormat12() % 10);
    numberArray[H10] = convertToDigit(hourFormat12() / 10);
  } else {
    numberArray[H1]  = convertToDigit(hour() % 10);
    numberArray[H10] = convertToDigit(hour() / 10);
  }
}

// ************************************************************
// Load the last acquired ticker value
// ************************************************************
void OutputManager_::loadNumberArrayTicker() {
  if (quoteManager.getIsQuoteValid()) {
    loadNumberArrayIntegerValue(quoteManager.getLastQuote());
  }
}

// ************************************************************
// Load the arbitrary value
// ************************************************************
void OutputManager_::loadNumberArrayValue() {
  loadNumberArrayIntegerValue(_arbitraryValue);
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArrayIntegerValue(unsigned int value) {
  unsigned int valueBound = value;
  if (valueBound > 999999)
    valueBound = 999999;
  
  byte s1 = valueBound % 10;
  valueBound = valueBound / 10;
  byte s10 = valueBound % 10;
  valueBound = valueBound / 10;
  byte m1 = valueBound % 10;
  valueBound = valueBound / 10;
  byte m10 = valueBound % 10;
  valueBound = valueBound / 10;
  byte h1 = valueBound % 10;
  valueBound = valueBound / 10;
  byte h10 = valueBound % 10;

  numberArray[S1]  = convertToDigit(s1  % 10);
  numberArray[S10] = convertToDigit(s10 % 10);
  numberArray[M1]  = convertToDigit(m1  % 10);
  numberArray[M10] = convertToDigit(m10 % 10);
  numberArray[H1]  = convertToDigit(h1  % 10);
  numberArray[H10] = convertToDigit(h10 % 10);
}

// ************************************************************
// Set all digits to the same value, but blank all except one
// ************************************************************
void OutputManager_::loadNumberArrayBurn(byte value) {
  allBlanked();
  loadNumberArraySameValue(value);
  displayType[_arbitraryValue / 10] = NORMAL;
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArrayDate() {
  switch (cc->dateFormat) {
    case DATE_FORMAT_YYMMDD:
      numberArray[S1]  = convertToDigit(day() % 10);
      numberArray[S10] = convertToDigit(day() / 10);
      numberArray[M1]  = convertToDigit(month() % 10);
      numberArray[M10] = convertToDigit(month() / 10);
      numberArray[H1]  = convertToDigit((year() - 2000) % 10);
      numberArray[H10] = convertToDigit((year() - 2000) / 10);
      break;
    case DATE_FORMAT_MMDDYY:
      numberArray[S1]  = convertToDigit((year() - 2000) % 10);
      numberArray[S10] = convertToDigit((year() - 2000) / 10);
      numberArray[M1]  = convertToDigit(day() % 10);
      numberArray[M10] = convertToDigit(day() / 10);
      numberArray[H1]  = convertToDigit(month() % 10);
      numberArray[H10] = convertToDigit(month() / 10);
      break;
    case DATE_FORMAT_DDMMYY:
      numberArray[S1]  = convertToDigit((year() - 2000) % 10);
      numberArray[S10] = convertToDigit((year() - 2000) / 10);
      numberArray[M1]  = convertToDigit(month() % 10);
      numberArray[M10] = convertToDigit(month() / 10);
      numberArray[H1]  = convertToDigit(day() % 10);
      numberArray[H10] = convertToDigit(day() / 10);
      break;
  }
}

// ************************************************************
// Spin the digits, used for the "scramble" effect
// ************************************************************
void OutputManager_::incrementNumberArray() {
  numberArray[S1]  = convertToDigit((numberArray[S1] +1)%10);
  numberArray[S10] = convertToDigit((numberArray[S10]+1)%10);
  numberArray[M1]  = convertToDigit((numberArray[M1] +1)%10);
  numberArray[M10] = convertToDigit((numberArray[M10]+1)%10);
  numberArray[H1]  = convertToDigit((numberArray[H1] +1)%10);
  numberArray[H10] = convertToDigit((numberArray[H10]+1)%10);
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void OutputManager_::loadNumberArraySameValue(byte value) {
  byte val = value % 10;
  numberArray[S1]  = convertToDigit(val);
  numberArray[S10] = convertToDigit(val);
  numberArray[M1]  = convertToDigit(val);
  numberArray[M10] = convertToDigit(val);
  numberArray[H1]  = convertToDigit(val);
  numberArray[H10] = convertToDigit(val);
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

  // Reset the temp blanking
  _blankTubesTemp = false;
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

  #ifdef FEATURE_BLINKENLIGHTS
  blinkenlights_t *bl = blinkenlightsManager.getBlinkenlights();
  #endif

  bool digitBlanked[DIGIT_COUNT];
  byte tmpNumberArray[DIGIT_COUNT];

  for ( int i = DIGIT_COUNT - 1 ; i >= 0  ; i -- ) {
    // Digit blanking
    digitBlanked[i] = (displayType[i] == BLANKED) ||
      // Digit blinking
      ((displayType[i] == BLINK) && !upOrDown) ||
      // display blanking
      _blankTubes ||
      // forced blanking for transitions
      _blankTubesTemp;

    // ------------------- Trigger scolling and fading - scolling takes precendence -------------------
    switch(_outputMode) {
      case primaryMode:
      case valueMode: {
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
          currNumberArray[i] = convertToDigit(scrollCounter[i]/cc->scrollSteps);
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
    // We're doing the last step - reset stuff and copy the digit over
    fadeState = 0;
    tmpSwitchTime = -1; // Make sure that we don't trigger the switch 
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
                                #ifdef NORMAL_DIGIT_OUTPUT
                                currNumberArray[H10], 
                                currNumberArray[H1],
                                digitBlanked[H10],
                                digitBlanked[H1],
                                #endif
                                #ifdef REVERSE_DIGIT_OUTPUT
                                currNumberArray[S1], 
                                currNumberArray[S10],
                                digitBlanked[S1],
                                digitBlanked[S10],
                                #endif
                                _blankSeparators,
                                #ifdef FEATURE_BLINKENLIGHTS
                                #ifdef NORMAL_DIGIT_OUTPUT
                                bl->bl1,
                                bl->bl2,
                                #endif
                                #ifdef REVERSE_DIGIT_OUTPUT
                                bl->bl5,
                                bl->bl6,
                                #endif
                                #else
                                false,
                                false,
                                #endif
                                _sep1State,
                                _sep2State);
  uint32_t tmpnextVal2 = decodeFromNumberArray(
                                #ifdef NORMAL_DIGIT_OUTPUT
                                currNumberArray[M10], 
                                currNumberArray[M1],
                                digitBlanked[M10],
                                digitBlanked[M1],
                                #endif
                                #ifdef REVERSE_DIGIT_OUTPUT
                                currNumberArray[M1], 
                                currNumberArray[M10],
                                digitBlanked[M1],
                                digitBlanked[M10],
                                #endif
                                _blankSeparators,
                                #ifdef FEATURE_BLINKENLIGHTS
                                #ifdef NORMAL_DIGIT_OUTPUT
                                bl->bl3,
                                bl->bl4,
                                #endif
                                #ifdef REVERSE_DIGIT_OUTPUT
                                bl->bl3,
                                bl->bl4,
                                #endif
                                #else
                                false,
                                false,
                                #endif
                                _sep3State,
                                _sep4State);
  uint32_t tmpnextVal3 = decodeFromNumberArray(
                                #ifdef NORMAL_DIGIT_OUTPUT
                                currNumberArray[S10], 
                                currNumberArray[S1],
                                digitBlanked[S10],
                                digitBlanked[S1],
                                #endif
                                #ifdef REVERSE_DIGIT_OUTPUT
                                currNumberArray[H1], 
                                currNumberArray[H10],
                                digitBlanked[H1],
                                digitBlanked[H10],
                                #endif
                                _blankSeparators,
                                #ifdef FEATURE_BLINKENLIGHTS
                                #ifdef NORMAL_DIGIT_OUTPUT
                                bl->bl5,
                                bl->bl6,
                                #endif
                                #ifdef REVERSE_DIGIT_OUTPUT
                                bl->bl1,
                                bl->bl2,
                                #endif
                                #else
                                false,
                                false,
                                #endif
                                bl->in1,
                                bl->in2);

  uint32_t tmpval1 = tmpnextVal1;
  uint32_t tmpval2 = tmpnextVal2;
  uint32_t tmpval3 = tmpnextVal3;

  if (tmpSwitchTime > 0) {
    // Only need to calculate the switch values if we are going to switch
    tmpval1 = decodeFromNumberArray(
                                  #ifdef NORMAL_DIGIT_OUTPUT
                                  tmpNumberArray[H10], 
                                  tmpNumberArray[H1],
                                  digitBlanked[H10],
                                  digitBlanked[H1],
                                  #endif
                                  #ifdef REVERSE_DIGIT_OUTPUT
                                  tmpNumberArray[S1], 
                                  tmpNumberArray[S10],
                                  digitBlanked[S1],
                                  digitBlanked[S10],
                                  #endif
                                  _blankSeparators,
                                  #ifdef FEATURE_BLINKENLIGHTS
                                  #ifdef NORMAL_DIGIT_OUTPUT
                                  bl->bl1,
                                  bl->bl2,
                                  #endif
                                  #ifdef REVERSE_DIGIT_OUTPUT
                                  bl->bl5,
                                  bl->bl6,
                                  #endif
                                  #else
                                  false,
                                  false,
                                  #endif
                                  _sep1State,
                                  _sep2State);
    tmpval2 = decodeFromNumberArray(
                                  #ifdef NORMAL_DIGIT_OUTPUT
                                  tmpNumberArray[M10], 
                                  tmpNumberArray[M1],
                                  digitBlanked[M10],
                                  digitBlanked[M1],
                                  #endif
                                  #ifdef REVERSE_DIGIT_OUTPUT
                                  tmpNumberArray[M1], 
                                  tmpNumberArray[M10],
                                  digitBlanked[M1],
                                  digitBlanked[M10],
                                  #endif
                                  _blankSeparators,
                                  #ifdef FEATURE_BLINKENLIGHTS
                                  #ifdef NORMAL_DIGIT_OUTPUT
                                  bl->bl3,
                                  bl->bl4,
                                  #endif
                                  #ifdef REVERSE_DIGIT_OUTPUT
                                  bl->bl3,
                                  bl->bl4,
                                  #endif
                                  #else
                                  false,
                                  false,
                                  #endif
                                  _sep3State,
                                  _sep4State);
    tmpval3 = decodeFromNumberArray(
                                  #ifdef NORMAL_DIGIT_OUTPUT
                                  tmpNumberArray[S10], 
                                  tmpNumberArray[S1],
                                  digitBlanked[S10],
                                  digitBlanked[S1],
                                  #endif
                                  #ifdef REVERSE_DIGIT_OUTPUT
                                  tmpNumberArray[H1], 
                                  tmpNumberArray[H10],
                                  digitBlanked[H1],
                                  digitBlanked[H10],
                                  #endif
                                  _blankSeparators,
                                  #ifdef FEATURE_BLINKENLIGHTS
                                  #ifdef NORMAL_DIGIT_OUTPUT
                                  bl->bl5,
                                  bl->bl6,
                                  #endif
                                  #ifdef REVERSE_DIGIT_OUTPUT
                                  bl->bl1,
                                  bl->bl2,
                                  #endif
                                  #else
                                  false,
                                  false,
                                  #endif
                                  bl->in1,
                                  bl->in2);
  }

  // move the values over, respect the MUTEX on the interrupt, otherwise we get visible glitches
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
uint32_t OutputManager_::decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool blankUnits, bool blankSeparators, bool bl1, bool bl2, bool led1, bool led2) {
  uint32_t decoded = 0;
  if (!blankTens) decoded = DECODE_DIGIT[valueToDecodeTens];
  if (!blankUnits) decoded = decoded | DECODE_DIGIT[valueToDecodeUnits] << 10;
  if (!blankSeparators) {
    if (led1) decoded |= DECODE_LED[0];
    if (led2) decoded |= DECODE_LED[1];
  }
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
  // only trigger stunts in time mode
  if (_outputMode != primaryMode)
    return;

  if (second() == ACP_TRIGGER_SECOND) {
    #ifdef OTM_EXTENDED_DEBUG
    debugMsgOtm("Check ACP trigger");
    #endif
    if ((cc->acpMode == ACP_MODE_1M) ||
        ((cc->acpMode == ACP_MODE_10M) && (minute() % 10 == 9)) || 
        ((cc->acpMode == ACP_MODE_1H) && (minute() == 9))) {
      if (cc->useLDRTube) {
        if (cc->suppressACP) {
          if (!ldrManager.isMinDim()) {
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

  if (_acpOffset == 1) {
    #ifdef OTM_EXTENDED_DEBUG
    debugMsgOtm("Triggering ACP");
    #endif
    _outputMode = acpMode;
    ldrManager.setLDRValueToMaxACP(true);
  }

  if (second() == SLOTS_TRIGGER_SECOND) {
    // If we have a mode configured and the main mode is different from the slots mode
    if ((cc->slotsMode > SLOTS_MODE_NONE) and (cc->pMode != cc->sMode)) {
      #ifdef OTM_EXTENDED_DEBUG
      debugMsgOtm("Triggering Slots mode: " + String(cc->slotsMode));
      #endif

      #ifdef FEATURE_TICKER
      if (cc->sMode == DISPLAY_TICKER) {
        if (quoteManager.getIsQuoteValid()) {

          byte randomMode = BACKLIGHT_UP;
          if (nowMillis % 2 == 0) {
            ledManager.setTickerOverrideValue(down);
          } else {
            ledManager.setTickerOverrideValue(up);
          }
        } else {
          debugMsgOtm("Skip displaying quote because no quote is valid");
          return;
        }
      }
      #endif

      _outputMode = secondaryMode;
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
      case SLOTS_MODE_SCRAMBLE: {
          activeTransition = &transitionScramble;
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

          #ifdef OTM_EXTENDED_DEBUG
          debugMsgOtm("ACP: " + String(_acpOffset-1));
          #endif
          loadNumberArraySameValue(_acpOffset-1);
          if (_acpOffset > 10) {
            _acpOffset = 0;
            #ifdef OTM_EXTENDED_DEBUG
            debugMsgOtm("ACP End");
            #endif
            _outputMode = primaryMode;
            loadNumberArrayPrimary();
            ldrManager.setLDRValueToMaxACP(false);
          } else {
            _acpOffset++;
          }
        } else {
          _acpTick++;
        }
      }
      break;
    }
    case secondaryMode: {
      if (activeTransition->isMessageOnDisplay(nowMillis)) {
        // Continue slots transition
        bool msgDisplaying = activeTransition->runEffect(nowMillis, cc->blankLeading);
        if (msgDisplaying) {
          activeTransition->updateRegularDisplaySeconds(second());
        }
      } else {
        // We were in slots but now we're not, so that means the last call ended them
        #ifdef OTM_EXTENDED_DEBUG
        debugMsgOtm("Ending slots");
        #endif

        #ifdef FEATURE_TICKER
        ledManager.setTickerOverrideValue(none);
        #endif

        _outputMode = primaryMode;
      }
      break;        
    }
    default:
      break;
  }
}

// ----------------------------------- ACP Handling -----------------------------------

// ************************************************************
// The number of the next ACP mode
// ************************************************************
byte OutputManager_::getNextACPMode() {
  byte nextNodeNumber = cc->acpMode + 1;
  if (nextNodeNumber > ACP_MODE_MAX) {
    nextNodeNumber = ACP_MODE_MIN;
  }

  return nextNodeNumber;
}

// ************************************************************
// The name of the next ACP mode
// ************************************************************
String OutputManager_::getNextACPModeName() {
  byte nextNodeNumber = getNextACPMode();

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

void OutputManager_::setACPMode(byte newMode) {
  if (newMode > ACP_MODE_MAX) newMode = ACP_MODE_MAX;
  if (newMode < ACP_MODE_MIN) newMode = ACP_MODE_MIN;
  cc->acpMode = newMode;
}

void OutputManager_::setNextACPMode() {
  cc->acpMode = getNextACPMode();
}

// ---------------------------------- Slots Handling ----------------------------------

// ************************************************************
// The number of the next Slots mode
// ************************************************************
byte OutputManager_::getNextSlotsMode() {
  byte nextNodeNumber = cc->slotsMode + 1;
  if (nextNodeNumber > SLOTS_MODE_MAX) {
    nextNodeNumber = SLOTS_MODE_MIN;
  }

  return nextNodeNumber;
}

// ************************************************************
// The name of the next ACP mode
// ************************************************************
String OutputManager_::getNextSlotsModeName() {
  byte nextNodeNumber = getNextSlotsMode();

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
    case SLOTS_MODE_SCRAMBLE: {
      return "Scramble";
      break;
    }
  } 
  return "Unknown";
}

void OutputManager_::setSlotsMode(byte newMode) {
  if (newMode > SLOTS_MODE_MAX) newMode = SLOTS_MODE_MAX;
  if (newMode < SLOTS_MODE_MIN) newMode = SLOTS_MODE_MIN;
  cc->slotsMode = newMode;
}

void OutputManager_::setNextSlotsMode() {
  cc->slotsMode = getNextSlotsMode();
}

// --------------------------------------- separators --------------------------------------

// ************************************************************
// Set the sepator neons and indicator LEDs
// ************************************************************
void OutputManager_::processSeparators() {
  
  switch (cc->sepMode) {
    case SEP_RAILROAD:
      {
        _sep1State = _sep3State = upOrDown;
        _sep2State = _sep4State = !upOrDown;
        break;
      }
    case SEP_RAILROAD_X:
      {
        _sep1State = _sep4State = upOrDown;
        _sep2State = _sep3State = !upOrDown;
        break;
      }
    case SEP_BLINK_SLOW:
      {
        _sep1State = _sep3State = upOrDown;
        _sep2State = _sep4State = upOrDown;
        break;
      }
    case SEP_BLINK_FAST:
      {
        if (secsDeltaAbs < 500) {
        _sep1State = _sep3State = true;
        _sep2State = _sep4State = true;
        } else {
        _sep1State = _sep3State = false;
        _sep2State = _sep4State = false;
        }
        break;
      }
    case SEP_BLINK_DBL:
      {
        if ((secsDeltaAbs < 100) || ((secsDeltaAbs > 200) && (secsDeltaAbs < 300))) {
        _sep1State = _sep3State = true;
        _sep2State = _sep4State = true;
        } else {
        _sep1State = _sep3State = false;
        _sep2State = _sep4State = false;
        }
        break;
      }
    case SEP_ON:
      {
        _sep1State = _sep3State = true;
        _sep2State = _sep4State = true;
        break;
      }
    case SEP_OFF:
      {
        _sep1State = _sep3State = false;
        _sep2State = _sep4State = false;
        break;
      }
    case SEP_AM_PM:
      {
        _sep1State = _sep3State = isAM();
        _sep2State = _sep4State = isPM();
        break;
      }
  }
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
// Set the mode we are in
// ************************************************************
void OutputManager_::updateOncePerSecond() {
  // Check Slots / ACP
  triggerStunts();

  // If we are in slots or ACP we don't need to look at the rest
  if (_outputMode == acpMode || _outputMode == secondaryMode) {
    return;
  }

  loadNumberArrayPrimary();
}

// ************************************************************
// Set the tube blanking status of the tubes
// ************************************************************
void OutputManager_::setBlankingStatusTubes(bool newStatus) {
  _blankTubes = newStatus;
}

// ************************************************************
// Set the tube blanking status of the tubes until the next
// display change. Used in transitions.
// ************************************************************
void OutputManager_::forceBlanking() {
  _blankTubesTemp = true;
}

// ************************************************************
// Set the tube blanking status of the tubes
// ************************************************************
void OutputManager_::setBlankingStatusTowers(bool newStatus) {
  _blankTubes = newStatus;
}

// ************************************************************
// Get the current display value of a given digit
// ************************************************************
byte OutputManager_::getCurrentDisplayDigitValue(byte digit) {
  if (digit < DIGIT_COUNT) {
    return numberArray[digit];
  } else {
    return 0;
  }
}

// ************************************************************
// Safety function: Convert given value to a valid digit value
// ************************************************************
clock_digit OutputManager_::convertToDigit(int value) {
  if (value < 0) {
    debugMsgOtm("Underrange error converting digit");
    debugMsgOtm("Got: " + String(value));
    return digit0;
  }
  if (value > 9) {
    debugMsgOtm("Overrange error converting digit");
    debugMsgOtm("Got: " + String(value));
    return digit9;
  }
  return (clock_digit) value;
}

#ifdef OTM_EXTENDED_DEBUG
void OutputManager_::dumpNumberArrayValues() {
  String val = "Number Array: " + 
    String(numberArray[0]) + String(numberArray[1]) + ":" + 
    String(numberArray[2]) + String(numberArray[3]) + ":" +
    String(numberArray[4]) + String(numberArray[5]);
  debugMsgOtm(val);

}
#endif

// ************************************************************
// Library internal singleton wiring
// ************************************************************
OutputManager_ &OutputManager_::getInstance() {
  static OutputManager_ instance;
  return instance;
}

OutputManager_ &outputManager = outputManager.getInstance();