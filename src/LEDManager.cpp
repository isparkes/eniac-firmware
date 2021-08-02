#include "LEDManager.h"

#ifdef APA106
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> leds(NUM_PIXELS_TOTAL, LED_DOUT);
#else
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> leds(NUM_PIXELS_TOTAL, LED_DOUT);
#endif

void LEDManager::setUp()
{
  // Set up the LED output
  leds.Begin();
}

// ************************************************************
// recalculate "slow moving" parameters
// ************************************************************
void LEDManager::recalculateVariables() {
  _backlightDim = (float) cc->backlightDimFactor / (float) 100;
#ifdef FEATURE_EXT_LEDS
  _underlightDim = (float) cc->extDimFactor / (float) 100;
#endif
}

// ************************************************************
// Set the LDR dimming value
// ************************************************************
void LEDManager::setLDRValue(unsigned int ldrValue)
{
  if (cc->useBLDim) {
    // calculate the PWM factor, goes between current_config.minDim% and 100%
    _ldrDimFactor = (float) (_ldrRange - ldrValue) / _ldrRange;
  }
}

// ************************************************************
// Set the LDR dimming value
// ************************************************************
void LEDManager::setLDRRange(unsigned int ldrRange)
{
    _ldrRange = (float) ldrRange;
}

// ************************************************************
// Set the pulse current value
// ************************************************************
void LEDManager::setPulseValue(unsigned int secsDelta)
{
  if (cc->useBLPulse) {
    // Calculate the brightness factor based on the "pulse"
    _pwmFactor = (float) secsDelta / (float) 1000.0;
  }
}

// ************************************************************
// Set blank status
// ************************************************************
void LEDManager::setBlanked(boolean blanked)
{
  _blanked = blanked;
}

// ************************************************************
// Set back light LEDs to the same colour
// ************************************************************
void LEDManager::setBacklightLEDs(byte red, byte green, byte blue) {
  for (int i = 0 ; i < NUM_BL_PIXELS ; i++) {
    setBacklightLED(i, red, green, blue);
  }
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager::setBacklightLED(byte index, byte red, byte green, byte blue) {  
    ledRb[LED_ADDR[index]] = red;
    ledGb[LED_ADDR[index]] = green;
    ledBb[LED_ADDR[index]] = blue;
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager::setTowerLEDs(byte red, byte green, byte blue) {
  #ifdef FEATURE_SEP_LED
    ledRb[4] = red;
    ledGb[4] = green;
    ledBb[4] = blue;
    
    ledRb[9] = red;
    ledGb[9] = green;
    ledBb[9] = blue;
  #endif
}

// ************************************************************
// Set under light LEDs to the same colour
// ************************************************************
void LEDManager::setUnderlightLEDs(byte red, byte green, byte blue) {
  #ifdef FEATURE_EXT_LEDS
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
    setUnderlightLED(i, red, green, blue);
  }
  #endif
}

// ************************************************************
// Set under light LEDs to the same colour
// ************************************************************
void LEDManager::setUnderlightLED(byte index, byte red, byte green, byte blue) {
  #ifdef FEATURE_EXT_LEDS
  ledRu[index] = red;
  ledGu[index] = green;
  ledBu[index] = blue;
  #endif
}

// ************************************************************
// Set day of week for the 'day of week' backlight mode
// ************************************************************
void LEDManager::setDayOfWeek(byte dow) {
  _dow = dow-1;
}

// ************************************************************
// Put the led buffers out
// ************************************************************
void LEDManager::outputLEDBuffer() {
  for (int i = 0 ; i < NUM_PIXELS_TOTAL - NUM_UL_PIXELS ; i++) {
#ifdef REVERSE_BL_OUTPUT
    RgbColor color(ledRb[NUM_BL_PIXELS - i - 1], ledGb[NUM_BL_PIXELS - i - 1], ledBb[NUM_BL_PIXELS - i - 1]);
#else
    RgbColor color(ledRb[i], ledGb[i], ledBb[i]);
#endif
    leds.SetPixelColor(i, color);
  }
  
  for (int i = 0 ; i < NUM_UL_PIXELS ; i++) {
#ifdef REVERSE_UL_OUTPUT
    RgbColor color(ledRu[DIGIT_COUNT - i - 1], ledGu[DIGIT_COUNT - i - 1], ledBu[DIGIT_COUNT - i - 1]);
#else
    RgbColor color(ledRu[i], ledGu[i], ledBu[i]);
#endif
    leds.SetPixelColor(i + NUM_BL_PIXELS, color);
  }
  leds.Show();
}

// ************************************************************
// Process the options and create a new buffer
// ************************************************************
void LEDManager::processLedStatus() {
  // -------------------------------- Backlights / Underlights -------------------------------

  if (_blanked) {
    setBacklightLEDs( getLEDAdjustedBL(0),
                      getLEDAdjustedBL(0),
                      getLEDAdjustedBL(0));
    setUnderlightLEDs(getLEDAdjustedUL(0),
                      getLEDAdjustedUL(0),
                      getLEDAdjustedUL(0));
  } else {
    switch (cc->backlightMode) {
      case BACKLIGHT_FIXED: {
          setBacklightLEDs( getLEDAdjustedBL(rgb_backlight_curve[cc->redCnl]),
                            getLEDAdjustedBL(rgb_backlight_curve[cc->grnCnl]),
                            getLEDAdjustedBL(rgb_backlight_curve[cc->bluCnl]));
          setUnderlightLEDs(getLEDAdjustedUL(rgb_backlight_curve[cc->redCnl]),
                            getLEDAdjustedUL(rgb_backlight_curve[cc->grnCnl]),
                            getLEDAdjustedUL(rgb_backlight_curve[cc->bluCnl]));
          break;
        }
      case BACKLIGHT_CYCLE: {
          cycleColours3(colors);
          setBacklightLEDs( getLEDAdjustedBL(colors[0]),
                            getLEDAdjustedBL(colors[1]),
                            getLEDAdjustedBL(colors[2]));
          setUnderlightLEDs(getLEDAdjustedUL(colors[0]),
                            getLEDAdjustedUL(colors[1]),
                            getLEDAdjustedUL(colors[2]));
          break;
        }
      case BACKLIGHT_COLOUR_TIME: {
          if (!_syncColourTime) {
            for (byte i = 0 ; i < DIGIT_COUNT ; i++) {
              byte numVal = numberArray[i];
              setBacklightLED(i, 
                              getLEDAdjustedBL(colourTimeR[numVal]),
                              getLEDAdjustedBL(colourTimeG[numVal]),
                              getLEDAdjustedBL(colourTimeB[numVal]));
              setUnderlightLED(i, 
                              getLEDAdjustedUL(colourTimeR[numVal]),
                              getLEDAdjustedUL(colourTimeG[numVal]),
                              getLEDAdjustedUL(colourTimeB[numVal]));
            }
          }
          break;
        }
      case BACKLIGHT_DAY_OF_WEEK: {
          setBacklightLEDs( getLEDAdjustedBL(dayOfWeekR[_dow]),
                            getLEDAdjustedBL(dayOfWeekG[_dow]),
                            getLEDAdjustedBL(dayOfWeekB[_dow]));
          setUnderlightLEDs(getLEDAdjustedUL(dayOfWeekR[_dow]),
                            getLEDAdjustedUL(dayOfWeekG[_dow]),
                            getLEDAdjustedUL(dayOfWeekB[_dow]));
          break;
        }
    }
    setTowerLEDs(   getLEDAdjustedUL(255),
                    getLEDAdjustedUL(0),
                    getLEDAdjustedUL(0));    
  }

  outputLEDBuffer();
}

// ************************************************************
// output a PWM LED channel, adjusting for dimming, PWM
// and user back light brightness
// ************************************************************
byte LEDManager::getLEDAdjustedBL(byte rawValue) {
  byte dimmedPWMVal;
  if (cc->useBLDim) {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _backlightDim * _ldrDimFactor);
    } else {
      dimmedPWMVal = (byte)(rawValue * _backlightDim * _ldrDimFactor);
    }
  } else {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _backlightDim);
    } else {
      dimmedPWMVal = (byte)(rawValue * _backlightDim);
    }
  }
  return dim_curve[dimmedPWMVal];
}

