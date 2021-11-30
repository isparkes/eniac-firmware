#include "OutputManager.h"

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void loadNumberArrayTime() {
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
void loadNumberArrayDate() {
  switch (cc->dateFormat) {
    case DATE_FORMAT_YYMMDD:
      numberArray[5] = day() % 10;
      numberArray[4] = day() / 10;
      numberArray[3] = month() % 10;
      numberArray[2] = month() / 10;
      numberArray[1] = (year() - 2000) % 10;
      numberArray[0] = (year() - 2000) / 10;
      break;
    case DATE_FORMAT_MMDDYY:
      numberArray[5] = (year() - 2000) % 10;
      numberArray[4] = (year() - 2000) / 10;
      numberArray[3] = day() % 10;
      numberArray[2] = day() / 10;
      numberArray[1] = month() % 10;
      numberArray[0] = month() / 10;
      break;
    case DATE_FORMAT_DDMMYY:
      numberArray[5] = (year() - 2000) % 10;
      numberArray[4] = (year() - 2000) / 10;
      numberArray[3] = month() % 10;
      numberArray[2] = month() / 10;
      numberArray[1] = day() % 10;
      numberArray[0] = day() / 10;
      break;
  }
}

// ************************************************************
// Break the time into displayable digits
// ************************************************************
void loadNumberArraySameValue(byte value) {
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
void allNormal(bool leadingBlank) {

  if (leadingBlank)
    applyBlanking();
  else 
    displayType[0] = NORMAL;

  displayType[1] = NORMAL;
  displayType[2] = NORMAL;
  displayType[3] = NORMAL;
  displayType[4] = NORMAL;
  displayType[5] = NORMAL;
}

// ************************************************************
// Display preset
// ************************************************************
void allBlanked() {
  displayType[0] = BLANKED;
  displayType[1] = BLANKED;
  displayType[2] = BLANKED;
  displayType[3] = BLANKED;
  displayType[4] = BLANKED;
  displayType[5] = BLANKED;
}

// ************************************************************
// Do a single complete display, including any fading and
// dimming requested. Prepares the display variables for
// the interrupt driven display output.
// This is the heart of the display processing!
// ************************************************************
void outputDisplay() {
  blinkelights_t *bl = blinkenlightsManager.getBlinkenlights();
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
      if (blinkState) {
      } else {
        tmpDispType = BLANKED;
      }
    }

    // Trigger scolling and fading - scolling takes precendence
    if (numberArray[i] != currNumberArray[i]) {
      // Do scrollback when we are going to 0
      if ((numberArray[i] == 0) && cc->scrollback && (scrollCounter[i] == 0)) {
        scrollCounter[i] = (currNumberArray[i]+1) * cc->scrollSteps;
      } else if ((fadeState == 0) && cc->fade) {
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

    tmpDispTypeArray[i] = tmpDispType;
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

  uint32_t tmpval1 = decodeFromNumberArray( tmpNumberArray[H10], 
                                tmpNumberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl->bl1,
                                bl->bl2,
                                led1State,
                                led2State);
  uint32_t tmpval2 = decodeFromNumberArray( tmpNumberArray[M10], 
                                tmpNumberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl->bl3,
                                bl->bl4,
                                led1State,
                                led2State);
  uint32_t tmpval3 = decodeFromNumberArray( tmpNumberArray[S10], 
                                tmpNumberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl->bl5,
                                bl->bl6,
                                indLed1,
                                indLed2);

  // ToDo fading/scrolling
  uint32_t tmpnextVal1 = decodeFromNumberArray( currNumberArray[H10], 
                                currNumberArray[H1],
                                tmpDispTypeArray[H10] == BLANKED,
                                tmpDispTypeArray[H1] == BLANKED,
                                bl->bl1,
                                bl->bl2,
                                led1State,
                                led2State);
  uint32_t tmpnextVal2 = decodeFromNumberArray( currNumberArray[M10], 
                                currNumberArray[M1],
                                tmpDispTypeArray[M10] == BLANKED,
                                tmpDispTypeArray[M1] == BLANKED,
                                bl->bl3,
                                bl->bl4,
                                led1State,
                                led2State);
  uint32_t tmpnextVal3 = decodeFromNumberArray( currNumberArray[S10], 
                                currNumberArray[S1],
                                tmpDispTypeArray[S10] == BLANKED,
                                tmpDispTypeArray[S1] == BLANKED,
                                bl->bl5,
                                bl->bl6,
                                indLed1,
                                indLed2);

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
uint32_t decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool blankUnits, bool bl1, bool bl2, bool led1, bool led2) {
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
void applyBlanking() {

  // We only want to blank the hours tens digit
  if (cc->blankLeading && numberArray[H10] == 0) {
    displayType[H10] = BLANKED;
  }
  else {
    displayType[H10] = NORMAL;
  }
}