// ************************************************************
// output a PWM LED channel, adjusting for dimming, PWM
// and user under light brightness
// ************************************************************
byte LEDManager::getLEDAdjustedUL(byte rawValue) {
  byte dimmedPWMVal;
  if (cc->useBLDim) {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _underlightDim * _ldrDimFactor);
    } else {
      dimmedPWMVal = (byte)(rawValue * _underlightDim * _ldrDimFactor);
    }
  } else {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _underlightDim);
    } else {
      dimmedPWMVal = (byte)(rawValue * _underlightDim);
    }
  }
  return dim_curve[dimmedPWMVal];
}

// ************************************************************
// Colour cycling 3: one colour dominates
// ************************************************************
void LEDManager::cycleColours3(int colors[3]) {
  _cycleCount++;
  if (_cycleCount > cc->cycleSpeed) {
    _cycleCount = 0;

    if (changeSteps == 0) {
      changeSteps = random(256);
      currentColour = random(3);
    }

    changeSteps--;

    switch (currentColour) {
      case 0:
        if (colors[0] < 255) {
          colors[0]++;
          if (colors[1] > 0) {
            colors[1]--;
          }
          if (colors[2] > 0) {
            colors[2]--;
          }
        } else {
          changeSteps = 0;
        }
        break;
      case 1:
        if (colors[1] < 255) {
          colors[1]++;
          if (colors[0] > 0) {
            colors[0]--;
          }
          if (colors[2] > 0) {
            colors[2]--;
          }
        } else {
          changeSteps = 0;
        }
        break;
      case 2:
        if (colors[2] < 255) {
          colors[2]++;
          if (colors[0] > 0) {
            colors[0]--;
          }
          if (colors[1] > 0) {
            colors[1]--;
          }
        } else {
          changeSteps = 0;
        }
        break;
    }
  }
}

void LEDManager::setSyncColourTime(boolean value) {
  _syncColourTime = value;
}


// ************************************************************
// Set the diagnostic LED colour - progressively setting the
// LEDs to dignostic colours
// ************************************************************
void LEDManager::setDiagnosticLED(byte stepNumber, byte state) {
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
    if (i > stepNumber) {
      setBacklightLED(i, 0x1f, 0x1f, 0x1f);
      setUnderlightLED(i, 0x1f, 0x1f, 0x1f);
    } else if (i == stepNumber) {
      if (state == STATUS_RED) {
      setBacklightLED(i, 0xff, 0, 0);
      setUnderlightLED(i, 0xff, 0, 0);
      } else if (state == STATUS_YELLOW) {
      setBacklightLED(i, 0xff, 0x7f, 0x0f);
      setUnderlightLED(i, 0xff, 0x7f, 0x0f);
      } else if (state == STATUS_GREEN) {
      setBacklightLED(i, 0, 0xff, 0);
      setUnderlightLED(i, 0, 0xff, 0);
      } else if (state == STATUS_BLUE) {
      setBacklightLED(i, 0, 0, 0xff);
      setUnderlightLED(i, 0, 0, 0xff);
      }
    }
  }
  outputLEDBuffer();
}